#include "pub/err.h"

#include "mem.h"
#include "list.h"

ivm_ptlist_t *
ivm_ptlist_new_c(ivm_size_t buf_size)
{
	ivm_ptlist_t *ret = STD_ALLOC(sizeof(*ret));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("ptlist"));

	if (!buf_size) buf_size = 1;

	ret->alloc
	= buf_size;

	ret->cur = 0;
	ret->lst = STD_ALLOC(sizeof(*ret->lst) * buf_size);

	IVM_ASSERT(ret->lst, IVM_ERROR_MSG_FAILED_ALLOC_NEW("ptlist buffer"));

	return ret;
}

void
ivm_ptlist_free(ivm_ptlist_t *ptlist)
{
	if (ptlist) {
		STD_FREE(ptlist->lst);
		STD_FREE(ptlist);
	}

	return;
}

void
ivm_ptlist_init_c(ivm_ptlist_t *ptlist,
				  ivm_size_t buf_size)
{
	ptlist->alloc
	= buf_size;

	ptlist->cur = 0;
	ptlist->lst = STD_ALLOC(sizeof(*ptlist->lst) * buf_size);

	IVM_ASSERT(ptlist->lst, IVM_ERROR_MSG_FAILED_ALLOC_NEW("ptlist buffer"));

	return;
}

void
ivm_ptlist_init_t(ivm_ptlist_t *ptlist,
				  void **lst,
				  ivm_size_t size)
{
	ivm_size_t alloc = sizeof(*ptlist->lst) * size;

	ptlist->alloc = size;

	ptlist->cur = size;
	ptlist->lst = STD_ALLOC(alloc);

	IVM_ASSERT(ptlist->lst, IVM_ERROR_MSG_FAILED_ALLOC_NEW("ptlist buffer"));

	STD_MEMCPY(ptlist->lst, lst, alloc);

	return;
}

#define VALUE_AT(ptlist, i) ((ptlist)->lst[i])

void
ivm_ptlist_compact(ivm_ptlist_t *ptlist)
{
	ptlist->lst = STD_REALLOC(
		ptlist->lst,
		sizeof(*ptlist->lst) * (ptlist->alloc = ptlist->cur)
	);

	IVM_ASSERT(ptlist->lst, IVM_ERROR_MSG_FAILED_ALLOC_NEW("compacted pointer list"));

	return;
}

ivm_size_t
ivm_ptlist_indexOf_c(ivm_ptlist_t *ptlist, void *ptr,
					 ivm_ptlist_comparer_t comp)
{
	void **i, **end;

	for (i = ptlist->lst,
		 end = i + ptlist->cur;
		 i != end; i++) {
		if (!comp(*i, ptr))
			return ivm_ptlist_offset(ptlist, i);
	}

	return -1;
}

ivm_list_t *
ivm_list_new_c(ivm_size_t esize, ivm_size_t buf_size)
{
	ivm_list_t *ret = STD_ALLOC(sizeof(*ret));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("list"));

	if (!buf_size) buf_size = 1;

	ret->esize = esize;
	ret->alloc = buf_size;
	ret->cur = 0;
	ret->lst = STD_ALLOC(esize * buf_size);

	IVM_ASSERT(ret->lst, IVM_ERROR_MSG_FAILED_ALLOC_NEW("list buffer"));

	return ret;
}

void
ivm_list_init_c(ivm_list_t *list,
				ivm_size_t esize,
				ivm_size_t buf_size)
{
	if (!buf_size) buf_size = 1;

	list->esize = esize;
	list->alloc = buf_size;
	list->cur = 0;
	list->lst = STD_ALLOC(esize * buf_size);

	IVM_ASSERT(list->lst, IVM_ERROR_MSG_FAILED_ALLOC_NEW("list buffer"));

	return;
}

void
ivm_list_free(ivm_list_t *list)
{
	if (list) {
		STD_FREE(list->lst);
		STD_FREE(list);
	}

	return;
}
