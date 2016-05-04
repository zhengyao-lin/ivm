#ifndef _IVM_VM_CONTEXT_H_
#define _IVM_VM_CONTEXT_H_

#include "obj.h"
#include "slot.h"

struct ivm_vmstate_t_tag;

typedef ivm_object_t ivm_context_t;
typedef void (*ivm_ctchain_foreach_proc_t)(ivm_context_t *, void *);

struct ivm_ctchain_sub_t_tag {
	ivm_context_t *ct;
	struct ivm_ctchain_sub_t_tag *outer;
	struct ivm_ctchain_sub_t_tag *inner;
};

typedef struct ivm_ctchain_sub_t_tag ivm_ctchain_iterator_t;

typedef struct ivm_ctchain_t_tag {
	struct ivm_ctchain_sub_t_tag *head;
	struct ivm_ctchain_sub_t_tag *tail;
} ivm_ctchain_t;

ivm_ctchain_t *
ivm_ctchain_new();
void
ivm_ctchain_free(ivm_ctchain_t *chain);

void
ivm_ctchain_addContext(ivm_ctchain_t *chain,
					   ivm_context_t *ct);
void
ivm_ctchain_removeContext(ivm_ctchain_t *chain,
						  ivm_context_t *ct);
ivm_object_t *
ivm_ctchain_search(ivm_ctchain_t *chain,
				   struct ivm_vmstate_t_tag *state,
				   const ivm_char_t *key);
ivm_slot_t *
ivm_ctchain_searchSlot(ivm_ctchain_t *chain,
					   struct ivm_vmstate_t_tag *state,
					   const ivm_char_t *key);
ivm_ctchain_t *
ivm_ctchain_clone(ivm_ctchain_t *chain);

#define ivm_ctchain_getLocal(chain) ((chain)->tail ? (chain)->tail->ct : IVM_NULL)
#define ivm_ctchain_getGlobal(chain) ((chain)->head ? (chain)->head->ct : IVM_NULL)

void
ivm_ctchain_foreach(ivm_ctchain_t *chain,
					ivm_ctchain_foreach_proc_t proc,
					void *arg);

#define IVM_CTCHAIN_ITER_SET(iter, val) ((iter)->ct = val)
#define IVM_CTCHAIN_ITER_GET(iter) ((iter)->ct)
#define IVM_CTCHAIN_EACHPTR(chain, ptr) for ((ptr) = (chain)->head; (ptr); (ptr) = (ptr)->inner)

#endif
