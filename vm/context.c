#include "pub/com.h"
#include "pub/vm.h"
#include "pub/err.h"
#include "pub/inlines.h"

#include "std/mem.h"
#include "std/ref.h"

#include "context.h"

ivm_object_t *
ivm_context_search(ivm_context_t *ctx,
				   ivm_vmstate_t *state,
				   const ivm_string_t *key)
{
	ivm_object_t *ret = IVM_NULL;

	do {
		ret = ivm_context_getSlot(ctx, state, key);
		if (ret) break;
		ctx = ctx->prev;
	} while (ctx);

	return ret;
}

ivm_bool_t
ivm_context_searchAndSetExistSlot(ivm_context_t *ctx,
								  ivm_vmstate_t *state,
								  const ivm_string_t *key,
								  ivm_object_t *val)
{
	do {
		if (ivm_context_setExistSlot(ctx, state, key, val))
			return IVM_TRUE;
		ctx = ctx->prev;
	} while (ctx);

	return IVM_FALSE;
}
