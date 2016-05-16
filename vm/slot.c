#include <stdio.h>
#include "pub/mem.h"
#include "pub/com.h"
#include "slot.h"
#include "obj.h"
#include "str.h"
#include "vm.h"
#include "gc/heap.h"
#include "err.h"

#define HEAP_STRDUP(state, str) (ivm_vmstate_alloc((state), sizeof(*(str)) * (IVM_STRLEN(str) + 1)))

IVM_PRIVATE
ivm_slot_t *
ivm_slot_new(ivm_vmstate_t *state,
			 const ivm_char_t *key,
			 ivm_object_t *value)
{
	ivm_slot_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ret->k = IVM_STRDUP_STATE(key, state);
	ret->v = value;
	ret->next = IVM_NULL;

	return ret;
}

IVM_PRIVATE
ivm_slot_t *
ivm_slot_copy(ivm_slot_t *slot, ivm_heap_t *heap)
{
	ivm_slot_t *ret = IVM_NULL;

	if (slot) {
		ret = ivm_heap_alloc(heap, sizeof(*ret));
		ret->k = IVM_STRDUP_HEAP(slot->k, heap);
		ret->v = slot->v;
		ret->next = IVM_NULL;
	}

	return ret;
}

#define FOREACH(i, table) for ((i) = IVM_SLOT_TABLE_HEAD(table); (i); (i) = (i)->next)

ivm_slot_table_t *
ivm_slot_table_new(ivm_vmstate_t *state)
{
	ivm_slot_table_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ret->head = IVM_NULL;
	ret->tail = IVM_NULL;

	return ret;
}

ivm_slot_table_t *
ivm_slot_table_copy(ivm_slot_table_t *table, ivm_heap_t *heap)
{
	ivm_slot_table_t *ret = IVM_NULL;
	ivm_slot_t *slot = IVM_NULL,
			   *cur = IVM_NULL;

	if (table) {
		ret = ivm_heap_alloc(heap, sizeof(*ret));
		ret->head = ivm_slot_copy(table->head, heap);

		if (ret->head) {
			for (slot = table->head->next, cur = ret->head;
				 slot; slot = slot->next, cur = cur->next) {
				cur->next = ivm_slot_copy(slot, heap);
			}
		}

		ret->tail = cur;

		/*
		for (slot = ret->head; slot; slot = slot->next, i++) printf("\t%p : %s\n", slot, slot->k);
		printf("after copy: %p, %p, %d\n", ret->head, ret->tail, i);	
		*/
	}

	return ret;
}

ivm_slot_t *
ivm_slot_table_findSlot(ivm_slot_table_t *table,
						ivm_vmstate_t *state,
						const ivm_char_t *key)
{
	ivm_slot_t *i;

	if (table) {
		FOREACH (i, table) {
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
					   ivm_object_t *obj)
{
	if (table) {
		if (!table->tail) {
			table->head
			= table->tail
			= ivm_slot_new(state, key, obj);
		} else {
			table->tail
			= table->tail->next
			= ivm_slot_new(state, key, obj);
		}

		return table->tail;
	}

	return IVM_NULL;
}

void
ivm_slot_table_foreach(ivm_slot_table_t *table,
					   ivm_slot_table_foreach_proc_t proc,
					   void *arg)
{
	ivm_slot_t *i;

	FOREACH (i, table) {
		proc(i, arg);
	}

	return;
}
