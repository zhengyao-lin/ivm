#include "pub/mem.h"
#include "list.h"
#include "err.h"

ivm_ptlist_t *
ivm_ptlist_new()
{
	ivm_ptlist_t *ret = MEM_ALLOC(sizeof(*ret));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("ptlist"));

	ret->alloc
	= ret->buf_size
	= IVM_DEFAULT_LIST_BUFFER_SIZE;

	ret->cur = 0;
	ret->lst = MEM_ALLOC_INIT(sizeof(*ret->lst)
							  * ret->buf_size);

	IVM_ASSERT(ret->lst, IVM_ERROR_MSG_FAILED_ALLOC_NEW("ptlist buffer"));

	return ret;
}

void
ivm_ptlist_free(ivm_ptlist_t *ptlist)
{
	if (ptlist) {
		MEM_FREE(ptlist->lst);
		MEM_FREE(ptlist);
	}

	return;
}

void
ivm_ptlist_inc(ivm_ptlist_t *ptlist)
{
	ptlist->lst = MEM_REALLOC(ptlist->lst,
							  sizeof(*ptlist->lst)
							  * (ptlist->alloc + ptlist->buf_size));
	IVM_ASSERT(ptlist->lst, IVM_ERROR_MSG_FAILED_ALLOC_NEW("increased ptlist space"));
	ptlist->alloc += ptlist->buf_size;

	return;
}

void
ivm_ptlist_push(ivm_ptlist_t *ptlist, void *p)
{
	if (ptlist->cur >= ptlist->alloc)
		ivm_ptlist_inc(ptlist);

	ptlist->lst[ptlist->cur++] = p;

	return;
}

void *
ivm_ptlist_pop(ivm_ptlist_t *ptlist)
{
	if (ptlist->cur > 0)
		return ptlist->lst[--ptlist->cur];
	return IVM_NULL;
}

#define VALUE_AT(ptlist, i) ((ptlist)->lst[i])

void
ivm_ptlist_foreach(ivm_ptlist_t *ptlist, ivm_ptlist_foreach_proc_t proc)
{
	ivm_size_t i;

	for (i = 0; i < ptlist->cur; i++)
		proc(VALUE_AT(ptlist, i));

	return;
}
