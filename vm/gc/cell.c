#include "pub/mem.h"
#include "cell.h"
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

void
ivm_dump_cell(ivm_vmstate_t *state, ivm_cell_t *cell)
{
	if (cell) {
		ivm_dump_object(state, cell->obj);
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

ivm_cell_t *
ivm_cell_move_to_set(ivm_cell_t *cell, ivm_cell_set_t *from, ivm_cell_set_t *to)
{
	if (cell && from) {
		if (from->head == cell)
			from->head = cell->next;

		if (from->tail == cell)
			from->tail = cell->prev;
	}
	ivm_cell_set_add_cell(to, cell);

	return cell;
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
ivm_dump_cell_set(ivm_vmstate_t *state, ivm_cell_set_t *set)
{
	ivm_cell_t *i, *tmp;

	if (set) {
		for (i = set->head; i;) {
			tmp = i;
			i = i->next;
			ivm_dump_cell(state, tmp);
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

ivm_cell_t *
ivm_cell_set_add_object(ivm_cell_set_t *set, ivm_object_t *obj)
{
	ivm_cell_t *n_cell;

	if (set) {
		n_cell = ivm_new_cell(obj);
		if (!set->tail) {
			set->head
			= set->tail
			= n_cell;
		} else {
			set->tail->next = n_cell;
			n_cell->prev = set->tail;

			set->tail = n_cell;
		}
		return n_cell;
	}

	return IVM_NULL;
}

ivm_cell_t *
ivm_cell_set_remove_tail(ivm_cell_set_t *set)
{
	ivm_cell_t *ret = IVM_NULL;

	if (set && set->tail) {
		ret = set->tail;
		if (set->tail->prev) {
			set->tail->prev->next = IVM_NULL;
		} else {
			set->head = IVM_NULL;
		}
		set->tail = set->tail->prev;
	}

	return ret;
}
