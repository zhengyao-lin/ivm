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

void
_ivm_slot_table_expand(ivm_slot_table_t *table,
					   ivm_vmstate_t *state) /* includes rehashing */
{
	ivm_size_t osize = table->size,
			   dsize = osize << 1; /* dest size */
	ivm_slot_t *otable = table->tabl;
	ivm_size_t i;

	table->uid = ivm_vmstate_genUID(state);
	table->tabl = ivm_vmstate_alloc(state,
									sizeof(*table->tabl)
									* dsize);
	MEM_INIT(table->tabl,
			 sizeof(*table->tabl)
			 * dsize);
	table->size = dsize;

#if IVM_USE_HASH_TABLE_AS_SLOT_TABLE
	if (dsize >= IVM_DEFAULT_SLOT_TABLE_TO_HASH_THRESHOLD) {
		SET_BIT_TRUE(table->is_hash);
		// dsize <<= 1;
	}
#endif

	if (table->is_hash) {
		for (i = 0; i < osize; i++) {
			if (otable[i].k != IVM_NULL)
				ivm_slot_table_addSlot(table, state, otable[i].k, otable[i].v);
		}
	} else {
		MEM_COPY(table->tabl, otable, sizeof(*table->tabl) * osize);
	}

	return;
}
