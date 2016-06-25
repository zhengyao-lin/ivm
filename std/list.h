#ifndef _IVM_STD_LIST_H_
#define _IVM_STD_LIST_H_

#include "pub/mem.h"
#include "pub/com.h"
#include "pub/const.h"
#include "pub/type.h"
#include "pub/err.h"

IVM_COM_HEADER

typedef struct {
	ivm_size_t alloc;
	void **end;
	ivm_size_t cur;
	void **lst;
} ivm_ptlist_t;

ivm_ptlist_t *
ivm_ptlist_new_c(ivm_size_t buf_size);

#define ivm_ptlist_new() (ivm_ptlist_new_c(IVM_DEFAULT_PTLIST_BUFFER_SIZE))

void
ivm_ptlist_free(ivm_ptlist_t *ptlist);

#define ivm_ptlist_setBufferSize(ptlist, size) ((ptlist)->buf_size = (size))

#define ivm_ptlist_size(ptlist) ((ptlist)->cur)
#define ivm_ptlist_at(ptlist, i) ((ptlist)->lst[i])
#define ivm_ptlist_ptrAt(ptlist, i) ((ptlist)->lst + (i))
#define ivm_ptlist_set(ptlist, i, val) ((ptlist)->lst[i] = (val))

IVM_INLINE
void
ivm_ptlist_inc(ivm_ptlist_t *ptlist)
{
	ptlist->alloc <<= 1;
	ptlist->end = (
		(ptlist->lst = MEM_REALLOC(ptlist->lst,
								   sizeof(*ptlist->lst)
								   * ptlist->alloc,
								   void **))
		+ ptlist->alloc
	);
	IVM_ASSERT(ptlist->lst, IVM_ERROR_MSG_FAILED_ALLOC_NEW("increased ptlist space"));

	return;
}

IVM_INLINE
ivm_size_t
ivm_ptlist_push(ivm_ptlist_t *ptlist, void *p)
{
	if (ptlist->cur >= ptlist->alloc) {
		ivm_ptlist_inc(ptlist);
	}

	ptlist->lst[ptlist->cur] = p;

	return ptlist->cur++;
}

IVM_INLINE
void *
ivm_ptlist_pop(ivm_ptlist_t *ptlist)
{
	return ptlist->cur ? ptlist->lst[--ptlist->cur] : IVM_NULL;
}

#define ivm_ptlist_setCur(ptlist, t) ((ptlist)->cur = (t))
#define ivm_ptlist_incCur(ptlist, t) ((ptlist)->cur += (t))
#define ivm_ptlist_empty(ptlist) (ivm_ptlist_setCur((ptlist), 0))
#define ivm_ptlist_has(ptlist, i) ((ptlist)->cur > (i))

/* similar to iterator */
#define ivm_ptlist_core(ptlist) ((ptlist)->lst)
#define ivm_ptlist_end(ptlist) ((ptlist)->end)
#define ivm_ptlist_offset(ptlist, ptr) \
	((ivm_ptr_t)(ptr) - (ivm_ptr_t)(ptlist)->lst) / sizeof(void *)

void
ivm_ptlist_compact(ivm_ptlist_t *ptlist);

#define IVM_PTLIST_ITER_TYPE(elem_type) elem_type *
#define IVM_PTLIST_ITER_BEGIN(ptlist) ((ptlist)->lst)
#define IVM_PTLIST_ITER_END(ptlist) ((ptlist)->lst + (ptlist)->cur)
#define IVM_PTLIST_ITER_AT(ptlist, i) ((ptlist)->lst + (i))
#define IVM_PTLIST_ITER_INDEX(ptlist, i) ((void **)(i) - (ptlist)->lst)
#define IVM_PTLIST_ITER_SET(iter, val) (*(iter) = val)
#define IVM_PTLIST_ITER_GET(iter) (*(iter))
#define IVM_PTLIST_EACHPTR(ptlist, iter, type) \
	type *__pl_end_##iter##__; \
	for ((iter) = (type *)((ptlist)->lst), \
		 __pl_end_##iter##__ = (iter) + (ptlist)->cur; \
		 (iter) != __pl_end_##iter##__; \
		 (iter)++)

typedef int (*ivm_ptlist_comparer_t)(const void *, const void *);

ivm_size_t
ivm_ptlist_indexOf_c(ivm_ptlist_t *ptlist, void *ptr, ivm_ptlist_comparer_t comp);

#define ivm_ptlist_indexOf(ptlist, ptr, comp) \
	(ivm_ptlist_indexOf_c((ptlist), \
						  (ptr), \
						  (ivm_ptlist_comparer_t)(comp)))

IVM_INLINE
void
ivm_ptlist_incTo(ivm_ptlist_t *ptlist,
				 ivm_size_t size)
{
	if (size > ptlist->alloc) {
		ptlist->alloc = size;
		ptlist->lst = MEM_REALLOC(ptlist->lst,
								  sizeof(*ptlist->lst)
								  * ptlist->alloc,
								  void **);
		IVM_ASSERT(ptlist->lst, IVM_ERROR_MSG_FAILED_ALLOC_NEW("increased ptlist space"));
	}

	return;
}

IVM_INLINE
void
ivm_ptlist_insert(ivm_ptlist_t *ptlist,
			   ivm_size_t i,
			   void *p)
{
	if (i >= ptlist->cur) {
		ivm_ptlist_incTo(ptlist, i + 1);
		MEM_INIT(ptlist->lst + ptlist->cur,
				 sizeof(*ptlist->lst) * (i - ptlist->cur));
		ptlist->cur = i + 1;
	}

	ptlist->lst[i] = p;

	return;
}

typedef struct {
	ivm_size_t esize;

	ivm_size_t alloc;
	ivm_size_t cur;
	ivm_byte_t *lst;
} ivm_list_t;

ivm_list_t *
ivm_list_new_c(ivm_size_t esize, ivm_size_t buf_size);

#define ivm_list_new(size) (ivm_list_new_c((size), IVM_DEFAULT_LIST_BUFFER_SIZE))

void
ivm_list_free(ivm_list_t *list);

IVM_INLINE
void
_ivm_list_expand(ivm_list_t *list)
{
	list->alloc <<= 1;
	list->lst = MEM_REALLOC(list->lst,
							list->esize * list->alloc,
							ivm_byte_t *);
	return;
}

IVM_INLINE
ivm_size_t
ivm_list_push(ivm_list_t *list, void *e)
{
	if (list->cur >= list->alloc) {
		_ivm_list_expand(list);
	}

	MEM_COPY(list->lst + (list->cur * list->esize),
			 e, list->esize);

	return ++list->cur;
}

IVM_INLINE
void *
ivm_list_at(ivm_list_t *list, ivm_size_t i)
{
	return list->lst + (i * list->esize);
}

#define ivm_list_size(list) ((list)->cur)
#define ivm_list_empty(list) ((list)->cur = 0)
#define ivm_list_core(list) ((list)->lst)

#define IVM_LIST_ITER_TYPE(elem_type) elem_type *
#define IVM_LIST_ITER_SET(iter, val, type) (*((type *)(iter)) = val)
#define IVM_LIST_ITER_GET(iter, type) (*((type *)(iter)))
#define IVM_LIST_ITER_GET_PTR(iter, type) ((type *)(iter))
#define IVM_LIST_EACHPTR(list, iter, type) \
	type *__l_end_##iter##__; \
	for ((iter) = (type *)((list)->lst), \
		 __l_end_##iter##__ = ((type *)(iter)) + (list)->cur; \
		 (iter) != __l_end_##iter##__; \
		 (iter)++)

IVM_COM_END

#endif
