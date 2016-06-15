#include <stdio.h>
#include "pub/mem.h"
#include "pub/com.h"
#include "pub/const.h"
#include "pub/err.h"
#include "pub/vm.h"

#include "std/hash.h"
#include "std/string.h"
#include "std/heap.h"
#include "slot.h"
#include "obj.h"
#include "bit.h"

#define SET_BIT_FALSE IVM_BIT_SET_FALSE
#define SET_BIT_TRUE IVM_BIT_SET_TRUE

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
	ivm_slot_t *tmp, *end;

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

		for (tmp = ret->tabl,
			 end = ret->tabl + ret->size;
			 tmp != end;
			 tmp++) {
			tmp->k = ivm_string_copyIfNotConst_heap(tmp->k, heap);
		}
	}

	return ret;
}

#define IS_EMPTY_SLOT(slot) (!(slot)->k)

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

void
ivm_slot_table_addSlot(ivm_slot_table_t *table,
					   ivm_vmstate_t *state,
					   const ivm_string_t *key,
					   ivm_object_t *obj)
{
	ivm_hash_val_t hash;
	ivm_size_t osize;

	register ivm_slot_t *i, *tmp, *end;

	if (table->is_hash) {
		hash = ivm_hash_fromString(ivm_string_trimHead(key));
		while (1) {
			tmp = table->tabl + hash % table->size;
			end = table->tabl + table->size;

			for (i = tmp;
				 i != end;
				 i++) {
				if (IS_EMPTY_SLOT(i)) {
					i->k = ivm_string_copyIfNotConst_pool(key, state);
					i->v = obj;
					return;
				} else if (ivm_string_compare(i->k, key)) {
					i->v = obj;
					return;
				}
			}

			for (i = table->tabl;
				 i != tmp;
				 i++) {
				if (IS_EMPTY_SLOT(i)) {
					i->k = ivm_string_copyIfNotConst_pool(key, state);
					i->v = obj;
					return;
				} else if (ivm_string_compare(i->k, key)) {
					i->v = obj;
					return;
				}
			}

			/* allocate new space */
			ivm_slot_table_expand(table, state);
		}
	} else {
		for (i = table->tabl,
			 end = table->tabl + table->size;
			 i != end; i++) {
			if (i->k == IVM_NULL) {
				i->k = ivm_string_copyIfNotConst_pool(key, state);
				i->v = obj;
				return;
			} else if (ivm_string_compare(i->k, key)) {
				i->v = obj;
				return;
			}
		}

		osize = table->size;
		ivm_slot_table_expand(table, state);
		tmp = table->tabl + osize;
		tmp->k = ivm_string_copyIfNotConst_pool(key, state);
		tmp->v = obj;
	}

	return;
}
