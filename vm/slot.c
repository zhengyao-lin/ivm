#include <stdio.h>
#include "pub/mem.h"
#include "err.h"
#include "slot.h"
#include "obj.h"
#include "str.h"
#include "vm.h"

static ivm_slot_t *
ivm_slot_new(ivm_vmstate_t *state,
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
ivm_slot_free(ivm_slot_t *slot, ivm_vmstate_t *state)
{
	if (slot) {
		MEM_FREE(slot->k);
		MEM_FREE(slot);
	}

	return;
}

ivm_slot_table_t *
ivm_slot_table_new(ivm_vmstate_t *state)
{
	ivm_slot_table_t *ret = MEM_ALLOC_INIT(sizeof(*ret));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("slot table"));

	ret->head = IVM_NULL;
	ret->tail = IVM_NULL;

	return ret;
}

void
ivm_slot_table_free(ivm_slot_table_t *table, ivm_vmstate_t *state)
{
	ivm_slot_t *i, *tmp;

	if (table) {
		for (i = ivm_slot_table_getHead(table, state); i;) {
			tmp = i;
			i = i->next;
			ivm_slot_free(tmp, state);
		}

		MEM_FREE(table);
	}

	return;
}

ivm_slot_t *
ivm_slot_table_findSlot(ivm_slot_table_t *table,
						ivm_vmstate_t *state,
						const ivm_char_t *key)
{
	ivm_slot_t *i;

	if (table) {
		for (i = ivm_slot_table_getHead(table, state); i; i = i->next) {
			if (!IVM_STRCMP(key, ivm_slot_getKey(i, state))) {
				return i;
			}
		}
	}

	return IVM_NULL;
}

ivm_slot_t *
ivm_slot_table_addSlot(ivm_slot_table_t *table,
					   ivm_vmstate_t *state,
					   const ivm_char_t *key,
					   ivm_object_t *obj,
					   ivm_object_t *parent)
{
	if (table) {
		if (!table->tail) {
			table->head
			= table->tail
			= ivm_slot_new(state, key, obj, parent);
		} else {
			table->tail
			= table->tail->next
			= ivm_slot_new(state, key, obj, parent);
		}

		return table->tail;
	}

	return IVM_NULL;
}
