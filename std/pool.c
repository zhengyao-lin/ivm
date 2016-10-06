#include "pub/err.h"

#include "mem.h"
#include "pool.h"
#include "list.h"
#include "heap.h"

ivm_ptpool_t *
ivm_ptpool_new(ivm_size_t ecount,
			   ivm_size_t esize)
{
	ivm_ptpool_t *ret = STD_ALLOC(sizeof(*ret));

	IVM_MEMCHECK(ret);

	ret->esize = esize;
	ivm_heap_init(&ret->heap, ecount * esize);
	ivm_ptlist_init_c(&ret->freed, ecount);

	return ret;
}

void
ivm_ptpool_free(ivm_ptpool_t *pool)
{
	if (pool) {
		ivm_heap_dump(&pool->heap);
		ivm_ptlist_dump(&pool->freed);

		STD_FREE(pool);
	}

	return;
}

void
ivm_ptpool_init(ivm_ptpool_t *pool,
				ivm_size_t ecount,
				ivm_size_t esize)
{
	pool->esize = esize;
	ivm_heap_init(&pool->heap, ecount * esize);
	ivm_ptlist_init_c(&pool->freed, ecount);

	return;
}
