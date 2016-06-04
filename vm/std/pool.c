#include "pub/mem.h"
#include "pool.h"
#include "list.h"
#include "err.h"

ivm_ptpool_t *
ivm_ptpool_new(ivm_size_t ecount,
			   ivm_size_t esize)
{
	ivm_ptpool_t *ret = MEM_ALLOC(sizeof(*ret),
								  ivm_ptpool_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("ptpool"));

	ret->ecount = ecount;
	ret->esize = esize;

	ret->bcount = 1;
	ret->bcur = 0;
	ret->blocks = MEM_ALLOC(sizeof(*ret->blocks),
							ivm_byte_t **);
	ret->blocks[0] = MEM_ALLOC(ecount * esize,
							   ivm_byte_t *);
	IVM_ASSERT(ret->blocks && ret->blocks[0],
			   IVM_ERROR_MSG_FAILED_ALLOC_NEW("ptpool block"));

	ret->freed = ivm_ptlist_new_c(ecount);

	return ret;
}

void
ivm_ptpool_free(ivm_ptpool_t *pool)
{
	ivm_size_t i;

	if (pool) {
		for (i = 0; i < pool->bcount; i++) {
			MEM_FREE(pool->blocks[i]);
		}
		
		MEM_FREE(pool->blocks);
		ivm_ptlist_free(pool->freed);

		MEM_FREE(pool);
	}

	return;
}

void *
ivm_ptpool_alloc(ivm_ptpool_t *pool)
{
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
}
