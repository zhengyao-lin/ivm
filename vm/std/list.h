#ifndef _IVM_VM_STD_LIST_H_
#define _IVM_VM_STD_LIST_H_

#define IVM_DEFAULT_LIST_BUFFER_SIZE 50

typedef void (*ivm_ptlist_foreach_proc_t)(void *p);

typedef struct {
	ivm_size_t buf_size;
	ivm_size_t alloc;
	ivm_size_t cur;
	void **lst;
} ivm_ptlist_t;

ivm_ptlist_t *
ivm_ptlist_new();
void
ivm_ptlist_free(ivm_ptlist_t *ptlist);
void
ivm_ptlist_inc(ivm_ptlist_t *ptlist);

#define ivm_ptlist_setBufferSize(ptlist, size) ((ptlist)->buf_size = (size))

#define ivm_ptlist_last(ptlist) ((ptlist)->cur > 0 ? (ptlist)->lst[(ptlist)->cur - 1] : IVM_NULL)
#define ivm_ptlist_size(ptlist) ((ptlist)->cur)

void
ivm_ptlist_push(ivm_ptlist_t *ptlist, void *p);
void *
ivm_ptlist_pop(ivm_ptlist_t *ptlist);

#define ivm_ptlist_setCur(ptlist, t) ((ptlist)->cur = t)

void
ivm_ptlist_foreach(ivm_ptlist_t *ptlist, ivm_ptlist_foreach_proc_t proc);

#endif
