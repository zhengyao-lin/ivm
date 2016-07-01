#ifndef _IVM_STD_POOL_H_
#define _IVM_STD_POOL_H_

#include "pub/mem.h"
#include "pub/com.h"
#include "pub/type.h"
#include "pub/err.h"

#include "chain.h"
#include "list.h"
#include "heap.h"

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

void
ivm_ptpool_init(ivm_ptpool_t *pool,
				ivm_size_t ecount,
				ivm_size_t esize);

IVM_INLINE
void
ivm_ptpool_destruct_s(ivm_ptpool_t pool)
{
	ivm_heap_free(pool.heap);
	ivm_ptlist_free(pool.freed);
	return;
}

IVM_INLINE
void
ivm_ptpool_destruct(ivm_ptpool_t *pool)
{
	if (pool) {
		ivm_heap_free(pool->heap);
		ivm_ptlist_free(pool->freed);
	}

	return;
}

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

IVM_INLINE
void *
ivm_ptpool_alloc_s(ivm_ptpool_t pool)
{
	ivm_byte_t *tmp;

	if (!(tmp = ivm_ptlist_pop(pool.freed))) {
		return ivm_heap_alloc(pool.heap, pool.esize);
	}

	return tmp;
}

/* dump all allocated objects */
IVM_INLINE
void
ivm_ptpool_dumpAll(ivm_ptpool_t *pool)
{
	ivm_heap_reset(pool->heap);
	ivm_ptlist_empty(pool->freed);
	return;
}

IVM_INLINE
void
ivm_ptpool_dumpAll_s(ivm_ptpool_t pool)
{
	ivm_heap_reset(pool.heap);
	ivm_ptlist_empty(pool.freed);
	return;
}

#define ivm_ptpool_dump(pool, ptr) (ivm_ptlist_push((pool)->freed, (ptr)))
#define ivm_ptpool_dump_s(pool, ptr) (ivm_ptlist_push((pool).freed, (ptr)))

IVM_COM_END

#endif
