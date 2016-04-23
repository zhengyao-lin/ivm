#include "pub/mem.h"
#include "func.h"
#include "context.h"
#include "runtime.h"
#include "vm.h"
#include "call.h"
#include "coro.h"
#include "err.h"

ivm_function_t *
ivm_function_new(ivm_ctchain_t *context,
				 ivm_exec_t *body,
				 ivm_signal_mask_t intsig)
{
	ivm_function_t *ret = MEM_ALLOC(sizeof(*ret));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("function"));

	ret->is_native = IVM_FALSE;
	ret->u.f.closure = context
					   ? ivm_ctchain_clone(context)
					   : ivm_ctchain_new();
	ret->u.f.body = body;
	ret->intsig = intsig;

	return ret;
}

ivm_function_t *
ivm_function_newNative(ivm_native_function_t func,
					   ivm_signal_mask_t intsig)
{
	ivm_function_t *ret = MEM_ALLOC(sizeof(*ret));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("native function"));

	ret->is_native = IVM_TRUE;
	ret->u.native = func;
	ret->intsig = intsig;

	return ret;
}

void
ivm_function_free(ivm_function_t *func)
{
	if (func) {
		if (!func->is_native) {
			ivm_ctchain_free(func->u.f.closure);
		}

		MEM_FREE(func);
	}

	return;
}

ivm_runtime_t *
ivm_function_createRuntime(ivm_function_t *func)
{
	if (!func) return IVM_NULL;
	
	if (func->is_native)
		return ivm_runtime_new(IVM_NULL, IVM_NULL);

	return ivm_runtime_new(func->u.f.body, func->u.f.closure);
}

ivm_caller_info_t *
ivm_function_invoke(ivm_function_t *func, ivm_coro_t *coro)
{
	if (func->is_native)
		return IVM_NULL;

	return ivm_runtime_invoke(coro->runtime, coro, func->u.f.body, func->u.f.closure);
}

ivm_object_t *
ivm_function_callNative(ivm_function_t *func,
						ivm_vmstate_t *state,
						ivm_ctchain_t *context, ivm_object_t *base,
						ivm_argc_t argc, ivm_object_t **argv)
{
	return func->u.native(state, context, base, argc, argv);
}
