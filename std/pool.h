#ifndef _IVM_STD_POOL_H_
#define _IVM_STD_POOL_H_

#include "pub/mem.h"
#include "pub/com.h"
#include "pub/type.h"

#include "chain.h"
#include "list.h"
#include "heap.h"

#include "../err.h"

IVM_COM_HEADER

typedef struct {
	ivm_size_t esize; /* element size */

	ivm_heap_t *heap;

	ivm_ptlist_t *freed; /* freed ptrs */
} ivm_ptpool_t;

ivm_ptpool_t *
ivm_ptpool_new(ivm_size_t ecount,
			   ivm_size_t esize);

void
ivm_ptpool_free(ivm_ptpool_t *pool);

IVM_INLINE
void *
ivm_ptpool_alloc(ivm_ptpool_t *pool)
{
	ivm_byte_t *tmp;

	if (!(tmp = ivm_ptlist_pop(pool->freed))) {
		return ivm_heap_alloc(pool->heap, pool->esize);
	}

	return tmp;
}


#if 0
	ivm_byte_t *tmp;
	ivm_size_t i, ecount, esize;

	if (!(tmp = ivm_ptlist_pop(pool->freed))) {
		/* no more freed element */
		ecount = pool->ecount;
		esize = pool->esize;
		i = pool->bcur % ecount * esize;

		if (pool->bcur
			/ ecount >= pool->bcount) {
			/* add block */
			pool->blocks
			= MEM_REALLOC(pool->blocks,
						  sizeof(*pool->blocks)
						  * ++pool->bcount,
						  ivm_byte_t **);

			pool->blocks[pool->bcount - 1]
			= MEM_ALLOC(ecount * esize, ivm_byte_t *);

			IVM_ASSERT(pool->blocks
					   && pool->blocks[pool->bcount - 1],
					   IVM_ERROR_MSG_FAILED_ALLOC_NEW("ptpool"));
		}

		pool->bcur++;
		tmp = &pool->blocks[pool->bcount - 1][i];
	}

	return tmp;
#endif

#define ivm_ptpool_dump(pool, ptr) (ivm_ptlist_push((pool)->freed, (ptr)))

IVM_COM_END

#endif
