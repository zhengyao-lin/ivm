#ifndef _IVM_VM_STD_CHAIN_H_
#define _IVM_VM_STD_CHAIN_H_

typedef struct ivm_ptchain_cell_t_tag {
	void *ptr;

	struct ivm_ptchain_cell_t_tag *prev;
	struct ivm_ptchain_cell_t_tag *next;
} ivm_ptchain_cell_t;

#define ivm_ptchain_cell_setPtr(cell, p) ((cell)->ptr = (p))

typedef struct {
	ivm_ptchain_cell_t *head;
	ivm_ptchain_cell_t *tail;
} ivm_ptchain_t;

ivm_ptchain_t *
ivm_ptchain_new();

void
ivm_ptchain_free(ivm_ptchain_t *chain);

void
ivm_ptchain_addTail(ivm_ptchain_t *chain,
					void *ptr);

void *
ivm_ptchain_removeTail(ivm_ptchain_t *chain);

#endif
