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
	struct ivm_context_t_tag *prev;
	ivm_slot_table_t *slots;
	struct {
		ivm_uint_t ref;
		ivm_bool_t gen;
		ivm_bool_t wb;
	} mark;
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

#define ivm_context_getGen(ctx) ((ctx)->mark.gen)
#define ivm_context_setGen(ctx, val) ((ctx)->mark.gen = (val))

#define ivm_context_getWB(ctx) ((ctx)->mark.wb)
#define ivm_context_setWB(ctx, val) ((ctx)->mark.wb = (val))

#define ivm_context_getPrev(ctx) ((ctx)->prev)

IVM_INLINE
ivm_context_t *
ivm_context_addRef(ivm_context_t *ctx)
{
	if (ctx) {
		ctx->mark.ref++;
	}

	return ctx;
}

ivm_context_t *
ivm_context_new(struct ivm_vmstate_t_tag *state,
				ivm_context_t *prev);

void
ivm_context_free(ivm_context_t *ctx,
				 struct ivm_vmstate_t_tag *state);

IVM_INLINE
ivm_context_t *
ivm_context_getGlobal(ivm_context_t *ctx)
{
	for (; ctx->prev; ctx = ctx->prev);
	return ctx;
}

/*
	cache versions of search and searchAndSetExistSlot are in inline/context.h
 */

ivm_object_t *
ivm_context_search(ivm_context_t *ctx,
				   struct ivm_vmstate_t_tag *state,
				   const ivm_string_t *key);

ivm_bool_t
ivm_context_searchAndSetExistSlot(ivm_context_t *ctx,
								  struct ivm_vmstate_t_tag *state,
								  const ivm_string_t *key,
								  ivm_object_t *val);

IVM_INLINE
void
ivm_context_expandSlotTable(ivm_context_t *ctx,
							struct ivm_vmstate_t_tag *state,
							ivm_size_t size)
{
	if (ctx->slots) {
		ivm_slot_table_expandTo(ctx->slots, state, size);
	} else {
		ctx->slots = ivm_slot_table_newAt_c(state, size, ctx->mark.gen);
	}

	return;
}

typedef ivm_ptpool_t ivm_context_pool_t;

#define ivm_context_pool_new(count) (ivm_ptpool_new((count), sizeof(ivm_context_t)))
#define ivm_context_pool_free ivm_ptpool_free
#define ivm_context_pool_init(pool, count) (ivm_ptpool_init((pool), (count), sizeof(ivm_context_t)))
#define ivm_context_pool_destruct ivm_ptpool_destruct
#define ivm_context_pool_alloc(pool) ((ivm_context_t *)ivm_ptpool_alloc(pool))
#define ivm_context_pool_dump ivm_ptpool_dump
#define ivm_context_pool_dumpAll ivm_ptpool_dumpAll

IVM_COM_END

#endif
