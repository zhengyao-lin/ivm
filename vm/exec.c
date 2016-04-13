#include <stdarg.h>
#include "pub/mem.h"
#include "exec.h"
#include "err.h"

ivm_exec_t *
ivm_exec_new()
{
	ivm_exec_t *ret = MEM_ALLOC(sizeof(*ret));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("executable"));

	ret->length = IVM_DEFAULT_PREALLOC_EXEC_SIZE;
	ret->cur = 0;
	ret->code = MEM_ALLOC_INIT(sizeof(*ret->code)
							   * IVM_DEFAULT_PREALLOC_EXEC_SIZE);

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
							 * (exec->length + IVM_DEFAULT_PREALLOC_EXEC_SIZE));
	IVM_ASSERT(exec->code, IVM_ERROR_MSG_FAILED_ALLOC_NEW("new code buffer in executable"));
	exec->length += IVM_DEFAULT_PREALLOC_EXEC_SIZE;

	return;
}

void
ivm_exec_addCode(ivm_exec_t *exec, ivm_opcode_t op, ivm_size_t arg_count, ...)
{
	va_list args;
	ivm_size_t i, n_cur;

	va_start(args, arg_count);

	n_cur = exec->cur + arg_count + 1;
	while (exec->length < n_cur)
		ivm_exec_addBuffer(exec);

	exec->code[exec->cur] = op;
	for (i = exec->cur + 1;
		 i < n_cur; i++) {
		exec->code[i] = va_arg(args, ivm_int_t);
	}
	exec->cur = n_cur;

	va_end(args);

	return;
}
