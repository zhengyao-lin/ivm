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
	ivm_runtime_t *ret = STD_ALLOC_INIT(sizeof(*ret),
										ivm_runtime_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("runtime"));

	return ret;
}

void
ivm_runtime_invokeNative(ivm_runtime_t *runtime,
						 ivm_vmstate_t *state,
						 ivm_context_t *ctx)
{
	runtime->ctx = ivm_context_addRef(ctx);
	runtime->bp = runtime->sp;

	IVM_FRAME_INIT_HEADER(runtime);

	return;
}

ivm_instr_t *
ivm_runtime_invoke(ivm_runtime_t *runtime,
				   ivm_vmstate_t *state,
				   const ivm_exec_t *exec,
				   ivm_context_t *ctx)
{
	runtime->ctx = ivm_context_addRef(ctx);
	runtime->bp = runtime->sp;

	IVM_FRAME_INIT_HEADER(runtime);

	runtime->offset = ivm_exec_offset(exec);

	return runtime->ip = ivm_exec_instrPtrStart(exec);
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
