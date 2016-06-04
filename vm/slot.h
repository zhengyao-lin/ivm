#ifndef _IVM_VM_SLOT_H_
#define _IVM_VM_SLOT_H_

#include "pub/com.h"
#include "type.h"
#include "std/hash.h"

IVM_COM_HEADER

struct ivm_object_t_tag;
struct ivm_vmstate_t_tag;
struct ivm_slot_t_tag;
struct ivm_heap_t_tag;

typedef void (*ivm_slot_table_foreach_proc_t)(struct ivm_slot_t_tag *, void *);

typedef struct ivm_slot_t_tag {
	const ivm_char_t *k;
	struct ivm_object_t_tag *v;
} ivm_slot_t;

#define ivm_slot_setValue(slot, state, obj) (slot ? slot->v = obj : IVM_NULL)
#define ivm_slot_getValue(slot, state) (slot ? slot->v : IVM_NULL)
#define ivm_slot_getKey(slot, state) (slot->k)

typedef struct ivm_slot_table_t_tag {
	ivm_bool_t is_hash: 1;
	ivm_size_t size;
	ivm_slot_t *tabl;
} ivm_slot_table_t;

ivm_slot_table_t *
ivm_slot_table_new(struct ivm_vmstate_t_tag *state);

ivm_slot_table_t *
ivm_slot_table_copy(ivm_slot_table_t *table, struct ivm_heap_t_tag *heap);

inline
ivm_slot_t * /* the return value is not stable and need to be used instantaneously */
ivm_slot_table_findSlot(ivm_slot_table_t *table,
						struct ivm_vmstate_t_tag *state,
						const ivm_char_t *key);
void
ivm_slot_table_addSlot(ivm_slot_table_t *table,
					   struct ivm_vmstate_t_tag *state,
					   const ivm_char_t *key,
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
				} else if (!strcmp(key, tmp->k)) {
					return tmp;
				}
			}
		} else {
			for (i = 0; i < table->size; i++) {
				if (!table->tabl[i].k) {
					return IVM_NULL;
				} else if (!strcmp(key, table->tabl[i].k)) {
					return &table->tabl[i];
				}
			}
		}
	}

	return IVM_NULL;
}

#undef IS_EMPTY_SLOT

IVM_COM_END

#endif
