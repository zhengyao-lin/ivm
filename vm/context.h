#ifndef _IVM_VM_CONTEXT_H_
#define _IVM_VM_CONTEXT_H_

#include "pub/com.h"
#include "pub/const.h"

#include "std/pool.h"
#include "std/ref.h"

#include "obj.h"
#include "slot.h"

IVM_COM_HEADER

struct ivm_vmstate_t_tag;
struct ivm_object_t_tag;

typedef struct ivm_context_t_tag {
	ivm_slot_table_t *slots;
} ivm_context_t;

/*
	3 types of interfaces:
	1. setSlot
	2. getSlot
	3. setExistSlot

	suffix:
	1. \ -- normal
	2. _r -- using raw(char *) string as key
	3. _cc -- use cache
 */

#define ivm_context_getSlotTable(ctx) \
	((ctx)->slots)

#define ivm_context_setSlotTable(ctx, table) \
	((ctx)->slots = (table))

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
					   ivm_instr_cache_t *cache)
{
	ivm_slot_table_t *slots = ctx->slots;

	if (slots) {
		/* check cache */
		if (ivm_slot_table_checkCacheValid(slots, cache)) {
			ivm_slot_table_setCacheSlot(state, cache, value);
			return;
		}
	} else {
		slots = ctx->slots = ivm_slot_table_new(state);
	}

	ivm_slot_table_setSlot_cc(slots, state, key, value, cache);

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
					   ivm_instr_cache_t *cache)
{
	ivm_slot_table_t *slots = ctx->slots;

	if (!slots) return IVM_NULL;

	if (ivm_slot_table_checkCacheValid(slots, cache)) {
		return ivm_slot_table_getCacheSlot(state, cache);
	}

	return ivm_slot_getValue(
		ivm_slot_table_getSlot_cc(ctx->slots, state, key, cache),
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
							ivm_instr_cache_t *cache)
{
	ivm_slot_table_t *slots = ctx->slots;

	if (slots) {
		if (ivm_slot_table_checkCacheValid(slots, cache)) {
			ivm_slot_table_setCacheSlot(state, cache, value);
			return IVM_TRUE;
		}
	} else {
		return IVM_FALSE;
	}

	return ivm_slot_table_setExistSlot_cc(
		ctx->slots, state,
		key, value, cache
	);
}

/*
 * context chain:
 * -------------------------------
 * | head | local | ... | global |
 * -------------------------------
 */

typedef struct ivm_ctchain_t_tag {
	IVM_REF_HEADER
	ivm_uint_t len;
	ivm_context_t chain[];
} ivm_ctchain_t;

#define ivm_ctchain_getSize(len) \
	(sizeof(ivm_ctchain_t) + (sizeof(ivm_context_t) * (len)))

#define ivm_ctchain_getContextSize(chain) \
	(sizeof(ivm_context_t) * (chain)->len)

IVM_INLINE
ivm_context_t *
ivm_ctchain_contextStart(ivm_ctchain_t *chain) // local context
{
	return chain->chain;
}

IVM_INLINE
ivm_context_t *
ivm_ctchain_contextLast(ivm_ctchain_t *chain) // global context
{
	return chain->chain + chain->len - 1;
}

IVM_INLINE
ivm_context_t *
ivm_ctchain_contextEnd(ivm_ctchain_t *chain)
{
	return chain->chain + chain->len;
}

IVM_INLINE
ivm_context_t *
ivm_ctchain_contextAt(ivm_ctchain_t *chain,
					  ivm_int_t i)
{
	return chain->chain + i;
}

IVM_INLINE
void
ivm_ctchain_setObjAt(ivm_ctchain_t *chain,
					 ivm_int_t i,
					 ivm_object_t *obj)
{
	if (obj) {
		chain->chain[i].slots
		= IVM_OBJECT_GET(obj, SLOTS);
	}

	return;
}

IVM_INLINE
ivm_ctchain_t *
ivm_ctchain_addRef(ivm_ctchain_t *chain)
{
	if (chain) {
		ivm_ref_inc(chain);
	}

	return chain;
}

ivm_ctchain_t *
ivm_ctchain_new(struct ivm_vmstate_t_tag *state, ivm_int_t len);

ivm_ctchain_t *
ivm_ctchain_appendContext(ivm_ctchain_t *chain,
						  struct ivm_vmstate_t_tag *state);

ivm_ctchain_t *
ivm_ctchain_clone(ivm_ctchain_t *chain,
				  struct ivm_vmstate_t_tag *state);

#define ivm_ctchain_getLocal ivm_ctchain_contextStart
#define ivm_ctchain_getGlobal ivm_ctchain_contextLast


/*
	cache versions of search and setExistSlot are in inline/context.h
 */

ivm_object_t *
ivm_ctchain_search(ivm_ctchain_t *chain,
				   struct ivm_vmstate_t_tag *state,
				   const ivm_string_t *key);

ivm_bool_t
ivm_ctchain_setExistSlot(ivm_ctchain_t *chain,
						 struct ivm_vmstate_t_tag *state,
						 const ivm_string_t *key,
						 ivm_object_t *val);

typedef ivm_context_t *ivm_ctchain_iterator_t;

#define IVM_CTCHAIN_ITER_SET(iter, val) ((iter)->slots = val)
#define IVM_CTCHAIN_ITER_GET(iter) ((iter)->slots)
#define IVM_CTCHAIN_EACHPTR(chain, iter) \
	ivm_ctchain_iterator_t __ctx_end_##iter##__; \
	for ((iter) = ivm_ctchain_contextStart(chain), \
		 __ctx_end_##iter##__ = ivm_ctchain_contextEnd(chain); \
		 (iter) != __ctx_end_##iter##__; (iter)++)

typedef struct {
	ivm_ptpool_t *pools[IVM_CONTEXT_POOL_MAX_CACHE_LEN];
} ivm_context_pool_t;

ivm_context_pool_t *
ivm_context_pool_new(ivm_size_t ecount);

void
ivm_context_pool_free(ivm_context_pool_t *pool);

void
ivm_context_pool_init(ivm_context_pool_t *pool,
					  ivm_size_t ecount);

void
ivm_context_pool_destruct(ivm_context_pool_t *pool);

IVM_COM_END

#endif
