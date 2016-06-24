#include "pub/mem.h"
#include "pub/err.h"
#include "pub/vm.h"

#include "cell.h"

ivm_cell_t *
ivm_cell_new(ivm_object_t *obj)
{
	ivm_cell_t *ret = MEM_ALLOC_INIT(sizeof(*ret), ivm_cell_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("cell"));

	ret->obj = obj;
	ret->prev = IVM_NULL;
	ret->next = IVM_NULL;

	return ret;
}

void
ivm_cell_free(ivm_cell_t *cell)
{
	if (cell) {
		MEM_FREE(cell);
	}

	return;
}

void
ivm_cell_destruct(ivm_cell_t *cell, ivm_vmstate_t *state)
{
	if (cell) {
		ivm_object_destruct(cell->obj, state);
		MEM_FREE(cell);
	}

	return;
}

#define IS_SUC_CELL(p, n) \
	((!(p) || (p)->next == (n)) \
	 && (!(n) || (n)->prev == (p)))

void
ivm_cell_moveBetween(ivm_cell_t *cell,
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
ivm_cell_moveToSet(ivm_cell_t *cell, ivm_cell_set_t *from, ivm_cell_set_t *to)
{
	if (cell && from) {
		if (from->head == cell)
			from->head = cell->next;

		if (from->tail == cell)
			from->tail = cell->prev;
	}
	ivm_cell_set_addCell(to, cell);

	return cell;
}

ivm_cell_set_t *
ivm_cell_set_new()
{
	ivm_cell_set_t *ret = MEM_ALLOC_INIT(sizeof(*ret),
										 ivm_cell_set_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("cell set"));

	ret->head = IVM_NULL;
	ret->tail = IVM_NULL;

	return ret;
}

void
ivm_cell_set_free(ivm_cell_set_t *set)
{
	ivm_cell_t *i, *tmp;

	if (set) {
		for (i = set->head; i;) {
			tmp = i;
			i = i->next;
			ivm_cell_free(tmp);
		}
		MEM_FREE(set);
	}

	return;
}

void
ivm_cell_set_destruct(ivm_cell_set_t *set, ivm_vmstate_t *state)
{
	ivm_cell_t *i, *tmp;

	if (set) {
		for (i = set->head; i;) {
			tmp = i;
			i = i->next;
			ivm_cell_destruct(tmp, state);
		}
		MEM_FREE(set);
	}

	return;
}

void
ivm_cell_set_addCell(ivm_cell_set_t *set, ivm_cell_t *cell)
{
	if (set) {
		ivm_cell_moveBetween(cell, set->tail, IVM_NULL);
		set->tail = cell;
		if (!set->head) {
			set->head = cell;
		}
	}

	return;
}

ivm_cell_t *
ivm_cell_set_addObject(ivm_cell_set_t *set, ivm_object_t *obj)
{
	ivm_cell_t *n_cell;

	if (set) {
		n_cell = ivm_cell_new(obj);
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
ivm_cell_set_removeTail(ivm_cell_set_t *set)
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
