#include "pub/mem.h"
#include "call.h"
#include "context.h"
#include "err.h"

ivm_caller_info_t *
ivm_caller_info_new(ivm_exec_t *exec,
					ivm_size_t st_top,
					ivm_pc_t pc,
					ivm_ctchain_t *context)
{
	ivm_caller_info_t *ret = MEM_ALLOC(sizeof(*ret));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("caller info"));

	ret->exec = exec;
	ret->st_top = st_top;
	ret->pc = pc;
	ret->context = context;

	return ret;
}

void
ivm_caller_info_free(ivm_caller_info_t *info)
{
	MEM_FREE(info);
	return;
}
