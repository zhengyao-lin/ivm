#include "pub/err.h"
#include "pub/vm.h"
#include "pub/inlines.h"

#include "std/mem.h"

#include "runtime.h"
#include "context.h"
#include "call.h"
#include "coro.h"

ivm_runtime_t *
ivm_runtime_new(ivm_vmstate_t *state)
{
	ivm_runtime_t *ret = STD_ALLOC_INIT(sizeof(*ret));

	IVM_MEMCHECK(ret);

	return ret;
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
