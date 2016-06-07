#include <stdarg.h>
#include "pub/mem.h"
#include "pub/com.h"
#include "std/str.h"
#include "exec.h"
#include "byte.h"
#include "err.h"

ivm_exec_t *
ivm_exec_new(ivm_string_pool_t *pool)
{
	ivm_exec_t *ret = MEM_ALLOC(sizeof(*ret), ivm_exec_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("executable"));

	ret->pool = pool;
	ret->alloc = IVM_DEFAULT_INSTR_BLOCK_BUFFER_SIZE;
	ret->next = 0;
	ret->instrs = MEM_ALLOC(sizeof(*ret->instrs)
							* IVM_DEFAULT_INSTR_BLOCK_BUFFER_SIZE,
							ivm_instr_t *);

	IVM_ASSERT(ret->instrs, IVM_ERROR_MSG_FAILED_ALLOC_NEW("instruction list in executable"));

	return ret;
}

void
ivm_exec_free(ivm_exec_t *exec)
{
	if (exec) {
		MEM_FREE(exec->instrs);
		MEM_FREE(exec);
	}

	return;
}

IVM_PRIVATE
void
ivm_exec_expand(ivm_exec_t *exec)
{
	exec->alloc <<= 1;
	exec->instrs = MEM_REALLOC(exec->instrs,
								sizeof(*exec->instrs)
								* exec->alloc,
								ivm_instr_t *);

	IVM_ASSERT(exec->instrs,
			   IVM_ERROR_MSG_FAILED_ALLOC_NEW("expanded instruction list in executable"));

	return;
}

ivm_size_t
ivm_exec_addInstr(ivm_exec_t *exec,
				  ivm_instr_t instr)
{
	if (exec->next >= exec->alloc) {
		ivm_exec_expand(exec);
	}

	exec->instrs[exec->next] = instr;

	return exec->next++;
}

#if 0

ivm_exec_t *
ivm_exec_new(ivm_string_pool_t *pool)
{
	ivm_exec_t *ret = MEM_ALLOC(sizeof(*ret), ivm_exec_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("executable"));

	ret->pool = pool;
	ret->length = IVM_DEFAULT_EXEC_BUFFER_SIZE;
	ret->cur = 0;
	ret->code = MEM_ALLOC_INIT(sizeof(*ret->code)
							   * IVM_DEFAULT_EXEC_BUFFER_SIZE,
							   ivm_byte_t *);

	IVM_ASSERT(ret->code, IVM_ERROR_MSG_FAILED_ALLOC_NEW("code buffer in executable"));

	return ret;
}

void
ivm_exec_free(ivm_exec_t *exec)
{
	if (exec) {
		MEM_FREE(exec->code);
		MEM_FREE(exec);
	}

	return;
}

void
ivm_exec_compact(ivm_exec_t *exec)
{
	if (exec && exec->length > exec->cur) {
		exec->code = MEM_REALLOC(exec->code,
								 sizeof(*exec->code)
								 * exec->cur,
								 ivm_byte_t *);
		IVM_ASSERT(exec->code, IVM_ERROR_MSG_FAILED_ALLOC_NEW("compacted code in executable"));
		exec->length = exec->cur;
	}

	return;
}

void
ivm_exec_addBuffer(ivm_exec_t *exec)
{
	exec->code = MEM_REALLOC(exec->code,
							 sizeof(*exec->code)
							 * (exec->length + IVM_DEFAULT_EXEC_BUFFER_SIZE),
							 ivm_byte_t *);
	IVM_ASSERT(exec->code, IVM_ERROR_MSG_FAILED_ALLOC_NEW("new code buffer in executable"));
	exec->length += IVM_DEFAULT_EXEC_BUFFER_SIZE;

	return;
}

#define CUR_EXEC (&exec->code[n_cur])
#define REM_LEN (exec->length - n_cur)

#define WRITE_ARG(arg_type, val) \
	while (exec->length <= n_cur || \
		   !(tmp = ivm_byte_write##arg_type(CUR_EXEC, \
											REM_LEN, \
											(val)))) { \
		ivm_exec_addBuffer(exec); \
	}

#define IF_INT_N_THEN_CALL_1(str, n, type) \
	if ((str)[n] == (#type)[4]) { \
		tmp_int = va_arg(args, ivm_int_t); \
		WRITE_ARG(type, tmp_int); \
		n_cur += tmp; \
		i += 2; \
		break; \
	}

#define IF_INT_N_THEN_CALL_2(str, n, type) \
	if ((str)[n] == (#type)[4] \
		&& (str)[(n) + 1] == (#type)[5]) { \
		tmp_int = va_arg(args, ivm_int_t); \
		WRITE_ARG(type, tmp_int); \
		n_cur += tmp; \
		i += 3; \
		break; \
	}

IVM_PRIVATE
ivm_size_t
ivm_exec_addCode_c(ivm_exec_t *exec, ivm_opcode_t op, const ivm_char_t *format, va_list args)
{
	ivm_size_t i, n_cur, tmp;
	ivm_int_t tmp_int;
	ivm_size_t ret = exec->cur, tmp_i;

	n_cur = exec->cur + 1;
	while (exec->length <= n_cur)
		ivm_exec_addBuffer(exec);
	exec->code[exec->cur] = op;

	for (i = 0; format[i] != '\0'; i++) {
		switch (format[i]) {
			case IVM_EXEC_FORMAT_ESC_CHAR:
				if (format[i + 1] == 'i') {
					IF_INT_N_THEN_CALL_1(format, i + 2, SInt8)
					else IF_INT_N_THEN_CALL_2(format, i + 2, SInt16)
					else IF_INT_N_THEN_CALL_2(format, i + 2, SInt32)
					else IF_INT_N_THEN_CALL_2(format, i + 2, SInt64)
					/* if any of the branches is triggered, the loop is continue */
					/* else fallthrough */
				} else if (format[i + 1] == 's') {
					tmp_i
					= ivm_string_pool_register(exec->pool,
											   va_arg(args, const ivm_char_t *));
					WRITE_ARG(SInt32, tmp_i);
					n_cur += tmp;
					i += 1;
					break;
				} else if (format[i + 1] == IVM_EXEC_FORMAT_ESC_CHAR) {
					i++;
					/* two successive escape characters -- only write 1 */
					/* fallthrough */
				}
				/* fallthrough */
			default:
				while (exec->length <= n_cur)
					ivm_exec_addBuffer(exec);
				exec->code[n_cur++] = format[i];
		}
	}

	exec->cur = n_cur;

	return ret;
}

ivm_size_t
ivm_exec_addCode(ivm_exec_t *exec, ivm_opcode_t op, const ivm_char_t *format, ...)
{
	va_list args;
	ivm_size_t ret;

	va_start(args, format);
	ret = ivm_exec_addCode_c(exec, op, format, args);
	va_end(args);

	return ret;
}

ivm_size_t
ivm_exec_addOp(ivm_exec_t *exec,
			   ivm_opcode_t op, ...)
{
	va_list args;
	ivm_size_t ret;

	va_start(args, op);
	ret = ivm_exec_addCode_c(exec, op, ivm_op_table_getArg(op), args);
	va_end(args);

	return ret;
}

void
ivm_exec_rewrite(ivm_exec_t *exec,
				 ivm_size_t addr,
				 ivm_opcode_t op,
				 const ivm_char_t *format, ...)
{
	va_list args;
	ivm_size_t cur_back;

	va_start(args, format);
	
	cur_back = exec->cur;
	exec->cur = addr;
	ivm_exec_addCode_c(exec, op, format, args);
	exec->cur = cur_back;

	va_end(args);

	return;
}

void
ivm_exec_rewriteArg(ivm_exec_t *exec,
					ivm_size_t addr,
					const ivm_char_t *format, ...)
{
	va_list args;
	ivm_size_t cur_back;

	va_start(args, format);
	
	cur_back = exec->cur;
	exec->cur = addr;
	ivm_exec_addCode_c(exec, exec->code[addr], format, args);
	exec->cur = cur_back;

	va_end(args);

	return;
}


void
ivm_exec_toInstrBlock(ivm_exec_t *exec,
					  ivm_exec_t *dest)
{
	ivm_pc_t pc = 0, tmp;
	ivm_instr_t instr;
	ivm_byte_t op;

	while (pc < exec->cur) {
		op = exec->code[pc];

		MEM_INIT(&instr, sizeof(ivm_instr_t));
		instr.proc = ivm_op_table_getProc(op);
		tmp = ivm_op_table_getOffset(op);
		if (tmp > 1) {
			MEM_COPY(&instr.op1, &exec->code[pc + 1], tmp - 1);
		}

		ivm_exec_addInstr(dest, instr);

		pc += tmp;
	}

	return;
}

#endif
