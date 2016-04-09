#ifndef _IVM_VM_SLOT_H_
#define _IVM_VM_SLOT_H_

#include "type.h"

struct ivm_object_t_tag;

typedef struct ivm_slot_t_tag {
	ivm_char_t *k;
	struct ivm_object_t_tag *v;

	struct ivm_slot_t_tag *next;
} ivm_slot_t;

typedef struct ivm_slot_table_t_tag {
	ivm_slot_t *head;
	ivm_slot_t *tail;
} ivm_slot_table_t;

ivm_slot_table_t * ivm_new_slot_table();
void ivm_free_slot_table(ivm_slot_table_t *table);

#define ivm_slot_table_head(table) ((table) ? (table)->head : IVM_NULL)
#define ivm_slot_table_tail(table) ((table) ? (table)->tail : IVM_NULL)

ivm_slot_t *
ivm_slot_find_slot(ivm_slot_table_t *table, const ivm_char_t *key);
ivm_slot_t *
ivm_slot_add_slot(ivm_slot_table_t *table, const ivm_char_t *key, struct ivm_object_t_tag *obj);

#define ivm_slot_set_value(slot, obj) (slot ? slot->v = obj : IVM_NULL)
#define ivm_slot_get_value(slot) (slot ? slot->v : IVM_NULL)
#define ivm_slot_get_key(slot) (slot->k)

#endif
