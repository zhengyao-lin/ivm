#include "pub/mem.h"
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
	ivm_heap_t *ret = MEM_ALLOC(sizeof(*ret));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("heap"));

	ret->bcount = 1;
	ret->bsize = bsize;
	ret->curs = MEM_ALLOC(sizeof(*ret->curs));
	ret->blocks = MEM_ALLOC(sizeof(*ret->blocks));
	
	IVM_ASSERT(ret->curs && ret->blocks, IVM_ERROR_MSG_FAILED_ALLOC_NEW("heap block"));

	ret->curs[0] = 0;
	ret->blocks[0] = MEM_ALLOC(bsize);

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

static
ivm_size_t
ivm_heap_addBlock(ivm_heap_t *heap)
{
	printf("new block added: %ld\n", heap->bcount);
	heap->curs = MEM_REALLOC(heap->curs,
							 sizeof(*heap->curs) * ++heap->bcount);
	heap->blocks = MEM_REALLOC(heap->blocks,
							   sizeof(*heap->blocks) * heap->bcount);

	heap->curs[heap->bcount - 1] = 0;
	heap->blocks[heap->bcount - 1] = MEM_ALLOC(heap->bsize);

	return heap->bcount;
}

static
void *
ivm_heap_allocAt(ivm_heap_t *heap, ivm_size_t idx, ivm_size_t size)
{
	void *ret = OFFSET(heap->blocks[idx], heap->curs[idx]);

	INC_SIZE(heap, idx, size);

	return ret;
}

void *
ivm_heap_alloc(ivm_heap_t *heap, ivm_size_t size)
{
	ivm_size_t i;

	IVM_ASSERT(size <= heap->bsize,
			   IVM_ERROR_MSG_SIZE_EXCEEDS_BLOCK_SIZE);

	if (!(i = HAS_SIZE(heap, size))) {
		i = ivm_heap_addBlock(heap);
	}

	return ivm_heap_allocAt(heap, i - 1, size);
}

void *
ivm_heap_addCopy(ivm_heap_t *heap, void *ptr, ivm_size_t size)
{
	void *new_ptr = ivm_heap_alloc(heap, size);

	MEM_COPY(new_ptr, ptr, size);

	return new_ptr;
}

ivm_size_t
ivm_heap_hasSize(ivm_heap_t *heap, ivm_size_t size)
{
	ivm_size_t i;

	for (i = 0; i < heap->bcount; i++) {
		if (heap->bsize - heap->curs[i] >= size) {
			return i + 1;
		}
	}

	return IVM_FALSE;
}

void
ivm_heap_reset(ivm_heap_t *heap)
{
	MEM_FREE(heap->curs);
	heap->curs = MEM_ALLOC_INIT(sizeof(*heap->curs) * heap->bcount);
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

	printf("compact to %ld\n", bcount);

	heap->bcount = bcount;
	heap->curs = MEM_REALLOC(heap->curs, sizeof(*heap->curs) * bcount);
	heap->blocks = MEM_REALLOC(heap->blocks, sizeof(*heap->blocks) * bcount);

	return;
}
