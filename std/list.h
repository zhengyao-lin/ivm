#ifndef _IVM_VM_STD_LIST_H_
#define _IVM_VM_STD_LIST_H_

#include "pub/mem.h"
#include "pub/com.h"
#include "pub/const.h"
#include "pub/type.h"
#include "pub/err.h"

IVM_COM_HEADER

typedef void (*ivm_ptlist_foreach_proc_t)(void *p);
typedef void (*ivm_ptlist_foreach_proc_arg_t)(void *p, void *arg);

typedef struct {
	ivm_size_t alloc;
	ivm_size_t cur;
	void **lst;
} ivm_ptlist_t;

ivm_ptlist_t *
ivm_ptlist_new_c(ivm_size_t buf_size);

#define ivm_ptlist_new() (ivm_ptlist_new_c(IVM_DEFAULT_PTLIST_BUFFER_SIZE))

void
ivm_ptlist_free(ivm_ptlist_t *ptlist);

#define ivm_ptlist_setBufferSize(ptlist, size) ((ptlist)->buf_size = (size))

#define ivm_ptlist_last(ptlist) ((ptlist)->cur > 0 ? (ptlist)->lst[(ptlist)->cur - 1] : IVM_NULL)
#define ivm_ptlist_size(ptlist) ((ptlist)->cur)
#define ivm_ptlist_at(ptlist, i) ((ptlist)->lst[i])
#define ivm_ptlist_ptrAt(ptlist, i) ((ptlist)->lst + (i))
#define ivm_ptlist_set(ptlist, i, val) ((ptlist)->lst[i] = (val))

IVM_INLINE
void
ivm_ptlist_inc(ivm_ptlist_t *ptlist)
{
	ptlist->alloc <<= 1;
	ptlist->lst = MEM_REALLOC(ptlist->lst,
							  sizeof(*ptlist->lst)
							  * ptlist->alloc,
							  void **);
	IVM_ASSERT(ptlist->lst, IVM_ERROR_MSG_FAILED_ALLOC_NEW("increased ptlist space"));

	return;
}

#define ivm_ptlist_push(ptlist, p) \
	(((ptlist)->cur >= (ptlist)->alloc ? ivm_ptlist_inc(ptlist), 0 : 0), \
	 (ptlist)->lst[(ptlist)->cur] = (p), (ptlist)->cur++)

#define ivm_ptlist_pop(ptlist) \
	((ptlist)->cur ? (ptlist)->lst[--(ptlist)->cur] : IVM_NULL)

#define ivm_ptlist_setCur(ptlist, t) ((ptlist)->cur = (t))
#define ivm_ptlist_incCur(ptlist, t) ((ptlist)->cur += (t))
#define ivm_ptlist_empty(ptlist) (ivm_ptlist_setCur((ptlist), 0))
#define ivm_ptlist_has(ptlist, i) ((ptlist)->cur > (i))

void
ivm_ptlist_foreach(ivm_ptlist_t *ptlist, ivm_ptlist_foreach_proc_t proc);
void
ivm_ptlist_foreach_arg(ivm_ptlist_t *ptlist, ivm_ptlist_foreach_proc_arg_t proc, void *arg);

void
ivm_ptlist_compact(ivm_ptlist_t *ptlist);

#define IVM_PTLIST_ITER_TYPE(elem_type) elem_type *
#define IVM_PTLIST_ITER_SET(iter, val) (*(iter) = val)
#define IVM_PTLIST_ITER_GET(iter) (*(iter))
#define IVM_PTLIST_EACHPTR(ptlist, iter, type) \
	for ((iter) = (type *)((ptlist)->lst); \
		 (iter) != ((type *)(ptlist)->lst) + (ptlist)->cur; \
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

#define ivm_list_at(list, i) ((void *)((list)->lst + ((i) * (list)->esize)))
#define ivm_list_size(list) ((list)->cur)

#define IVM_LIST_ITER_TYPE(elem_type) elem_type *
#define IVM_LIST_ITER_SET(iter, val, type) (*((type *)(iter)) = val)
#define IVM_LIST_ITER_GET(iter, type) (*((type *)(iter)))
#define IVM_LIST_ITER_GET_PTR(iter, type) ((type *)(iter))
#define IVM_LIST_EACHPTR(list, iter, type) \
	for ((iter) = (type *)((list)->lst); \
		 (iter) != ((type *)(list)->lst) + (list)->cur; \
		 (iter)++)

IVM_COM_END

#endif
