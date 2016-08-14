#ifndef _IVM_VM_INLINE_CONTEXT_H_
#define _IVM_VM_INLINE_CONTEXT_H_

#include "pub/const.h"
#include "pub/com.h"
#include "pub/vm.h"

#include "std/string.h"

#include "../obj.h"
#include "../context.h"

IVM_COM_HEADER

IVM_INLINE
void
ivm_context_setSlot(ivm_context_t *ctx,
					struct ivm_vmstate_t_tag *state,
					const ivm_string_t *key,
					struct ivm_object_t_tag *value)
{
	if (!ctx->slots) {
		ctx->slots = ivm_slot_table_new(state);
	}

	ivm_slot_table_setSlot(ctx->slots, state, key, value);

	return;
}

IVM_INLINE
void
ivm_context_setSlot_r(ivm_context_t *ctx,
					  struct ivm_vmstate_t_tag *state,
					  const ivm_char_t *rkey,
					  struct ivm_object_t_tag *value)
{
	if (!ctx->slots) {
		ctx->slots = ivm_slot_table_new(state);
	}

	ivm_slot_table_setSlot_r(ctx->slots, state, rkey, value);

	return;
}

IVM_INLINE
void
ivm_context_setSlot_cc(ivm_context_t *ctx,
					   struct ivm_vmstate_t_tag *state,
					   const ivm_string_t *key,
					   struct ivm_object_t_tag *value,
					   ivm_instr_t *instr)
{
	ivm_slot_table_t *slots = ctx->slots;

	if (!slots) {
		/* check cache */
		/*
		if (ivm_slot_table_checkCacheValid(slots, instr)) {
			ivm_slot_table_setCacheSlot(state, instr, value);
			return;
		}
		} else {*/
		slots = ctx->slots = ivm_slot_table_new(state);
	}

	ivm_slot_table_setSlot_cc(slots, state, key, value, instr);

	return;
}

IVM_INLINE
ivm_object_t *
ivm_context_getSlot(ivm_context_t *ctx,
					struct ivm_vmstate_t_tag *state,
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
					   struct ivm_vmstate_t_tag *state,
					   const ivm_string_t *key,
					   ivm_instr_t *instr)
{
	ivm_slot_table_t *slots = ctx->slots;

	if (!slots) return IVM_NULL;

	/*if (ivm_slot_table_checkCacheValid(slots, instr)) {
		return ivm_slot_table_getCacheSlot(state, instr);
	}*/

	return ivm_slot_getValue(
		ivm_slot_table_getSlot_cc(ctx->slots, state, key, instr),
		state
	);
}

IVM_INLINE
ivm_bool_t
ivm_context_setExistSlot(ivm_context_t *ctx,
						 struct ivm_vmstate_t_tag *state,
						 const ivm_string_t *key,
						 ivm_object_t *value)
{
	ivm_slot_table_t *slots = ctx->slots;

	if (!slots) {
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
							struct ivm_vmstate_t_tag *state,
							const ivm_string_t *key,
							struct ivm_object_t_tag *value,
							ivm_instr_t *instr)
{
	ivm_slot_table_t *slots = ctx->slots;

	if (!slots) {
		/*if (ivm_slot_table_checkCacheValid(slots, instr)) {
			ivm_slot_table_setCacheSlot(state, instr, value);
			return IVM_TRUE;
		}
	} else {*/
		return IVM_FALSE;
	}

	return ivm_slot_table_setExistSlot_cc(
		ctx->slots, state,
		key, value, instr
	);
}

#define GET_CONTEXT(chain_sub) ((chain_sub)->slots)

#define ivm_ctchain_free(chain, state) (ivm_vmstate_dumpContext((state), (chain)))

IVM_INLINE
ivm_ctchain_t *
ivm_context_pool_alloc(ivm_context_pool_t *pool, ivm_int_t len)
{
	ivm_ctchain_t *ret;

	if (len < IVM_CONTEXT_POOL_MAX_CACHE_LEN) {
		ret = (ivm_ctchain_t *)
			  ivm_ptpool_alloc(pool->pools[len]);
	} else {
		ret = MEM_ALLOC(ivm_ctchain_getSize(len),
						ivm_ctchain_t *);
		IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("context chain"));
	}

	ivm_ref_init(ret);
	ret->len = len;
	ivm_ctchain_setGen(ret, 0);
	ivm_ctchain_setWB(ret, 0);

	return ret;
}

IVM_INLINE
void
ivm_context_pool_dump(ivm_context_pool_t *pool,
					  ivm_ctchain_t *chain)
{
	if (chain && !ivm_ref_dec(chain)) {
		if (chain->len < IVM_CONTEXT_POOL_MAX_CACHE_LEN) {
			ivm_ptpool_dump(pool->pools[chain->len], chain);
		} else {
			MEM_FREE(chain);
		}
	}

	return;
}

IVM_INLINE
void
ivm_context_pool_dumpAll(ivm_context_pool_t *pool)
{
	ivm_ptpool_t **i, **end;

	for (i = pool->pools,
		 end = i + IVM_CONTEXT_POOL_MAX_CACHE_LEN + 1;
		 i != end; i++) {
		ivm_ptpool_dumpAll(*i);
	}

	return;
}

IVM_INLINE
void
ivm_ctchain_setObjAt(ivm_ctchain_t *chain,
					 ivm_vmstate_t *state,
					 ivm_int_t i,
					 ivm_object_t *obj)
{
	if (obj) {
		IVM_WBCTX(
			state, chain,
			chain->chain[i].slots
			= IVM_OBJECT_GET(obj, SLOTS)
		);
	} else {
		chain->chain[i].slots = IVM_NULL;
	}

	return;
}

IVM_INLINE
void
ivm_ctchain_setLocal(ivm_ctchain_t *chain,
					 ivm_vmstate_t *state,
					 ivm_object_t *obj)
{
	if (obj) {
		IVM_WBCTX(
			state, chain,
			chain->chain[0].slots
			= IVM_OBJECT_GET(obj, SLOTS)
		);
	} else {
		chain->chain[0].slots = IVM_NULL;
	}

	return;
}

IVM_INLINE
void
ivm_ctchain_setGlobal(ivm_ctchain_t *chain,
					  ivm_vmstate_t *state,
					  ivm_object_t *obj)
{
	if (obj) {
		IVM_WBCTX(
			state, chain,
			chain->chain[chain->len - 1].slots
			= IVM_OBJECT_GET(obj, SLOTS)
		);
	} else {
		chain->chain[chain->len - 1].slots = IVM_NULL;
	}

	return;
}

/*
IVM_INLINE
void
ivm_ctchain_setSlotsAt(ivm_ctchain_t *chain,
					   ivm_vmstate_t *state,
					   ivm_int_t i,
					   ivm_slot_table_t *table)
{
	if (obj) {
		IVM_WBCTX(state, chain, table);
		chain->chain[i].slots = table;
	}

	return;
}
*/

IVM_INLINE
ivm_object_t *
ivm_ctchain_search_cc(ivm_ctchain_t *chain,
					  ivm_vmstate_t *state,
					  const ivm_string_t *key,
					  ivm_instr_t *instr)
{
	ivm_object_t *ret = IVM_NULL;
	ivm_context_t *i, *end;
	ivm_int_t j = 0;

	// IVM_TRACE("len: %d %p\n", chain->len, ivm_ctchain_contextLast(chain)->slots);
	for (i = ivm_ctchain_contextStart(chain),
		 end = i + chain->len;
		 i != end; i++, j++) {
		ret = ivm_context_getSlot_cc(i, state, key, instr);
		if (ret) {
			break;
		}
	}

	return ret;
}

IVM_INLINE
ivm_bool_t
ivm_ctchain_setExistSlot_cc(ivm_ctchain_t *chain,
							struct ivm_vmstate_t_tag *state,
							const ivm_string_t *key,
							ivm_object_t *val,
							ivm_instr_t *instr)
{
	ivm_bool_t ret = IVM_FALSE;
	ivm_context_t *i, *end;

	for (i = ivm_ctchain_contextStart(chain),
		 end = i + chain->len;
		 i != end; i++) {
		ret = ivm_context_setExistSlot_cc(i, state, key, val, instr);
		if (ret) break;
	}

	return ret;
}

#undef GET_CONTEXT

IVM_INLINE
ivm_ctchain_t *
ivm_ctchain_appendContext(ivm_ctchain_t *chain,
						  ivm_vmstate_t *state)
{
	ivm_ctchain_t *ret;

	if (chain) {
		ret = ivm_vmstate_allocContext(state, chain->len + 1);
		MEM_COPY(ivm_ctchain_contextAt(ret, 1),
				 ivm_ctchain_contextStart(chain),
				 ivm_ctchain_getContextSize(chain));
	} else {
		ret = ivm_vmstate_allocContext(state, 1);
	}

	ret->chain[0].slots = IVM_NULL;

	return ret;
}

IVM_INLINE
ivm_ctchain_t *
ivm_ctchain_clone(ivm_ctchain_t *chain,
				  ivm_vmstate_t *state)
{
	ivm_ctchain_t *ret = IVM_NULL;

	if (chain) {
		ret = ivm_vmstate_allocContext(state, chain->len);
		ivm_ref_init(ret);
		MEM_COPY(ivm_ctchain_contextStart(ret),
				 ivm_ctchain_contextStart(chain),
				 ivm_ctchain_getContextSize(chain));
	}

	return ret;
}

IVM_COM_END

#endif
