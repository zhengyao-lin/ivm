#ifndef _IVM_VM_CONTEXT_H_
#define _IVM_VM_CONTEXT_H_

#include "obj.h"

struct ivm_vmstate_t_tag;
typedef ivm_object_t ivm_context_t;

struct ivm_ctchain_sub_t_tag {
	ivm_context_t *ct;
	struct ivm_ctchain_sub_t_tag *outer;
	struct ivm_ctchain_sub_t_tag *inner;
};

typedef struct {
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

#endif
