#include "pub/mem.h"
#include "runtime.h"
#include "context.h"
#include "err.h"

ivm_runtime_t *
ivm_runtime_new(ivm_exec_t *exec, ivm_ctchain_t *context)
{
	ivm_runtime_t *ret = MEM_ALLOC(sizeof(*ret));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("runtime"));

	ret->pc = 0;
	ret->exec = exec;
	ret->context = ivm_ctchain_clone(context);

	return ret;
}

void
ivm_runtime_free(ivm_runtime_t *runtime)
{
	if (runtime) {
		ivm_ctchain_free(runtime->context);
		MEM_FREE(runtime);
	}
	
	return;
}
