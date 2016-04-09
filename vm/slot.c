#include <stdio.h>
#include "pub/mem.h"
#include "err.h"
#include "slot.h"
#include "obj.h"
#include "str.h"

static ivm_slot_t *
ivm_new_slot(const ivm_char_t *key,
			 ivm_object_t *value)
{
	ivm_slot_t *ret = MEM_ALLOC_INIT(sizeof(*ret));

	IVM_ASSERT(ret, "Failed to allocate new room for new slot");

	ret->k = IVM_STRDUP(key);
	ret->v = value;
	ret->next = IVM_NULL;

	return ret;
}

static void
ivm_free_slot(ivm_slot_t *slot)
{
	if (slot) {
		MEM_FREE(slot->k);
		MEM_FREE(slot);
	}

	return;
}

ivm_slot_table_t *
ivm_new_slot_table()
{
	ivm_slot_table_t *ret = MEM_ALLOC_INIT(sizeof(*ret));

	IVM_ASSERT(ret, "Failed to allocate new room for new slot table");

	ret->head = IVM_NULL;
	ret->tail = IVM_NULL;

	return ret;
}

void
ivm_free_slot_table(ivm_slot_table_t *table)
{
	ivm_slot_t *i, *tmp;

	if (table) {
		for (i = ivm_slot_table_head(table); i;) {
			tmp = i;
			i = i->next;
			ivm_free_slot(tmp);
		}

		MEM_FREE(table);
	}

	return;
}

ivm_slot_t *
ivm_slot_find_slot(ivm_slot_table_t *table,
			  const ivm_char_t *key)
{
	ivm_slot_t *i;

	if (table) {
		for (i = ivm_slot_table_head(table); i; i = i->next) {
			if (!IVM_STRCMP(key, ivm_slot_get_key(i))) {
				return i;
			}
		}
	}

	return IVM_NULL;
}

ivm_slot_t *
ivm_slot_add_slot(ivm_slot_table_t *table,
			 const ivm_char_t *key,
			 ivm_object_t *obj)
{
	if (table) {
		if (!table->tail) {
			table->head
			= table->tail
			= ivm_new_slot(key, obj);
		} else {
			table->tail
			= table->tail->next
			= ivm_new_slot(key, obj);
		}

		return table->tail;
	}

	return IVM_NULL;
}
