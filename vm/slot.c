#include <stdio.h>
#include "pub/mem.h"
#include "pub/com.h"
#include "pub/const.h"
#include "slot.h"
#include "obj.h"
#include "str.h"
#include "vm.h"
#include "gc/heap.h"
#include "std/hash.h"
#include "err.h"

#define SET_BIT_FALSE(bit) ((bit) &= 0)
#define SET_BIT_TRUE(bit) ((bit) |= 1)

ivm_slot_table_t *
ivm_slot_table_new(ivm_vmstate_t *state)
{
	ivm_slot_table_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	SET_BIT_FALSE(ret->is_hash);

#if IVM_USE_HASH_TABLE_AS_SLOT_TABLE
	ret->is_hash |= (IVM_DEFAULT_SLOT_TABLE_SIZE >= IVM_DEFAULT_SLOT_TABLE_TO_HASH_THRESHOLD);
#endif

	ret->size = IVM_DEFAULT_SLOT_TABLE_SIZE;
	ret->tabl = ivm_vmstate_alloc(state,
								  sizeof(*ret->tabl)
								  * IVM_DEFAULT_SLOT_TABLE_SIZE);
	MEM_INIT(ret->tabl,
			 sizeof(*ret->tabl)
			 * IVM_DEFAULT_SLOT_TABLE_SIZE);

	return ret;
}

ivm_slot_table_t *
ivm_slot_table_copy(ivm_slot_table_t *table, struct ivm_heap_t_tag *heap)
{
	ivm_slot_table_t *ret = IVM_NULL;

	if (table) {
		ret = ivm_heap_alloc(heap, sizeof(*ret));
		ret->is_hash = table->is_hash;
		ret->size = table->size;
		ret->tabl = ivm_heap_alloc(heap,
								   sizeof(*ret->tabl)
								   * ret->size);
		MEM_COPY(ret->tabl,
				 table->tabl,
				 sizeof(*ret->tabl)
				 * ret->size);
	}

	return ret;
}

#define IS_EMPTY_SLOT(slot) (!(slot)->k)

ivm_slot_t *
ivm_slot_table_findSlot(ivm_slot_table_t *table,
						ivm_vmstate_t *state,
						const ivm_char_t *key)
{
	ivm_hash_val_t hash;
	ivm_size_t size;
	ivm_uint_t h1, h2;
	ivm_uint_t i, j;

	ivm_slot_t *tmp;
	
	if (table) {
		if (table->is_hash) {
			hash = ivm_hash_fromString(key);
			size = table->size;
			h1 = hash % size;
			h2 = 1 + hash % (size - 1);

			for (i = h1, j = 0;
				 j < size;
				 i += h2, j++) {
				tmp = &table->tabl[i % size];
				if (IS_EMPTY_SLOT(tmp)) {
					return IVM_NULL;
				} else if (strcmp(key, tmp->k) == 0) {
					return tmp;
				}
			}
		} else {
			for (i = 0; i < table->size; i++) {
				if (table->tabl[i].k == IVM_NULL) {
					return IVM_NULL;
				} else if (strcmp(key, table->tabl[i].k) == 0) {
					return &table->tabl[i];
				}
			}
		}
	}

	return IVM_NULL;
}

IVM_PRIVATE
void
ivm_slot_table_expand(ivm_slot_table_t *table,
					  ivm_vmstate_t *state) /* includes rehashing */
{
	ivm_size_t osize = table->size,
			   dsize = osize << 1; /* dest size */
	ivm_slot_t *otable = table->tabl;
	ivm_size_t i;

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

void
ivm_slot_table_addSlot(ivm_slot_table_t *table,
					   ivm_vmstate_t *state,
					   const ivm_char_t *key,
					   ivm_object_t *obj)
{
	ivm_hash_val_t hash;
	ivm_size_t size;
	ivm_uint_t h1, h2;
	ivm_uint_t i, j;

	ivm_slot_t *tmp;

	if (table->is_hash) {
		hash = ivm_hash_fromString(key);
		while (1) {
			size = table->size;
			h1 = hash % size;
			h2 = 1 + hash % (size - 1);

			for (i = h1, j = 0;
				 j < size;
				 i += h2, j++) {
				tmp = &table->tabl[i % size];
				if (IS_EMPTY_SLOT(tmp)) {
					tmp->k = key;
					tmp->v = obj;
					return;
				} else if (strcmp(key, tmp->k) == 0) {
					tmp->v = obj;
					return;
				}
			}

			/* allocate new space */
			ivm_slot_table_expand(table, state);
		}
	} else {
		for (i = 0; i < table->size; i++) {
			if (table->tabl[i].k == IVM_NULL) {
				table->tabl[i].k = key;
				table->tabl[i].v = obj;
				return;
			} else if (strcmp(key, table->tabl[i].k) == 0) {
				table->tabl[i].v = obj;
				return;
			}
		}

		ivm_slot_table_expand(table, state);
		table->tabl[i].k = key;
		table->tabl[i].v = obj;
	}

	return;
}
