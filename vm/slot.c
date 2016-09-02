#include <stdio.h>

#include "std/mem.h"
#include "pub/com.h"
#include "pub/const.h"
#include "pub/err.h"
#include "pub/vm.h"
#include "pub/inlines.h"

#include "std/hash.h"
#include "std/string.h"
#include "std/heap.h"
#include "std/bit.h"

#include "slot.h"
#include "obj.h"

#define SET_BIT_FALSE IVM_BIT_SET_FALSE
#define SET_BIT_TRUE IVM_BIT_SET_TRUE

IVM_PRIVATE
IVM_INLINE
void
_ivm_slot_table_init(ivm_slot_table_t *table,
					 ivm_vmstate_t *state,
					 ivm_size_t init,
					 ivm_int_t gen)
{
	STD_INIT(&table->mark, sizeof(table->mark));

	ivm_slot_table_setGen(table, gen);

	if (init < IVM_DEFAULT_SLOT_TABLE_SIZE) {
		init = IVM_DEFAULT_SLOT_TABLE_SIZE;
	}

#if IVM_USE_HASH_TABLE_AS_SLOT_TABLE
	if (init >= IVM_DEFAULT_SLOT_TABLE_TO_HASH_THRESHOLD) {
		init <<= 1; // lower load ratio
		SET_BIT_TRUE(table->mark.sub.is_hash);
	}
#endif

	table->size = init;
	ivm_slot_table_updateUID(table, state);
	table->tabl = ivm_vmstate_allocAt(state,
									  sizeof(*table->tabl)
									  * init,
									  gen);
	table->oops = IVM_NULL;
	// table->mark.sub.oop_count = 0;
	STD_INIT(table->tabl,
			 sizeof(*table->tabl)
			 * init);

	return;
}

ivm_slot_table_t *
ivm_slot_table_new(ivm_vmstate_t *state)
{
	ivm_slot_table_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	_ivm_slot_table_init(ret, state, 0, 0);

	return ret;
}

ivm_slot_table_t *
ivm_slot_table_newAt(ivm_vmstate_t *state, ivm_int_t gen)
{
	ivm_slot_table_t *ret = ivm_vmstate_allocAt(state, sizeof(*ret), gen);

	_ivm_slot_table_init(ret, state, 0, gen);

	return ret;
}

ivm_slot_table_t *
ivm_slot_table_new_c(ivm_vmstate_t *state,
					 ivm_size_t prealloc)
{
	ivm_slot_table_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	_ivm_slot_table_init(ret, state, prealloc, 0);

	return ret;
}

ivm_slot_table_t *
ivm_slot_table_copy(ivm_slot_table_t *table,
					ivm_vmstate_t *state,
					ivm_heap_t *heap)
{
	ivm_slot_table_t *ret;

	ret = ivm_heap_addCopy(heap, table, sizeof(*table));

	ivm_slot_table_setWB(ret, 0);
	ivm_slot_table_setCopy(ret, IVM_NULL);

	ivm_slot_table_updateUID(ret, state);
	ret->tabl = ivm_heap_alloc(heap,
							   sizeof(*ret->tabl)
							   * ret->size);
	if (ret->oops) {
		ret->oops = ivm_heap_addCopy(
			heap, ret->oops,
			sizeof(*ret->oops) * ret->mark.sub.oop_count
		);
	}

	STD_MEMCPY(ret->tabl,
			 table->tabl,
			 sizeof(*ret->tabl)
			 * ret->size);

	return ret;
}

ivm_slot_table_t *
_ivm_slot_table_copy_state(ivm_slot_table_t *table,
						   ivm_vmstate_t *state)
{
	ivm_slot_table_t *ret;

	ret = ivm_vmstate_addCopy(state, table, sizeof(*table));

	ivm_slot_table_setWB(ret, 0);
	ivm_slot_table_setGen(ret, 0);
	ivm_slot_table_setCopy(ret, IVM_NULL);

	ivm_slot_table_updateUID(ret, state);
	ret->tabl = ivm_vmstate_alloc(state,
								  sizeof(*ret->tabl)
								  * ret->size);
	if (ret->oops) {
		ret->oops = ivm_vmstate_addCopy(
			state, ret->oops,
			sizeof(*ret->oops) * ret->mark.sub.oop_count
		);
	}

	STD_MEMCPY(ret->tabl,
			 table->tabl,
			 sizeof(*ret->tabl)
			 * ret->size);

	return ret;
}

#define IS_EMPTY_SLOT(slot) (!(slot)->k)

ivm_bool_t
_ivm_slot_table_expand(ivm_slot_table_t *table,
					   ivm_vmstate_t *state) /* includes rehashing */
{
	ivm_size_t osize = table->size,
			   dsize = osize << 1; /* dest size */
	ivm_slot_t *otable = table->tabl;
	ivm_slot_t *i, *end;
	ivm_bool_t ret = IVM_FALSE;

	ivm_slot_table_updateUID(table, state);
	table->tabl = ivm_vmstate_allocAt(
		state, sizeof(*table->tabl) * dsize,
		ivm_slot_table_getGen(table)
	);

	// IVM_TRACE("slot table expanding!\n");

	STD_INIT(table->tabl,
			 sizeof(*table->tabl)
			 * dsize);
	table->size = dsize;

#if IVM_USE_HASH_TABLE_AS_SLOT_TABLE
	if (!table->mark.sub.is_hash &&
		dsize >= IVM_DEFAULT_SLOT_TABLE_TO_HASH_THRESHOLD) {
		SET_BIT_TRUE(table->mark.sub.is_hash);
		// dsize <<= 1;
		ret = IVM_TRUE;
	}
#endif

	if (table->mark.sub.is_hash) {
		for (i = otable, end = i + osize;
			 i != end; i++) {
			if (i->k != IVM_NULL) {
				// IVM_TRACE("find slot: %s\n" ivm_string_trimHead(otable[i].k));
				ivm_slot_table_setSlot(table, state, i->k, i->v);
			}
		}
	} else {
		STD_MEMCPY(table->tabl, otable, sizeof(*table->tabl) * osize);
	}

	return ret;
}

void
ivm_slot_table_setSlot_r(ivm_slot_table_t *table,
						 ivm_vmstate_t *state,
						 const ivm_char_t *rkey,
						 ivm_object_t *obj)
{
	const ivm_string_t *key
	= (const ivm_string_t *)
	  ivm_string_pool_registerRaw(IVM_VMSTATE_GET(state, CONST_POOL), rkey);

	ivm_slot_table_setSlot(table, state, key, obj);
	
	return;
}
