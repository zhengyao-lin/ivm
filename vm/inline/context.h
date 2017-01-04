#ifndef _IVM_VM_INLINE_CONTEXT_H_
#define _IVM_VM_INLINE_CONTEXT_H_

#include "pub/const.h"
#include "pub/com.h"
#include "pub/vm.h"

#include "std/string.h"

#include "vm/obj.h"
#include "vm/context.h"

IVM_COM_HEADER

IVM_INLINE
ivm_slot_table_t *
ivm_context_initSlots(ivm_context_t *ctx,
					  ivm_vmstate_t *state)
{
	if (!ctx->slots) {
		// ctx->obj = ivm_object_new_c(state, 0);
		ctx->slots = ivm_slot_table_newAt(state, ivm_context_getGen(ctx));
		// IVM_WBCTX(state, ctx, ctx->obj);
	}

	return ctx->slots;
}

IVM_INLINE
ivm_context_t *
ivm_context_new(ivm_vmstate_t *state,
				ivm_context_t *prev)
{
	ivm_context_t *ret = ivm_vmstate_allocContext(state);

	STD_INIT(ret, sizeof(*ret));
	ret->prev = ivm_context_addRef(prev);

	// IVM_TRACE("context created: %p\n", (void *)ret);

	return ret;
}

IVM_INLINE
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

IVM_INLINE
void
ivm_context_setSlot(ivm_context_t *ctx,
					ivm_vmstate_t *state,
					const ivm_string_t *key,
					ivm_object_t *value)
{
	ivm_slot_table_setSlot(ivm_context_initSlots(ctx, state), state, key, value);
	return;
}

IVM_INLINE
void
ivm_context_setSlot_r(ivm_context_t *ctx,
					  ivm_vmstate_t *state,
					  const ivm_char_t *rkey,
					  ivm_object_t *value)
{
	ivm_slot_table_setSlot_r(ivm_context_initSlots(ctx, state), state, rkey, value);
	return;
}

IVM_INLINE
void
ivm_context_setSlot_cc(ivm_context_t *ctx,
					   ivm_vmstate_t *state,
					   const ivm_string_t *key,
					   ivm_object_t *value,
					   ivm_instr_t *instr)
{
	ivm_slot_table_setSlot_cc(ivm_context_initSlots(ctx, state), state, key, value, instr);
	return;
}

IVM_INLINE
ivm_object_t *
ivm_context_getSlot(ivm_context_t *ctx,
					ivm_vmstate_t *state,
					const ivm_string_t *key)
{
	return ctx->slots ? ivm_slot_getValue(ivm_slot_table_getSlot(ctx->slots, state, key), state) : IVM_NULL;
}

IVM_INLINE
ivm_object_t *
ivm_context_getSlot_cc(ivm_context_t *ctx,
					   ivm_vmstate_t *state,
					   const ivm_string_t *key,
					   ivm_instr_t *instr)
{
	if (!ctx->slots) return IVM_NULL;

	return ivm_slot_getValue(ivm_slot_table_getSlot_cc(ctx->slots, state, key, instr), state);
}

IVM_INLINE
ivm_bool_t
ivm_context_setExistSlot(ivm_context_t *ctx,
						 ivm_vmstate_t *state,
						 const ivm_string_t *key,
						 ivm_object_t *value)
{
	if (!ctx->slots) return IVM_FALSE;

	return ivm_slot_table_setExistSlot(
		ctx->slots, state,
		key, value
	);
}

IVM_INLINE
ivm_bool_t
ivm_context_setExistSlot_cc(ivm_context_t *ctx,
							ivm_vmstate_t *state,
							const ivm_string_t *key,
							ivm_object_t *value,
							ivm_instr_t *instr)
{
	if (!ctx->slots) return IVM_FALSE;

	return ivm_slot_table_setExistSlot_cc(
		ctx->slots, state,
		key, value, instr
	);
}

IVM_INLINE
ivm_object_t *
ivm_context_getObject(ivm_context_t *ctx,
					  ivm_vmstate_t *state)
{
	if (!ctx->obj) {
		ivm_context_initSlots(ctx, state);

		ctx->obj = ivm_object_new_t(state, ctx->slots);
		IVM_WBCTX_OBJ(state, ctx, ctx->obj);
	}

	return ctx->obj;
}

IVM_INLINE
ivm_object_t *
ivm_context_getObject_c(ivm_context_t *ctx)
{
	return ctx->obj;
}

IVM_INLINE
ivm_slot_table_t *
ivm_context_getSlots_c(ivm_context_t *ctx)
{
	return ctx->slots;
}

IVM_INLINE
void
ivm_context_setObject(ivm_context_t *ctx,
					  ivm_vmstate_t *state,
					  ivm_object_t *obj)
{
	ctx->obj = obj;

	if (obj) {
		IVM_WBCTX_OBJ(state, ctx, obj);

		ctx->slots = IVM_OBJECT_GET(obj, SLOTS);
		
		if (!ctx->slots) {
			ivm_object_expandSlotTable(obj, state, 0);
			ctx->slots = IVM_OBJECT_GET(obj, SLOTS);
		}
	} else {
		ctx->slots = IVM_NULL;
	}

	return;
}

/* no write barrier(called by gc only) */
IVM_INLINE
void
_ivm_context_updateObject_c(ivm_context_t *ctx,
							ivm_object_t *obj)
{
	ctx->obj = obj;
	return;
}

IVM_INLINE
void
_ivm_context_updateSlots_c(ivm_context_t *ctx,
						  ivm_slot_table_t *slots)
{
	ctx->slots = slots;
	return;
}

IVM_INLINE
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

IVM_INLINE
ivm_object_t *
ivm_context_search_cc(ivm_context_t *ctx,
					  ivm_vmstate_t *state,
					  const ivm_string_t *key,
					  ivm_instr_t *instr)
{
	ivm_object_t *ret = IVM_NULL;

	do {
		ret = ivm_context_getSlot_cc(ctx, state, key, instr);
		if (ret) {
			// IVM_TRACE("end!!!\n");
			return ret;
		}
		ctx = ctx->prev;
	} while (ctx);
	// IVM_TRACE("end!!!\n");

	return IVM_NULL;
}

IVM_INLINE
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

IVM_INLINE
ivm_bool_t
ivm_context_searchAndSetExistSlot_cc(ivm_context_t *ctx,
									 ivm_vmstate_t *state,
									 const ivm_string_t *key,
									 ivm_object_t *val,
									 ivm_instr_t *instr)
{
	do {
		if (ivm_context_setExistSlot_cc(ctx, state, key, val, instr))
			return IVM_TRUE;
		ctx = ctx->prev;
	} while (ctx);

	return IVM_FALSE;
}

IVM_INLINE
void
ivm_context_expandSlotTable(ivm_context_t *ctx,
							struct ivm_vmstate_t_tag *state,
							ivm_size_t size)
{
	ivm_slot_table_expandTo(ivm_context_initSlots(ctx, state), state, size);
	return;
}

IVM_COM_END

#endif
