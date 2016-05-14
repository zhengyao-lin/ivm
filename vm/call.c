#include "pub/mem.h"
#include "call.h"
#include "context.h"
#include "runtime.h"
#include "err.h"

ivm_caller_info_t *
ivm_caller_info_new(ivm_runtime_t *runtime,
					ivm_size_t st_top)
{
	ivm_caller_info_t *ret = MEM_ALLOC(sizeof(*ret));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("caller info"));

	MEM_COPY(ret, runtime, sizeof(*runtime));
	ret->st_top = st_top;

	return ret;
}

void
ivm_caller_info_free(ivm_caller_info_t *info)
{
	MEM_FREE(info);
	return;
}
