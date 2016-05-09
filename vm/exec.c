#include <stdarg.h>
#include "pub/mem.h"
#include "exec.h"
#include "str.h"
#include "byte.h"
#include "err.h"

ivm_exec_t *
ivm_exec_new(ivm_string_pool_t *pool)
{
	ivm_exec_t *ret = MEM_ALLOC(sizeof(*ret));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("executable"));

	ret->pool = pool;
	ret->length = IVM_DEFAULT_EXEC_BUFFER_SIZE;
	ret->cur = 0;
	ret->code = MEM_ALLOC_INIT(sizeof(*ret->code)
							   * IVM_DEFAULT_EXEC_BUFFER_SIZE);

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
								 * exec->cur);
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
							 * (exec->length + IVM_DEFAULT_EXEC_BUFFER_SIZE));
	IVM_ASSERT(exec->code, IVM_ERROR_MSG_FAILED_ALLOC_NEW("new code buffer in executable"));
	exec->length += IVM_DEFAULT_EXEC_BUFFER_SIZE;

	return;
}

#define ESC_CHAR '$'

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

static
ivm_size_t
ivm_exec_addCode_c(ivm_exec_t *exec, ivm_opcode_t op, ivm_char_t *format, va_list args)
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
			case ESC_CHAR:
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
				} else if (format[i + 1] == ESC_CHAR) {
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
ivm_exec_addCode(ivm_exec_t *exec, ivm_opcode_t op, ivm_char_t *format, ...)
{
	va_list args;
	ivm_size_t ret;

	va_start(args, format);
	ret = ivm_exec_addCode_c(exec, op, format, args);
	va_end(args);

	return ret;
}

void
ivm_exec_rewrite(ivm_exec_t *exec,
				 ivm_size_t addr,
				 ivm_opcode_t op,
				 ivm_char_t *format, ...)
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
					ivm_char_t *format, ...)
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
