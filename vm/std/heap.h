#ifndef _IVM_VM_GC_HEAP_H_
#define _IVM_VM_GC_HEAP_H_

#include "pub/mem.h"
#include "pub/com.h"
#include "pub/type.h"
#include "pub/err.h"

IVM_COM_HEADER

typedef struct ivm_heap_t_tag {
	ivm_size_t bcount;
	ivm_size_t bsize;
	ivm_size_t btop;
	ivm_size_t *curs;
	ivm_byte_t **blocks;
} ivm_heap_t;

#define IVM_HEAP_GET_BLOCK_SIZE(heap) ((heap)->bsize)
#define IVM_HEAP_SET_BLOCK_SIZE(heap, val) ((heap)->bsize = (val))
#define IVM_HEAP_GET_BLOCK_COUNT(heap) ((heap)->bcount)
#define IVM_HEAP_GET_CUR_SIZE(heap) ((heap)->curs)

#define IVM_HEAP_GET(obj, member) IVM_GET((obj), IVM_HEAP, member)
#define IVM_HEAP_SET(obj, member, val) IVM_SET((obj), IVM_HEAP, member, (val))

ivm_heap_t *
ivm_heap_new(ivm_size_t bsize);

void
ivm_heap_free(ivm_heap_t *heap);

#define OFFSET(ptr, size) (&(ptr)[size])
#define HAS_SIZE ivm_heap_hasSize
#define INC_SIZE(heap, idx, s) ((heap)->curs[idx] += (s))

IVM_INLINE
ivm_size_t
_ivm_heap_addBlock(ivm_heap_t *heap)
{
	heap->curs = MEM_REALLOC(heap->curs,
							 sizeof(*heap->curs) * ++heap->bcount,
							 ivm_size_t *);
	heap->blocks = MEM_REALLOC(heap->blocks,
							   sizeof(*heap->blocks) * heap->bcount,
							   ivm_byte_t **);

	heap->curs[heap->bcount - 1] = 0;
	heap->blocks[heap->bcount - 1] = MEM_ALLOC(heap->bsize,
											   ivm_byte_t *);

	return heap->bcount;
}

IVM_INLINE
void *
_ivm_heap_allocAt(ivm_heap_t *heap, ivm_size_t idx, ivm_size_t size)
{
	void *ret = OFFSET(heap->blocks[idx], heap->curs[idx]);

	INC_SIZE(heap, idx, size);

	return ret;
}

IVM_INLINE
ivm_size_t
ivm_heap_hasSize(ivm_heap_t *heap, ivm_size_t size)
{
	ivm_size_t i;

	for (i = heap->btop; i < heap->bcount; i++) {
		if (heap->bsize - heap->curs[i] >= size) {
			return i + 1;
		}
	}

	return IVM_FALSE;
}

IVM_INLINE
void *
ivm_heap_alloc_c(ivm_heap_t *heap, ivm_size_t size, ivm_bool_t *add_block)
{
	ivm_size_t i;

	IVM_ASSERT(size <= heap->bsize,
			   IVM_ERROR_MSG_SIZE_EXCEEDS_BLOCK_SIZE);

	if (!(i = HAS_SIZE(heap, size))) {
		i = _ivm_heap_addBlock(heap);
		if (add_block)
			*add_block = IVM_TRUE;
	}

	return _ivm_heap_allocAt(heap, i - 1, size);
}

#undef OFFSET
#undef HAS_SIZE
#undef INC_SIZE

#define ivm_heap_alloc(heap, size) (ivm_heap_alloc_c((heap), (size), IVM_NULL))

void *
ivm_heap_addCopy(ivm_heap_t *heap, void *ptr, ivm_size_t size);

void
ivm_heap_reset(ivm_heap_t *heap);

void
ivm_heap_compact(ivm_heap_t *heap);

IVM_COM_END

#endif
