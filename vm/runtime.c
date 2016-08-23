#include "pub/mem.h"
#include "pub/err.h"
#include "pub/vm.h"
#include "pub/inlines.h"

#include "runtime.h"
#include "context.h"
#include "call.h"
#include "coro.h"

ivm_runtime_t *
ivm_runtime_new(ivm_vmstate_t *state)
{
	ivm_runtime_t *ret = MEM_ALLOC_INIT(sizeof(*ret),
										ivm_runtime_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("runtime"));

	return ret;
}

void
ivm_runtime_invoke(ivm_runtime_t *runtime,
				   ivm_vmstate_t *state,
				   const ivm_exec_t *exec,
				   ivm_context_t *ctx)
{
	runtime->ctx = ivm_context_addRef(ctx);
	runtime->ip = exec ? ivm_exec_instrPtrStart(exec) : IVM_NULL;
	runtime->bp = runtime->sp;
	runtime->cat = IVM_NULL;
	runtime->no_reg = IVM_FALSE;

	return;
}

ivm_context_t *
ivm_runtime_appendContextNode(ivm_runtime_t *runtime,
							  ivm_vmstate_t *state)
{
	ivm_context_t *orig = runtime->ctx;

	runtime->ctx
	= ivm_context_addRef(ivm_context_new(state, orig));

	ivm_context_free(orig, state);

	return runtime->ctx;
}

ivm_context_t *
ivm_runtime_removeContextNode(ivm_runtime_t *runtime,
							  ivm_vmstate_t *state)
{
	ivm_context_t *orig = runtime->ctx;

	runtime->ctx
	= ivm_context_addRef(ivm_context_getPrev(orig));

	IVM_ASSERT(runtime->ctx, IVM_ERROR_MSG_CONTEXT_NO_PREV_NODE);

	ivm_context_free(orig, state);

	return runtime->ctx;
}
