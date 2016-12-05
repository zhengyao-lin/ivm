#include "pub/com.h"
#include "pub/err.h"
#include "pub/vm.h"
#include "pub/type.h"

#include "mem.h"
#include "heap.h"

ivm_heap_t *
ivm_heap_new(ivm_size_t bsize)
{
	ivm_heap_t *ret = STD_ALLOC(sizeof(*ret));

	IVM_MEMCHECK(ret);

	ret->bcount = 2;
	ret->bsize = bsize;

	ret->btop = 0;
	ret->blocks = STD_ALLOC(sizeof(*ret->blocks) * 2);
	
	IVM_MEMCHECK(ret->blocks);

	ret->bcurp = ret->blocks[0] = STD_ALLOC(bsize);
	ret->blocks[1] = STD_ALLOC(bsize);

	IVM_MEMCHECK(ret->bcurp && ret->blocks[1]);

	ret->bendp = ret->bcurp + bsize;

	return ret;
}

void
ivm_heap_free(ivm_heap_t *heap)
{
	ivm_size_t i;
	if (heap) {
		for (i = 0; i < heap->bcount; i++)
			STD_FREE(heap->blocks[i]);
		STD_FREE(heap->blocks);
		STD_FREE(heap);
	}

	return;
}

void
ivm_heap_init(ivm_heap_t *heap,
			  ivm_size_t bsize)
{
	heap->bcount = 2;
	heap->bsize = bsize;

	heap->btop = 0;
	heap->blocks = STD_ALLOC(sizeof(*heap->blocks) * 2);
	
	IVM_MEMCHECK(heap->blocks);

	heap->bcurp = heap->blocks[0] = STD_ALLOC(bsize);
	heap->blocks[1] = STD_ALLOC(bsize);

	IVM_MEMCHECK(heap->bcurp && heap->blocks[1]);

	heap->bendp = heap->bcurp + bsize;

	return;
}

void
ivm_heap_dump(ivm_heap_t *heap)
{
	ivm_size_t i;

	if (heap) {
		for (i = 0; i < heap->bcount; i++)
			STD_FREE(heap->blocks[i]);
		STD_FREE(heap->blocks);
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

	if (heap->bcount <= IVM_DEFAULT_HEAP_MAX_COMPACT_BC)
		return;

	for (i = heap->blocks + heap->btop + 1,
		 end = heap->blocks + heap->bcount;
		 i != end; i++) {
		STD_FREE(*i);
	}

	heap->bcount = heap->btop + 1;
	heap->blocks = STD_REALLOC(
		heap->blocks,
		sizeof(*heap->blocks) * heap->bcount
	);

	return;
}

IVM_PRIVATE
IVM_INLINE
void *
_ivm_heap_addLargeBlock(ivm_heap_t *heap, ivm_size_t size)
{
	ivm_byte_t *cur_block;
	ivm_byte_t *orig = IVM_NULL;

	if (++heap->btop < heap->bcount) {
		orig = heap->blocks[heap->btop];
		cur_block = STD_ALLOC(size);
	} else {
		heap->btop = heap->bcount++;

		heap->blocks = STD_REALLOC(
			heap->blocks,
			sizeof(*heap->blocks) * heap->bcount
		);

		cur_block = STD_ALLOC(size);
	}

	if (!cur_block) {
		heap->btop--;
		return IVM_NULL;
	}

	heap->blocks[heap->btop] = cur_block;
	STD_FREE(orig);

	heap->bcurp
	= heap->bendp
	= cur_block + size;

	return cur_block;
}

void *
_ivm_heap_addBlock(ivm_heap_t *heap, ivm_size_t size)
{
	ivm_byte_t *cur_block;

	if (size <= heap->bsize) {
		if (++heap->btop < heap->bcount) {
			cur_block = heap->blocks[heap->btop];
		} else {
			heap->btop = heap->bcount++;

			heap->blocks = STD_REALLOC(
				heap->blocks,
				sizeof(*heap->blocks) * heap->bcount
			);

			// IVM_TRACE("%d %d\n", heap->btop, heap->bcount);

			cur_block
			= heap->blocks[heap->btop]
			= STD_ALLOC(heap->bsize);
		}

		heap->bcurp = cur_block + size;
		heap->bendp = cur_block + heap->bsize;

		return cur_block;
	}
	
	return _ivm_heap_addLargeBlock(heap, size);
}
