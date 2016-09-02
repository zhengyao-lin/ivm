#include "pub/com.h"
#include "pub/vm.h"
#include "pub/err.h"
#include "pub/inlines.h"

#include "std/mem.h"
#include "std/ref.h"

#include "context.h"

ivm_context_t *
ivm_context_new(ivm_vmstate_t *state,
				ivm_context_t *prev)
{
	ivm_context_t *ret = ivm_vmstate_allocContext(state);

	if (prev) ivm_context_addRef(prev);
	ret->prev = prev;
	STD_INIT(&ret->slots, sizeof(ret->slots) + sizeof(ret->mark));

	return ret;
}

void
ivm_context_free(ivm_context_t *ctx,
				 ivm_vmstate_t *state)
{
	ivm_context_t *tmp;

	while (ctx && !--ctx->mark.ref) {
		tmp = ctx->prev;
		ivm_vmstate_dumpContext(state, ctx);
		ctx = tmp;
	}

	return;
}

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
