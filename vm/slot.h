#ifndef _IVM_VM_SLOT_H_
#define _IVM_VM_SLOT_H_

#include "type.h"

struct ivm_object_t_tag;
struct ivm_vmstate_t_tag;
struct ivm_slot_t_tag;

typedef void (*ivm_slot_table_foreach_proc_t)(struct ivm_slot_t_tag *, void *);

typedef struct ivm_slot_t_tag {
	struct ivm_object_t_tag *p;

	ivm_char_t *k;
	struct ivm_object_t_tag *v;

	struct ivm_slot_t_tag *next;
} ivm_slot_t;

#define ivm_slot_setValue(slot, state, obj) (slot ? slot->v = obj : IVM_NULL)
#define ivm_slot_getValue(slot, state) (slot ? slot->v : IVM_NULL)
#define ivm_slot_getKey(slot, state) (slot->k)

typedef struct ivm_slot_table_t_tag {
	ivm_slot_t *head;
	ivm_slot_t *tail;
} ivm_slot_table_t;

ivm_slot_table_t * ivm_slot_table_new(struct ivm_vmstate_t_tag *state);
void ivm_slot_table_free(ivm_slot_table_t *table, struct ivm_vmstate_t_tag *state);

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
					   struct ivm_object_t_tag *obj,
					   struct ivm_object_t_tag *parent);

void
ivm_slot_table_foreach(ivm_slot_table_t *table,
					   ivm_slot_table_foreach_proc_t proc,
					   void *arg);

#endif
