#include "pub/mem.h"
#include "vm.h"
#include "err.h"
#include "obj.h"
#include "gc/heap.h"

ivm_vmstate_t *
ivm_vmstate_new()
{
	ivm_vmstate_t *ret = MEM_ALLOC_INIT(sizeof(*ret));
	ivm_type_t *tmp_type;

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("vm state"));
	
	ret->gc_flag = IVM_FALSE;
	ret->heap_count = 0;
	ret->heaps = IVM_NULL;

	ret->cur_coro = 0;
	ret->coro_list = ivm_coro_list_new();
	
	ret->exec_list = ivm_exec_list_new();
	ret->type_list = ivm_type_list_new();

	tmp_type = ivm_type_new(IVM_NULL_T, IVM_NULL, IVM_NULL);
	ivm_type_list_register(ret->type_list, tmp_type);
	
	tmp_type = ivm_type_new(IVM_OBJECT_T, IVM_NULL, IVM_NULL);
	ivm_type_list_register(ret->type_list, tmp_type);
	
	tmp_type = ivm_type_new(IVM_NUMERIC_T, IVM_NULL, IVM_NULL);
	ivm_type_list_register(ret->type_list, tmp_type);

	tmp_type = ivm_type_new(IVM_FUNCTION_T, IVM_NULL, IVM_NULL);
	ivm_type_list_register(ret->type_list, tmp_type);

	return ret;
}

void
ivm_vmstate_free(ivm_vmstate_t *state)
{
	ivm_size_t i;

	if (state) {
		for (i = 0; i < state->heap_count; i++) {
			ivm_heap_free(state->heaps[i]);
		}
		MEM_FREE(state->heaps);
		ivm_coro_list_free(state->coro_list);
		ivm_exec_list_free(state->exec_list);
		ivm_type_list_foreach(state->type_list, ivm_type_free);
		ivm_type_list_free(state->type_list);
		MEM_FREE(state);
	}

	return;
}

void
ivm_vmstate_addHeap(ivm_vmstate_t *state)
{
	state->heaps = MEM_REALLOC(state->heaps,
							  sizeof(ivm_heap_t *)
							  * ++state->heap_count);
	
	state->heaps[state->heap_count - 1]
	= ivm_heap_new(IVM_DEFAULT_INIT_HEAP_SIZE);

	return;
}

ivm_heap_t *
ivm_vmstate_findHeap(ivm_vmstate_t *state,
					 ivm_object_t *obj)
{
	ivm_heap_t *ret = IVM_NULL;
	ivm_size_t i;

	for (i = 0; i < state->heap_count; i++) {
		if (ivm_heap_isInHeap(state->heaps[i], obj)) {
			ret = state->heaps[i];
		}
	}

	return ret;
}

ivm_object_t *
ivm_vmstate_alloc(ivm_vmstate_t *state)
{
	ivm_object_t *ret = IVM_NULL;

	ret = ivm_heap_alloc(ivm_vmstate_curHeap(state));
	if (!ret) {
		ivm_vmstate_addHeap(state);
		ivm_vmstate_markForGC(state);

		ret = ivm_heap_alloc(ivm_vmstate_curHeap(state));
		IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("object in heap"));
	}

	return ret;
}

ivm_object_t *
ivm_vmstate_newObject(ivm_vmstate_t *state)
{
	ivm_object_t *ret = IVM_NULL;

	ret = ivm_vmstate_alloc(state);
	ivm_object_init(ret, state);

	return ret;
}

void
ivm_vmstate_freeObject(ivm_vmstate_t *state, ivm_object_t *obj)
{
	ivm_heap_t *heap;

	heap = ivm_vmstate_findHeap(state, obj);
	IVM_ASSERT(heap, IVM_ERROR_MSG_CANNOT_FIND_OBJECT_IN_HEAP);
	ivm_heap_freeObject(heap, state, obj);

	return;
}
