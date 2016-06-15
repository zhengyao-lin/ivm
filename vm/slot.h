#ifndef _IVM_VM_SLOT_H_
#define _IVM_VM_SLOT_H_

#include "pub/com.h"
#include "pub/type.h"

#include "std/hash.h"
#include "std/string.h"

IVM_COM_HEADER

struct ivm_object_t_tag;
struct ivm_vmstate_t_tag;
struct ivm_slot_t_tag;
struct ivm_heap_t_tag;

typedef void (*ivm_slot_table_foreach_proc_t)(struct ivm_slot_t_tag *, void *);

typedef struct ivm_slot_t_tag {
	const ivm_string_t *k;
	struct ivm_object_t_tag *v;
} ivm_slot_t;

#define ivm_slot_setValue(slot, state, obj) (slot ? slot->v = obj : IVM_NULL)
#define ivm_slot_getValue(slot, state) (slot ? slot->v : IVM_NULL)

typedef struct ivm_slot_table_t_tag {
	ivm_bool_t is_hash: 1;
	ivm_size_t size;
	ivm_slot_t *tabl;
} ivm_slot_table_t;

ivm_slot_table_t *
ivm_slot_table_new(struct ivm_vmstate_t_tag *state);

ivm_slot_table_t *
ivm_slot_table_copy(ivm_slot_table_t *table, struct ivm_heap_t_tag *heap);

void
ivm_slot_table_addSlot(ivm_slot_table_t *table,
					   struct ivm_vmstate_t_tag *state,
					   const ivm_string_t *key,
					   struct ivm_object_t_tag *obj);

typedef ivm_slot_t *ivm_slot_table_iterator_t;

#define IVM_SLOT_TABLE_ITER_SET_KEY(iter, key) ((iter)->k = (key))
#define IVM_SLOT_TABLE_ITER_SET_VAL(iter, val) ((iter)->v = (val))
#define IVM_SLOT_TABLE_ITER_SET(iter, key, val) ((iter)->k = (key), (iter)->v = (val))
#define IVM_SLOT_TABLE_ITER_GET_KEY(iter) ((iter)->k)
#define IVM_SLOT_TABLE_ITER_GET_VAL(iter) ((iter)->v)
#define IVM_SLOT_TABLE_EACHPTR(table, iter) \
	for ((iter) = (table)->tabl; \
		 (iter) < ((table)->tabl + (table)->size); \
		 (iter)++)

#define IS_EMPTY_SLOT(slot) (!(slot)->k)

IVM_INLINE
ivm_slot_t *
ivm_slot_table_findSlot(ivm_slot_table_t *table,
						struct ivm_vmstate_t_tag *state,
						const ivm_string_t *key)
{
	ivm_hash_val_t hash;
	// register ivm_uint_t h1, h2;
	register ivm_slot_t *i, *tmp, *end;
	
	if (table) {
		if (table->is_hash) {
			hash = ivm_hash_fromString(ivm_string_trimHead(key)) % table->size;
			// h2 = 1; // + hash % (size - 1);

			tmp = table->tabl + hash;
			end = table->tabl + table->size;

			for (i = tmp; i != end; i++) {
				if (IS_EMPTY_SLOT(i)) {
					return IVM_NULL;
				} else if (ivm_string_compare(i->k, key)) {
					return i;
				}
			}

			for (i = table->tabl; i != tmp; i++) {
				if (IS_EMPTY_SLOT(i)) {
					return IVM_NULL;
				} else if (ivm_string_compare(i->k, key)) {
					return i;
				}
			}
		} else {
			for (tmp = table->tabl,
				 end = table->tabl + table->size;
				 tmp != end; tmp++) {
				if (!tmp->k) {
					return IVM_NULL;
				} else if (ivm_string_compare(tmp->k, key)) {
					return tmp;
				}
			}
		}
	}

	return IVM_NULL;
}

#undef IS_EMPTY_SLOT

IVM_COM_END

#endif
