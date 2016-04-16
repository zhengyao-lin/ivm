#include "pub/mem.h"
#include "coro.h"
#include "stack.h"
#include "call.h"
#include "vm.h"
#include "err.h"

ivm_coro_t *
ivm_coro_new()
{
	ivm_coro_t *ret = MEM_ALLOC(sizeof(*ret));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("coroutine"));

	ret->stack = ivm_vmstack_new();
	ret->call_st = ivm_call_stack_new();
	ret->runtime = IVM_NULL;

	return ret;
}

void
ivm_coro_free(ivm_coro_t *coro)
{
	if (coro) {
		ivm_call_stack_foreach(coro->call_st, (ivm_stack_foreach_proc_t)ivm_caller_info_free);
		ivm_vmstack_free(coro->stack);
		ivm_call_stack_free(coro->call_st);
		ivm_runtime_free(coro->runtime);
		MEM_FREE(coro);
	}

	return;
}

ivm_object_t *
ivm_coro_start(ivm_coro_t *coro, ivm_vmstate_t *state, ivm_function_t *root)
{
	ivm_object_t *ret = IVM_NULL;

	if (!coro->runtime)
		coro->runtime = ivm_function_createRuntime(root);

	if (ivm_function_isNative(root)) {
		ret = ivm_function_callNative(root, state, coro->runtime->context, NULL, 0, NULL);
	}

	return ret ? ret : IVM_NULL_OBJ(state);
}
