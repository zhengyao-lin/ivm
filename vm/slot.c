#include <stdio.h>
#include "pub/mem.h"
#include "err.h"
#include "slot.h"
#include "obj.h"
#include "str.h"
#include "vm.h"

static ivm_slot_t *
ivm_new_slot(ivm_vmstate_t *state,
			 const ivm_char_t *key,
			 ivm_object_t *value,
			 ivm_object_t *parent)
{
	ivm_slot_t *ret = MEM_ALLOC_INIT(sizeof(*ret));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("slot"));

	ret->p = parent;
	ret->k = IVM_STRDUP(key);
	ret->v = value;
	ret->next = IVM_NULL;

	return ret;
}

static void
ivm_free_slot(ivm_vmstate_t *state, ivm_slot_t *slot)
{
	if (slot) {
		MEM_FREE(slot->k);
		MEM_FREE(slot);
	}

	return;
}

ivm_slot_table_t *
ivm_new_slot_table(ivm_vmstate_t *state)
{
	ivm_slot_table_t *ret = MEM_ALLOC_INIT(sizeof(*ret));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("slot table"));

	ret->head = IVM_NULL;
	ret->tail = IVM_NULL;

	return ret;
}

void
ivm_free_slot_table(ivm_vmstate_t *state, ivm_slot_table_t *table)
{
	ivm_slot_t *i, *tmp;

	if (table) {
		for (i = ivm_slot_table_head(state, table); i;) {
			tmp = i;
			i = i->next;
			ivm_free_slot(state, tmp);
		}

		MEM_FREE(table);
	}

	return;
}

ivm_slot_t *
ivm_slot_table_find_slot(ivm_vmstate_t *state,
						 ivm_slot_table_t *table,
						 const ivm_char_t *key)
{
	ivm_slot_t *i;

	if (table) {
		for (i = ivm_slot_table_head(state, table); i; i = i->next) {
			if (!IVM_STRCMP(key, ivm_slot_get_key(state, i))) {
				return i;
			}
		}
	}

	return IVM_NULL;
}

ivm_slot_t *
ivm_slot_table_add_slot(ivm_vmstate_t *state,
						ivm_slot_table_t *table,
						const ivm_char_t *key,
						ivm_object_t *obj,
						ivm_object_t *parent)
{
	if (table) {
		if (!table->tail) {
			table->head
			= table->tail
			= ivm_new_slot(state, key, obj, parent);
		} else {
			table->tail
			= table->tail->next
			= ivm_new_slot(state, key, obj, parent);
		}

		return table->tail;
	}

	return IVM_NULL;
}
