#include "pub/mem.h"
#include "heap.h"
#include "type.h"
#include "obj.h"
#include "err.h"
#include "vm.h"

#define CELL_SET_OBJ(c, o) ((c)->obj = (o))

#pragma GCC diagnostic ignored "-Wpedantic"

ivm_heap_t *
ivm_new_heap(ivm_size_t obj_count)
{
	void *pre = IVM_NULL;
	void *pre_cell = IVM_NULL;
	ivm_heap_t *ret = MEM_ALLOC(sizeof(*ret));
	ivm_size_t i;
	ivm_cell_t *tmp;

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("heap"));

	ret->pre = pre = MEM_ALLOC_INIT((sizeof(ivm_object_t) + sizeof(ivm_cell_t)) * obj_count);
	IVM_ASSERT(pre, IVM_ERROR_MSG_FAILED_ALLOC_NEW("prealloc memory in heap"));

	ret->empty = ivm_new_cell_set();
	IVM_ASSERT(ret->empty,
			   IVM_ERROR_MSG_FAILED_ALLOC_NEW("empty set in heap"));

	/* preallocated cell start address */
	pre_cell = pre + (sizeof(ivm_object_t) * obj_count);

	for (i = 0; i < obj_count; i++) {
		tmp = (ivm_cell_t *)(pre_cell + (i * sizeof(ivm_cell_t)));
		CELL_SET_OBJ(tmp, pre + (i * sizeof(ivm_object_t)));
		ivm_cell_set_add_cell(ret->empty, tmp);
	}
	ret->size = ret->empty_size = obj_count;

	return ret;
}

void
ivm_free_heap(ivm_heap_t *heap)
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
		cell = ivm_cell_set_remove_tail(heap->empty);
		IVM_ASSERT(cell, IVM_ERROR_MSG_NO_SPARE_MEM);
		ret = cell->obj;
		heap->empty_size--;
	}

	return ret;
}

ivm_object_t *
ivm_heap_new_object(ivm_vmstate_t *state, ivm_heap_t *heap)
{
	ivm_object_t *ret = ivm_heap_alloc(heap);

	if (ret) {
		ivm_init_object(state, ret);
	}

	return ret;
}

void
ivm_heap_free_object(ivm_vmstate_t *state,
					 ivm_heap_t *heap,
					 ivm_object_t *obj)
{
	ivm_cell_t *cell;

	if (heap) {
		IVM_ASSERT(heap->empty_size < heap->size, IVM_ERROR_MSG_WRONG_FREE_HEAP);
		cell = (ivm_cell_t *)(heap->pre + (sizeof(ivm_object_t) * heap->size)
							  + (sizeof(ivm_cell_t) * heap->empty_size));
		ivm_dump_object(state, obj);
		CELL_SET_OBJ(cell, obj);
		ivm_cell_set_add_cell(heap->empty, cell);
	}

	return;
}
