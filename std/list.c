#include "pub/mem.h"
#include "pub/err.h"

#include "list.h"

ivm_ptlist_t *
ivm_ptlist_new_c(ivm_size_t buf_size)
{
	ivm_ptlist_t *ret = MEM_ALLOC(sizeof(*ret),
								  ivm_ptlist_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("ptlist"));

	ret->alloc
	= buf_size;

	ret->cur = 0;
	ret->lst = MEM_ALLOC(sizeof(*ret->lst)
						 * buf_size,
						 void **);

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
ivm_ptlist_init_c(ivm_ptlist_t *ptlist,
				  ivm_size_t buf_size)
{
	ptlist->alloc
	= buf_size;

	ptlist->cur = 0;
	ptlist->lst = MEM_ALLOC(sizeof(*ptlist->lst)
							* buf_size,
							void **);

	IVM_ASSERT(ptlist->lst, IVM_ERROR_MSG_FAILED_ALLOC_NEW("ptlist buffer"));

	return;
}

#define VALUE_AT(ptlist, i) ((ptlist)->lst[i])

void
ivm_ptlist_compact(ivm_ptlist_t *ptlist)
{
	ptlist->lst = MEM_REALLOC(ptlist->lst,
							  sizeof(*ptlist->lst)
							  * (ptlist->alloc = ptlist->cur),
							  void **);
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
	ivm_list_t *ret = MEM_ALLOC(sizeof(*ret),
								ivm_list_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("list"));

	ret->esize = esize;
	ret->alloc = buf_size;
	ret->cur = 0;
	ret->lst = MEM_ALLOC(esize * buf_size,
						 ivm_byte_t *);

	IVM_ASSERT(ret->lst, IVM_ERROR_MSG_FAILED_ALLOC_NEW("list buffer"));

	return ret;
}

void
ivm_list_free(ivm_list_t *list)
{
	if (list) {
		MEM_FREE(list->lst);
		MEM_FREE(list);
	}

	return;
}
