#include <stdio.h>

#include "pub/mem.h"
#include "pub/com.h"
#include "pub/const.h"
#include "pub/err.h"
#include "pub/vm.h"

#include "std/hash.h"
#include "std/string.h"
#include "std/heap.h"
#include "std/bit.h"

#include "slot.h"
#include "obj.h"

#define SET_BIT_FALSE IVM_BIT_SET_FALSE
#define SET_BIT_TRUE IVM_BIT_SET_TRUE

ivm_slot_table_t *
ivm_slot_table_new(ivm_vmstate_t *state)
{
	ivm_slot_table_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	SET_BIT_FALSE(ret->is_hash);
	SET_BIT_FALSE(ret->is_shared);

#if IVM_USE_HASH_TABLE_AS_SLOT_TABLE
	ret->is_hash |= (IVM_DEFAULT_SLOT_TABLE_SIZE >= IVM_DEFAULT_SLOT_TABLE_TO_HASH_THRESHOLD);
#endif

	ret->mark = IVM_MARK_INIT;
	ret->size = IVM_DEFAULT_SLOT_TABLE_SIZE;
	ret->uid = ivm_vmstate_genUID(state);
	ret->tabl = ivm_vmstate_alloc(state,
								  sizeof(*ret->tabl)
								  * IVM_DEFAULT_SLOT_TABLE_SIZE);
	MEM_INIT(ret->tabl,
			 sizeof(*ret->tabl)
			 * IVM_DEFAULT_SLOT_TABLE_SIZE);

	return ret;
}

ivm_slot_table_t *
ivm_slot_table_new_c(ivm_vmstate_t *state,
					 ivm_size_t prealloc)
{
	ivm_slot_table_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	SET_BIT_FALSE(ret->is_hash);
	SET_BIT_FALSE(ret->is_shared);

	if (prealloc < IVM_DEFAULT_SLOT_TABLE_SIZE) {
		prealloc = IVM_DEFAULT_SLOT_TABLE_SIZE;
	}

#if IVM_USE_HASH_TABLE_AS_SLOT_TABLE
	if (prealloc >= IVM_DEFAULT_SLOT_TABLE_TO_HASH_THRESHOLD) {
		prealloc <<= 1; // lower load ratio
		SET_BIT_TRUE(ret->is_hash);
	}
#endif

	ret->mark = IVM_MARK_INIT;
	ret->size = prealloc;
	ret->uid = ivm_vmstate_genUID(state);
	ret->tabl = ivm_vmstate_alloc(state,
								  sizeof(*ret->tabl)
								  * prealloc);
	MEM_INIT(ret->tabl,
			 sizeof(*ret->tabl)
			 * prealloc);

	return ret;
}

ivm_slot_table_t *
ivm_slot_table_copy(ivm_slot_table_t *table,
					ivm_vmstate_t *state,
					ivm_heap_t *heap)
{
	ivm_slot_table_t *ret = IVM_NULL;
	// ivm_slot_t *tmp, *end;

	if (table) {
		ret = ivm_heap_alloc(heap, sizeof(*ret));
		ret->is_hash = table->is_hash;
		SET_BIT_FALSE(ret->is_shared);
		ret->mark = IVM_MARK_INIT;
		ret->size = table->size;
		ret->uid = ivm_vmstate_genUID(state);
		ret->tabl = ivm_heap_alloc(heap,
								   sizeof(*ret->tabl)
								   * ret->size);
		MEM_COPY(ret->tabl,
				 table->tabl,
				 sizeof(*ret->tabl)
				 * ret->size);
#if 0
		for (tmp = ret->tabl,
			 end = ret->tabl + ret->size;
			 tmp != end;
			 tmp++) {
			tmp->k = ivm_string_copyIfNotConst_heap(tmp->k, heap);
		}
#endif
	}

	return ret;
}

ivm_slot_table_t *
_ivm_slot_table_copy_state(ivm_slot_table_t *table,
						   ivm_vmstate_t *state)
{
	ivm_slot_table_t *ret = IVM_NULL;
	// ivm_slot_t *tmp, *end;

	if (table) {
		ret = ivm_vmstate_alloc(state, sizeof(*ret));
		ret->is_hash = table->is_hash;
		SET_BIT_FALSE(ret->is_shared);
		ret->mark = IVM_MARK_INIT;
		ret->size = table->size;
		ret->uid = ivm_vmstate_genUID(state);
		ret->tabl = ivm_vmstate_alloc(state,
									  sizeof(*ret->tabl)
									  * ret->size);
		MEM_COPY(ret->tabl,
				 table->tabl,
				 sizeof(*ret->tabl)
				 * ret->size);

#if 0
		for (tmp = ret->tabl,
			 end = ret->tabl + ret->size;
			 tmp != end;
			 tmp++) {
			tmp->k = ivm_string_copyIfNotConst_state(tmp->k, state);
		}
#endif
	}

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

	table->uid = ivm_vmstate_genUID(state);
	table->tabl = ivm_vmstate_alloc(state,
									sizeof(*table->tabl)
									* dsize);
	MEM_INIT(table->tabl,
			 sizeof(*table->tabl)
			 * dsize);
	table->size = dsize;

#if IVM_USE_HASH_TABLE_AS_SLOT_TABLE
	if (!table->is_hash &&
		dsize >= IVM_DEFAULT_SLOT_TABLE_TO_HASH_THRESHOLD) {
		SET_BIT_TRUE(table->is_hash);
		// dsize <<= 1;
		ret = IVM_TRUE;
	}
#endif

	if (table->is_hash) {
		for (i = otable, end = i + osize;
			 i != end; i++) {
			if (i->k != IVM_NULL) {
				// IVM_TRACE("find slot: %s\n" ivm_string_trimHead(otable[i].k));
				ivm_slot_table_addSlot(table, state, i->k, i->v);
			}
		}
	} else {
		MEM_COPY(table->tabl, otable, sizeof(*table->tabl) * osize);
	}

	return ret;
}

void
ivm_slot_table_addSlot_r(ivm_slot_table_t *table,
						 ivm_vmstate_t *state,
						 const ivm_char_t *rkey,
						 ivm_object_t *obj)
{
	const ivm_string_t *key
	= (const ivm_string_t *)
	  ivm_string_pool_registerRaw(IVM_VMSTATE_GET(state, CONST_POOL), rkey);

	ivm_slot_table_addSlot(table, state, key, obj);
	
	return;
}
