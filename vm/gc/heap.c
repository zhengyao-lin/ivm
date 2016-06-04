#include "pub/mem.h"
#include "pub/com.h"
#include "heap.h"
#include "type.h"
#include "obj.h"
#include "err.h"
#include "vm.h"

#define OFFSET(ptr, size) (&(ptr)[size])
#define HAS_SIZE ivm_heap_hasSize
#define INC_SIZE(heap, idx, s) ((heap)->curs[idx] += (s))

ivm_heap_t *
ivm_heap_new(ivm_size_t bsize)
{
	ivm_heap_t *ret = MEM_ALLOC(sizeof(*ret),
								ivm_heap_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("heap"));

	ret->bcount = 1;
	ret->bsize = bsize;
	ret->btop = 0;
	ret->curs = MEM_ALLOC(sizeof(*ret->curs),
						  ivm_size_t *);
	ret->blocks = MEM_ALLOC(sizeof(*ret->blocks),
							ivm_byte_t **);
	
	IVM_ASSERT(ret->curs && ret->blocks, IVM_ERROR_MSG_FAILED_ALLOC_NEW("heap block"));

	ret->curs[0] = 0;
	ret->blocks[0] = MEM_ALLOC(bsize, ivm_byte_t *);

	return ret;
}

void
ivm_heap_free(ivm_heap_t *heap)
{
	ivm_size_t i;
	if (heap) {
		MEM_FREE(heap->curs);
		for (i = 0; i < heap->bcount; i++)
			MEM_FREE(heap->blocks[i]);
		MEM_FREE(heap->blocks);
		MEM_FREE(heap);
	}

	return;
}

void *
ivm_heap_addCopy(ivm_heap_t *heap, void *ptr, ivm_size_t size)
{
	void *new_ptr = ivm_heap_alloc(heap, size);

	MEM_COPY(new_ptr, ptr, size);

	return new_ptr;
}

void
ivm_heap_reset(ivm_heap_t *heap)
{
	MEM_FREE(heap->curs);
	heap->curs = MEM_ALLOC_INIT(sizeof(*heap->curs) * heap->bcount,
								ivm_size_t *);
	return;
}

void
ivm_heap_compact(ivm_heap_t *heap)
{
	ivm_size_t i, bcount = 0;

	for (i = 0; i < heap->bcount; i++) {
		if (heap->curs[i]) {
			heap->curs[bcount] = heap->curs[i];
			heap->blocks[bcount] = heap->blocks[i];
			bcount++;
		} else {
			MEM_FREE(heap->blocks[i]);
		}
	}

	IVM_OUT("compact to %ld\n", bcount);

	heap->bcount = bcount;
	heap->curs = MEM_REALLOC(heap->curs,
							 sizeof(*heap->curs) * bcount,
							 ivm_size_t *);
	heap->blocks = MEM_REALLOC(heap->blocks,
							   sizeof(*heap->blocks) * bcount,
							   ivm_byte_t **);

	return;
}
