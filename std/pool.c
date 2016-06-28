#include "pub/mem.h"
#include "pub/err.h"

#include "pool.h"
#include "list.h"
#include "heap.h"

ivm_ptpool_t *
ivm_ptpool_new(ivm_size_t ecount,
			   ivm_size_t esize)
{
	ivm_ptpool_t *ret = MEM_ALLOC(sizeof(*ret),
								  ivm_ptpool_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("ptpool"));

	ret->esize = esize;
	ret->heap = ivm_heap_new(ecount * esize);
	ret->freed = ivm_ptlist_new_c(ecount);

	return ret;
}

void
ivm_ptpool_free(ivm_ptpool_t *pool)
{
	if (pool) {
		ivm_heap_free(pool->heap);
		ivm_ptlist_free(pool->freed);

		MEM_FREE(pool);
	}

	return;
}

void
ivm_ptpool_init(ivm_ptpool_t *pool,
				ivm_size_t ecount,
				ivm_size_t esize)
{
	pool->esize = esize;
	pool->heap = ivm_heap_new(ecount * esize);
	pool->freed = ivm_ptlist_new_c(ecount);

	return;
}
