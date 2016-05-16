#include "pub/mem.h"
#include "runtime.h"
#include "context.h"
#include "call.h"
#include "coro.h"
#include "err.h"

ivm_runtime_t *
ivm_runtime_new()
{
	ivm_runtime_t *ret = MEM_ALLOC_INIT(sizeof(*ret),
										ivm_runtime_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("runtime"));

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

void
ivm_runtime_invoke(ivm_runtime_t *runtime,
				   ivm_exec_t *exec,
				   ivm_ctchain_t *context)
{
	runtime->pc = 0;
	runtime->exec = exec;
	runtime->context = context
					   ? ivm_ctchain_clone(context)
					   : ivm_ctchain_new();

	return;
}

ivm_caller_info_t *
ivm_runtime_getCallerInfo(ivm_runtime_t *runtime,
						  ivm_coro_t *coro)
{
	return ivm_caller_info_new(runtime,
							   ivm_coro_stackTop(coro));
}

void
ivm_runtime_restore(ivm_runtime_t *runtime, ivm_coro_t *coro,
					struct ivm_caller_info_t_tag *info)
{
	ivm_ctchain_free(runtime->context);

	MEM_COPY(runtime, info, sizeof(*runtime));

	return;
}
