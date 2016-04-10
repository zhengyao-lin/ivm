#ifndef _IVM_VM_SLOT_H_
#define _IVM_VM_SLOT_H_

#include "type.h"

struct ivm_object_t_tag;
struct ivm_vmstate_t_tag;

typedef struct ivm_slot_t_tag {
	struct ivm_object_t_tag *p;

	ivm_char_t *k;
	struct ivm_object_t_tag *v;

	struct ivm_slot_t_tag *next;
} ivm_slot_t;

typedef struct ivm_slot_table_t_tag {
	ivm_slot_t *head;
	ivm_slot_t *tail;
} ivm_slot_table_t;

ivm_slot_table_t * ivm_slot_table_new(struct ivm_vmstate_t_tag *state);
void ivm_slot_table_free(struct ivm_vmstate_t_tag *state, ivm_slot_table_t *table);

#define ivm_slot_table_getHead(state, table) ((table) ? (table)->head : IVM_NULL)
#define ivm_slot_table_getTail(state, table) ((table) ? (table)->tail : IVM_NULL)

ivm_slot_t *
ivm_slot_table_findSlot(struct ivm_vmstate_t_tag *state,
						ivm_slot_table_t *table,
						const ivm_char_t *key);
ivm_slot_t *
ivm_slot_table_addSlot(struct ivm_vmstate_t_tag *state,
					   ivm_slot_table_t *table,
					   const ivm_char_t *key,
					   struct ivm_object_t_tag *obj,
					   struct ivm_object_t_tag *parent);

#define ivm_slot_setValue(state, slot, obj) (slot ? slot->v = obj : IVM_NULL)
#define ivm_slot_getValue(state, slot) (slot ? slot->v : IVM_NULL)
#define ivm_slot_getKey(state, slot) (slot->k)

#endif
