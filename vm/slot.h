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

ivm_slot_table_t * ivm_new_slot_table(struct ivm_vmstate_t_tag *state);
void ivm_free_slot_table(struct ivm_vmstate_t_tag *state, ivm_slot_table_t *table);

#define ivm_slot_table_head(state, table) ((table) ? (table)->head : IVM_NULL)
#define ivm_slot_table_tail(state, table) ((table) ? (table)->tail : IVM_NULL)

ivm_slot_t *
ivm_slot_table_find_slot(struct ivm_vmstate_t_tag *state,
						 ivm_slot_table_t *table,
						 const ivm_char_t *key);
ivm_slot_t *
ivm_slot_table_add_slot(struct ivm_vmstate_t_tag *state,
						ivm_slot_table_t *table,
						const ivm_char_t *key,
						struct ivm_object_t_tag *obj,
						struct ivm_object_t_tag *parent);

#define ivm_slot_set_value(state, slot, obj) (slot ? slot->v = obj : IVM_NULL)
#define ivm_slot_get_value(state, slot) (slot ? slot->v : IVM_NULL)
#define ivm_slot_get_key(state, slot) (slot->k)

#endif
