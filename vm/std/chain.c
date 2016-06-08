#include "pub/mem.h"
#include "pub/com.h"
#include "pub/err.h"

#include "chain.h"

IVM_PRIVATE
ivm_ptchain_cell_t *
ivm_ptchain_cell_new(void *ptr)
{
	ivm_ptchain_cell_t *ret = MEM_ALLOC(sizeof(*ret),
										ivm_ptchain_cell_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("ptchain cell"));

	ret->ptr = ptr;
	ret->prev
	= ret->next
	= IVM_NULL;

	return ret;
}

IVM_PRIVATE
void
ivm_ptchain_cell_free(ivm_ptchain_cell_t *cell)
{
	MEM_FREE(cell);
	return;
}

#define SET_PREV(c, p) ((c)->prev = (p))
#define SET_NEXT(c, n) ((c)->next = (n))
#define SET_BOTH(c, p, n) \
	(SET_PREV((c), (p)), \
	 SET_NEXT((c), (n)))
#define LINK(p, n) ((p)->next = (n), (n)->prev = (p), (n))
#define VALUE(c) ((c)->ptr)

ivm_ptchain_t *
ivm_ptchain_new()
{
	ivm_ptchain_t *ret = MEM_ALLOC(sizeof(*ret),
								   ivm_ptchain_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("ptchain"));

	ret->head
	= ret->tail
	= IVM_NULL;

	return ret;
}

void
ivm_ptchain_free(ivm_ptchain_t *chain)
{
	ivm_ptchain_cell_t *i, *tmp;

	if (chain) {
		for (i = chain->head; i;) {
			tmp = i;
			i = i->next;
			ivm_ptchain_cell_free(tmp);
		}
		MEM_FREE(chain);
	}

	return;
}

void
ivm_ptchain_addTail_c(ivm_ptchain_t *chain,
					  ivm_ptchain_cell_t *cell)
{
	if (!chain->tail) {
		chain->head = chain->tail = cell;
		SET_BOTH(cell, IVM_NULL, IVM_NULL);
	} else {
		chain->tail
		= LINK(chain->tail, cell);
		SET_NEXT(cell, IVM_NULL);
	}

	return;
}

void
ivm_ptchain_addTail(ivm_ptchain_t *chain,
					void *ptr)
{
	ivm_ptchain_addTail_c(chain,
						  ivm_ptchain_cell_new(ptr));
	return;
}

ivm_ptchain_cell_t *
ivm_ptchain_removeTail_c(ivm_ptchain_t *chain)
{
	ivm_ptchain_cell_t *ret = IVM_NULL;

	if (chain->tail) {
		ret = chain->tail;

		if (chain->head == chain->tail) {
			chain->head = IVM_NULL;
			chain->tail = IVM_NULL;
		} else {
			chain->tail = chain->tail->prev;
			SET_NEXT(chain->tail, IVM_NULL);
		}

		if (ret) {
			SET_BOTH(ret, IVM_NULL, IVM_NULL);
		}
	}

	return ret;
}

void *
ivm_ptchain_removeTail(ivm_ptchain_t *chain)
{
	ivm_ptchain_cell_t *cell = ivm_ptchain_removeTail_c(chain);
	void *ret = VALUE(cell);

	ivm_ptchain_cell_free(cell);

	return ret;
}
