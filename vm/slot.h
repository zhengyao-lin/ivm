#ifndef _IVM_VM_SLOT_H_
#define _IVM_VM_SLOT_H_

#include "pub/com.h"
#include "pub/type.h"

#include "std/hash.h"
#include "std/string.h"
#include "std/uid.h"

#include "instr.h"

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
	ivm_uid_t uid;
	ivm_slot_t *tabl;
} ivm_slot_table_t;

ivm_slot_table_t *
ivm_slot_table_new(struct ivm_vmstate_t_tag *state);

ivm_slot_table_t *
ivm_slot_table_copy(ivm_slot_table_t *table,
					struct ivm_vmstate_t_tag *state,
					struct ivm_heap_t_tag *heap);

void
ivm_slot_table_addSlot_cc(ivm_slot_table_t *table,
						  struct ivm_vmstate_t_tag *state,
						  const ivm_string_t *key,
						  struct ivm_object_t_tag *obj,
						  ivm_instr_cache_t *cache);

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

#define ivm_slot_table_checkCacheValid(table, cache) \
	((table) && ivm_instr_cache_id(cache) == (table)->uid)

#define FIND_SLOT(e1, e2) \
	{                                                                                   \
		ivm_hash_val_t hash;                                                            \
		register ivm_slot_t *i, *tmp, *end;                                             \
                                                                                        \
		if (table) {                                                                    \
			if (table->is_hash) {                                                       \
				hash = ivm_hash_fromString(ivm_string_trimHead(key)) % table->size;     \
                                                                                        \
				tmp = table->tabl + hash;                                               \
				end = table->tabl + table->size;                                        \
                                                                                        \
				for (i = tmp; i != end; i++) {                                          \
					if (IS_EMPTY_SLOT(i)) {                                             \
						return IVM_NULL;                                                \
					} else if (ivm_string_compare(i->k, key)) {                         \
						e1;                                                             \
						return i;                                                       \
					}                                                                   \
				}                                                                       \
                                                                                        \
				for (i = table->tabl; i != tmp; i++) {                                  \
					if (IS_EMPTY_SLOT(i)) {                                             \
						return IVM_NULL;                                                \
					} else if (ivm_string_compare(i->k, key)) {                         \
						e1;                                                             \
						return i;                                                       \
					}                                                                   \
				}                                                                       \
			} else {                                                                    \
				for (tmp = table->tabl,                                                 \
					 end = table->tabl + table->size;                                   \
					 tmp != end; tmp++) {                                               \
					if (!tmp->k) {                                                      \
						return IVM_NULL;                                                \
					} else if (ivm_string_compare(tmp->k, key)) {                       \
						e2;                                                             \
						return tmp;                                                     \
					}                                                                   \
				}                                                                       \
			}                                                                           \
		}                                                                               \
                                                                                        \
		return IVM_NULL;                                                                \
	}

/* cache version */
IVM_INLINE
ivm_slot_t *
ivm_slot_table_findSlot_cc(ivm_slot_table_t *table,
						   struct ivm_vmstate_t_tag *state,
						   const ivm_string_t *key,
						   ivm_instr_cache_t *cache)
FIND_SLOT(
	*cache = ivm_instr_cache_build(table->uid, (ivm_ptr_t)i),
	*cache = ivm_instr_cache_build(table->uid, (ivm_ptr_t)tmp)
)

IVM_INLINE
ivm_slot_t *
ivm_slot_table_findSlot(ivm_slot_table_t *table,
						struct ivm_vmstate_t_tag *state,
						const ivm_string_t *key)
FIND_SLOT(0, 0)


#undef FIND_SLOT

#undef IS_EMPTY_SLOT

IVM_COM_END

#endif
