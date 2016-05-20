#ifndef _IVM_VM_STD_LIST_H_
#define _IVM_VM_STD_LIST_H_

#include "pub/com.h"
#include "pub/const.h"
#include "pub/type.h"

IVM_COM_HEADER

#if IVM_DEBUG

#define IVM_DEFAULT_LIST_BUFFER_SIZE 1

#else

#define IVM_DEFAULT_LIST_BUFFER_SIZE 32

#endif

typedef void (*ivm_ptlist_foreach_proc_t)(void *p);
typedef void (*ivm_ptlist_foreach_proc_arg_t)(void *p, void *arg);

typedef struct {
	ivm_size_t buf_size;
	ivm_size_t alloc;
	ivm_size_t cur;
	void **lst;
} ivm_ptlist_t;

ivm_ptlist_t *
ivm_ptlist_new_c(ivm_size_t buf_size);
#define ivm_ptlist_new() (ivm_ptlist_new_c(IVM_DEFAULT_LIST_BUFFER_SIZE))
void
ivm_ptlist_free(ivm_ptlist_t *ptlist);
void
ivm_ptlist_inc(ivm_ptlist_t *ptlist);

#define ivm_ptlist_setBufferSize(ptlist, size) ((ptlist)->buf_size = (size))

#define ivm_ptlist_last(ptlist) ((ptlist)->cur > 0 ? (ptlist)->lst[(ptlist)->cur - 1] : IVM_NULL)
#define ivm_ptlist_size(ptlist) ((ptlist)->cur)
#define ivm_ptlist_at(ptlist, i) ((ptlist)->lst[i])

ivm_size_t
ivm_ptlist_push(ivm_ptlist_t *ptlist, void *p);
void *
ivm_ptlist_pop(ivm_ptlist_t *ptlist);

#define ivm_ptlist_setCur(ptlist, t) ((ptlist)->cur = t)

void
ivm_ptlist_foreach(ivm_ptlist_t *ptlist, ivm_ptlist_foreach_proc_t proc);
void
ivm_ptlist_foreach_arg(ivm_ptlist_t *ptlist, ivm_ptlist_foreach_proc_arg_t proc, void *arg);

void
ivm_ptlist_compact(ivm_ptlist_t *ptlist);

#define IVM_PTLIST_ITER_TYPE(elem_type) elem_type *
#define IVM_PTLIST_EACHPTR(ptlist, ptr, type) \
	for ((ptr) = (type *)((ptlist)->lst); \
		 (ptr) != &(((type *)(ptlist)->lst)[(ptlist)->cur]); \
		 (ptr)++)

typedef int (*ivm_ptlist_comparer_t)(const void *, const void *);

ivm_size_t
ivm_ptlist_indexOf_c(ivm_ptlist_t *ptlist, void *ptr, ivm_ptlist_comparer_t comp);

#define ivm_ptlist_indexOf(ptlist, ptr, comp) \
	(ivm_ptlist_indexOf_c((ptlist), \
						  (ptr), \
						  (ivm_ptlist_comparer_t)(comp)))

IVM_COM_END

#endif
