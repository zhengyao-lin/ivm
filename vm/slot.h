#ifndef _IVM_VM_SLOT_H_
#define _IVM_VM_SLOT_H_

#include "pub/com.h"
#include "type.h"

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

#if 0

ivm_slot_table_t *
ivm_slot_table_new(struct ivm_vmstate_t_tag *state);

ivm_slot_table_t *
ivm_slot_table_copy(ivm_slot_table_t *table, struct ivm_heap_t_tag *heap);

#define IVM_SLOT_TABLE_HEAD(table) ((table) ? (table)->head : IVM_NULL)
#define IVM_SLOT_TABLE_TAIL(table) ((table) ? (table)->tail : IVM_NULL)

ivm_slot_t *
ivm_slot_table_findSlot(ivm_slot_table_t *table,
						struct ivm_vmstate_t_tag *state,
						const ivm_char_t *key);
ivm_slot_t *
ivm_slot_table_addSlot(ivm_slot_table_t *table,
					   struct ivm_vmstate_t_tag *state,
					   const ivm_char_t *key,
					   struct ivm_object_t_tag *obj);

void
ivm_slot_table_foreach(ivm_slot_table_t *table,
					   ivm_slot_table_foreach_proc_t proc,
					   void *arg);

typedef ivm_slot_t *ivm_slot_table_iterator_t;

#define IVM_SLOT_TABLE_ITER_SET_KEY(iter, key) ((iter)->k = (key))
#define IVM_SLOT_TABLE_ITER_SET_VAL(iter, val) ((iter)->v = (val))
#define IVM_SLOT_TABLE_ITER_SET(iter, key, val) ((iter)->k = (key), (iter)->v = (val))
#define IVM_SLOT_TABLE_ITER_GET_KEY(iter) ((iter)->k)
#define IVM_SLOT_TABLE_ITER_GET_VAL(iter) ((iter)->v)
#define IVM_SLOT_TABLE_EACHPTR(table, iter) \
	for ((iter) = IVM_SLOT_TABLE_HEAD(table); \
		 (iter); (iter) = (iter)->next)

#endif

IVM_COM_END

#endif
