#ifndef _IVM_STD_CHAIN_H_
#define _IVM_STD_CHAIN_H_

#include "pub/com.h"

IVM_COM_HEADER

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

IVM_COM_END

#endif
