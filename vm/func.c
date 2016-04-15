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

	ret->closure = ivm_ctchain_clone(context);
	ret->body = body;
	ret->intsig = intsig;

	return ret;
}

void
ivm_function_free(ivm_function_t *func)
{
	if (func) {
		ivm_ctchain_free(func->closure);
		MEM_FREE(func);
	}

	return;
}
