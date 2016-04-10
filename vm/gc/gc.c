#include "pub/mem.h"
#include "gc.h"
#include "type.h"
#include "obj.h"
#include "err.h"
#include "vm.h"

ivm_cell_t *
ivm_new_cell(ivm_object_t *obj)
{
	ivm_cell_t *ret = MEM_ALLOC_INIT(sizeof(*ret));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("cell"));

	ret->obj = obj;
	ret->prev = IVM_NULL;
	ret->next = IVM_NULL;

	return ret;
}

void
ivm_free_cell(ivm_cell_t *cell)
{
	if (cell) {
		MEM_FREE(cell);
	}

	return;
}

void
ivm_dispose_cell(ivm_vmstate_t *state, ivm_cell_t *cell)
{
	if (cell) {
		ivm_free_object(state, cell->obj);
		MEM_FREE(cell);
	}

	return;
}

#define IS_SUC_CELL(prev, next) \
	((!(prev) || (prev)->next == (next)) \
	 && (!(next) || (next)->prev == (prev)))

void
ivm_cell_move_between(ivm_cell_t *cell,
					  ivm_cell_t *prev,
					  ivm_cell_t *next)
{
	IVM_ASSERT(IS_SUC_CELL(prev, next),
			   IVM_ERROR_MSG_INSERT_CELL_TO_NON_SUC_CELL);
	if (cell) {
		if (cell->prev)
			cell->prev->next = cell->next;

		if (cell->next)
			cell->next->prev = cell->prev;

		cell->prev = prev;
		cell->next = next;

		if (prev)
			prev->next = cell;

		if (next)
			next->prev = cell;
	}

	return;
}

ivm_cell_set_t *
ivm_new_cell_set()
{
	ivm_cell_set_t *ret = MEM_ALLOC_INIT(sizeof(*ret));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("cell set"));

	ret->head = IVM_NULL;
	ret->tail = IVM_NULL;

	return ret;
}

void
ivm_free_cell_set(ivm_cell_set_t *set)
{
	ivm_cell_t *i, *tmp;

	if (set) {
		for (i = set->head; i;) {
			tmp = i;
			i = i->next;
			ivm_free_cell(tmp);
		}
		MEM_FREE(set);
	}

	return;
}

void
ivm_dispose_cell_set(ivm_vmstate_t *state, ivm_cell_set_t *set)
{
	ivm_cell_t *i, *tmp;

	if (set) {
		for (i = set->head; i;) {
			tmp = i;
			i = i->next;
			ivm_dispose_cell(state, tmp);
		}
		MEM_FREE(set);
	}

	return;
}

void
ivm_cell_set_add_cell(ivm_cell_set_t *set, ivm_cell_t *cell)
{
	if (set) {
		ivm_cell_move_between(cell, set->tail, NULL);
		set->tail = cell;
		if (!set->head) {
			set->head = cell;
		}
	}

	return;
}
