#ifndef _IVM_VM_CONTEXT_H_
#define _IVM_VM_CONTEXT_H_

#include "obj.h"

typedef ivm_object_t ivm_context_t;

struct ivm_ct_chain_sub_t {
	ivm_context_t *ct;
	struct ivm_ct_chain_sub_t *outer;
	struct ivm_ct_chain_sub_t *inner;
};

typedef struct {
	ivm_ct_chain_sub_t *head;
	ivm_ct_chain_sub_t *tail;
} ivm_ctchain_t;

ivm_ctchain_t *
ivm_ctchain_new();
void
ivm_ctchain_free(ivm_ctchain_t *chain);

#endif
