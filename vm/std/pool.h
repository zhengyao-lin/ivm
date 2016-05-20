#ifndef _IVM_VM_STD_POOL_H_
#define _IVM_VM_STD_POOL_H_

#include "pub/com.h"
#include "pub/type.h"
#include "chain.h"
#include "list.h"

IVM_COM_HEADER

typedef struct {
	ivm_size_t ecount; /* element count */
	ivm_size_t esize; /* element size */

	ivm_size_t bcount; /* block count */
	ivm_size_t bcur; /* current element */
	ivm_byte_t **blocks; /* preallocated room */

	ivm_ptlist_t *freed; /* freed ptrs */
} ivm_ptpool_t;

ivm_ptpool_t *
ivm_ptpool_new(ivm_size_t ecount,
			   ivm_size_t esize);

void
ivm_ptpool_free(ivm_ptpool_t *pool);

void *
ivm_ptpool_alloc(ivm_ptpool_t *pool);

void
ivm_ptpool_dump(ivm_ptpool_t *pool, void *ptr);

IVM_COM_END

#endif
