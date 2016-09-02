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
void
ivm_context_setSlot(ivm_context_t *ctx,
					ivm_vmstate_t *state,
					const ivm_string_t *key,
					ivm_object_t *value)
{
	if (!ctx->slots) {
		IVM_WBCTX(state, ctx, ctx->slots = ivm_slot_table_new(state));
	}

	ivm_slot_table_setSlot(ctx->slots, state, key, value);

	return;
}

IVM_INLINE
void
ivm_context_setSlot_r(ivm_context_t *ctx,
					  ivm_vmstate_t *state,
					  const ivm_char_t *rkey,
					  ivm_object_t *value)
{
	if (!ctx->slots) {
		IVM_WBCTX(state, ctx, ctx->slots = ivm_slot_table_new(state));
	}

	ivm_slot_table_setSlot_r(ctx->slots, state, rkey, value);

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
	if (!ctx->slots) {
		IVM_WBCTX(state, ctx, ctx->slots = ivm_slot_table_new(state));
	}

	ivm_slot_table_setSlot_cc(ctx->slots, state, key, value, instr);

	return;
}

IVM_INLINE
ivm_object_t *
ivm_context_getSlot(ivm_context_t *ctx,
					ivm_vmstate_t *state,
					const ivm_string_t *key)
{
	return ctx->slots ? ivm_slot_getValue(
		ivm_slot_table_getSlot(ctx->slots, state, key),
		state
	) : IVM_NULL;
}

IVM_INLINE
ivm_object_t *
ivm_context_getSlot_cc(ivm_context_t *ctx,
					   ivm_vmstate_t *state,
					   const ivm_string_t *key,
					   ivm_instr_t *instr)
{
	if (!ctx->slots) return IVM_NULL;

	return ivm_slot_getValue(
		ivm_slot_table_getSlot_cc(ctx->slots, state, key, instr),
		state
	);
}

IVM_INLINE
ivm_bool_t
ivm_context_setExistSlot(ivm_context_t *ctx,
						 ivm_vmstate_t *state,
						 const ivm_string_t *key,
						 ivm_object_t *value)
{
	if (!ctx->slots) {
		return IVM_FALSE;
	}

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
	if (!ctx->slots) {
		return IVM_FALSE;
	}

	return ivm_slot_table_setExistSlot_cc(
		ctx->slots, state,
		key, value, instr
	);
}

#define ivm_context_getSlotTable(ctx) \
	((ctx)->slots)

IVM_INLINE
void
ivm_context_linkToObject(ivm_context_t *ctx,
						 ivm_vmstate_t *state,
						 ivm_object_t *obj)
{
	ivm_slot_table_t *slots = IVM_OBJECT_GET(obj, SLOTS);

	slots = ivm_slot_table_copyOnWrite(slots, state);
	IVM_OBJECT_SET(obj, SLOTS, slots);

	IVM_WBCTX(state, ctx, ctx->slots = slots);

	return;
}

/* no write barrier(called by gc only) */
IVM_INLINE
void
_ivm_context_setSlotTable_c(ivm_context_t *ctx,
						    ivm_slot_table_t *table)
{
	ctx->slots = table;
	return;
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
		if (ret) break;
		ctx = ctx->prev;
	} while (ctx);

	return ret;
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

IVM_COM_END

#endif
