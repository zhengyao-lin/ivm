#include "pub/mem.h"
#include "runtime.h"
#include "context.h"
#include "call.h"
#include "coro.h"
#include "vm.h"
#include "err.h"

ivm_runtime_t *
ivm_runtime_new(ivm_vmstate_t *state)
{
	ivm_runtime_t *ret = MEM_ALLOC_INIT(sizeof(*ret),
										ivm_runtime_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("runtime"));

	return ret;
}

void
ivm_runtime_free(ivm_runtime_t *runtime,
				 ivm_vmstate_t *state)
{
	if (runtime) {
		ivm_ctchain_free(runtime->context, state);
		MEM_FREE(runtime);
	}
	
	return;
}

void
ivm_runtime_invoke(ivm_runtime_t *runtime,
				   ivm_vmstate_t *state,
				   ivm_exec_t *exec,
				   ivm_ctchain_t *context)
{
	runtime->ip = exec ? ivm_exec_instrPtrStart(exec) : IVM_NULL;
	runtime->exec = exec;
	runtime->context = context;

	return;
}

ivm_frame_t *
ivm_runtime_getFrame(ivm_runtime_t *runtime,
					 ivm_vmstate_t *state,
					 ivm_coro_t *coro)
{
	return ivm_frame_new(state,
						 runtime,
						 ivm_coro_stackTop(coro));
}

void
ivm_runtime_restore(ivm_runtime_t *runtime,
					ivm_vmstate_t *state,
					ivm_coro_t *coro,
					ivm_frame_t *frame)
{
	ivm_ctchain_free(runtime->context, state);

	MEM_COPY(runtime, frame, sizeof(*runtime));

	return;
}
