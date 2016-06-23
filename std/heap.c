#include "pub/mem.h"
#include "pub/com.h"
#include "pub/err.h"
#include "pub/vm.h"
#include "pub/type.h"

#include "heap.h"

ivm_heap_t *
ivm_heap_new(ivm_size_t bsize)
{
	ivm_heap_t *ret = MEM_ALLOC(sizeof(*ret),
								ivm_heap_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("heap"));

	ret->bcount = 1;
	ret->bsize = bsize;

	ret->btop = 0;
	ret->blocks = MEM_ALLOC(sizeof(*ret->blocks),
							ivm_byte_t **);
	
	IVM_ASSERT(ret->blocks, IVM_ERROR_MSG_FAILED_ALLOC_NEW("heap block"));

	ret->bcurp = ret->blocks[0] = MEM_ALLOC(bsize, ivm_byte_t *);

	IVM_ASSERT(ret->bcurp, IVM_ERROR_MSG_FAILED_ALLOC_NEW("heap block"));

	ret->bendp = ret->bcurp + bsize;

	return ret;
}

void
ivm_heap_free(ivm_heap_t *heap)
{
	ivm_size_t i;
	if (heap) {
		for (i = 0; i < heap->bcount; i++)
			MEM_FREE(heap->blocks[i]);
		MEM_FREE(heap->blocks);
		MEM_FREE(heap);
	}

	return;
}

void
ivm_heap_reset(ivm_heap_t *heap)
{
	heap->btop = 0;
	heap->bcurp = *heap->blocks;
	heap->bendp = *heap->blocks + heap->bsize;
	return;
}

void
ivm_heap_compact(ivm_heap_t *heap)
{
	ivm_byte_t **i, **end;

	for (i = heap->blocks + heap->btop + 1,
		 end = heap->blocks + heap->bcount;
		 i != end; i++) {
		MEM_FREE(*i);
	}

	heap->bcount = heap->btop + 1;
	heap->blocks = MEM_REALLOC(heap->blocks,
							   sizeof(*heap->blocks)
							   * heap->bcount,
							   ivm_byte_t **);

	return;
}
