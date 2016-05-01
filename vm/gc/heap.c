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
	printf("new block added\n");
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

/* old heap */
#if 0

#define CELL_SET_OBJ(c, o) ((c)->obj = (o))

#define OFFSET_AS(p, i, type) (((type *)p)[i])

/*
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wpointer-arith"
*/

ivm_heap_t *
ivm_heap_new(ivm_size_t obj_count)
{
	void *pre = IVM_NULL;
	void *pre_cell = IVM_NULL;
	ivm_heap_t *ret = MEM_ALLOC(sizeof(*ret));
	ivm_size_t i;
	ivm_cell_t *tmp;

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("heap"));

	ret->pre = pre = MEM_ALLOC_INIT((sizeof(ivm_object_t) + sizeof(ivm_cell_t)) * obj_count);
	IVM_ASSERT(pre, IVM_ERROR_MSG_FAILED_ALLOC_NEW("prealloc memory in heap"));

	ret->empty = ivm_cell_set_new();
	IVM_ASSERT(ret->empty,
			   IVM_ERROR_MSG_FAILED_ALLOC_NEW("empty set in heap"));

	/* preallocated cell start address */
	pre_cell = &OFFSET_AS(pre, obj_count, ivm_object_t);

	for (i = 0; i < obj_count; i++) {
		tmp = &(OFFSET_AS(pre_cell, i, ivm_cell_t));
		CELL_SET_OBJ(tmp, &OFFSET_AS(pre, i, ivm_object_t));
		ivm_cell_set_addCell(ret->empty, tmp);
	}
	ret->size = ret->empty_size = obj_count;

	return ret;
}

void
ivm_heap_free(ivm_heap_t *heap)
{
	if (heap) {
		MEM_FREE(heap->empty);
		MEM_FREE(heap->pre);
		MEM_FREE(heap);
	}

	return;
}

ivm_object_t *
ivm_heap_alloc(ivm_heap_t *heap)
{
	ivm_object_t *ret = IVM_NULL;
	ivm_cell_t *cell;

	if (heap && heap->empty->tail) {
		cell = ivm_cell_set_removeTail(heap->empty);
		/* IVM_ASSERT(cell, IVM_ERROR_MSG_NO_SPARE_MEM); */
		if (!cell) return ret;
		ret = cell->obj;
		heap->empty_size--;
	}

	return ret;
}

void
ivm_heap_freeObject(ivm_heap_t *heap,
					ivm_vmstate_t *state,
					ivm_object_t *obj)
{
	ivm_cell_t *cell;
	void *pre_cell;

	if (heap) {
		IVM_ASSERT(heap->empty_size < heap->size, IVM_ERROR_MSG_WRONG_FREE_HEAP);
		
		pre_cell = &OFFSET_AS(heap->pre, heap->size, ivm_object_t);
		cell = &OFFSET_AS(pre_cell, heap->empty_size, ivm_cell_t);

		ivm_object_dump(obj, state);
		ivm_cell_init(cell, obj);

		ivm_cell_set_addCell(heap->empty, cell);
	}

	return;
}

ivm_bool_t
ivm_heap_isInHeap(ivm_heap_t *heap, ivm_object_t *obj)
{
	if (heap) {
		return heap->pre <= (void *)obj
			   && (void *)obj <= (void *)(&OFFSET_AS(heap->pre,
			   										 heap->size - 1,
			   										 ivm_object_t));
	}

	return IVM_FALSE;
}

#endif
