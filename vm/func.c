#include "pub/mem.h"
#include "func.h"
#include "context.h"
#include "err.h"

ivm_function_t *
ivm_function_new(ivm_ctchain_t *context,
				 ivm_exec_t *body,
				 ivm_signal_mask_t intsig)
{
	ivm_function_t *ret = MEM_ALLOC(sizeof(*ret));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("function"));

	ret->is_native = IVM_FALSE;
	ret->u.f.closure = ivm_ctchain_clone(context);
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
