#include "pub/mem.h"
#include "vm.h"
#include "err.h"
#include "gc/heap.h"

ivm_vmstate_t *
ivm_vmstate_new()
{
	ivm_vmstate_t *ret = MEM_ALLOC_INIT(sizeof(*ret));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("vm state"));
	
	ret->gc_flag = IVM_FALSE;
	ret->heap_count = 0;
	ret->heaps = IVM_NULL;

	ret->coro_count = 0;
	ret->cur_coro = 0;
	ret->coros = IVM_NULL;
	
	return ret;
}

void
ivm_vmstate_free(ivm_vmstate_t *state)
{
	ivm_size_t i;

#if IVM_CHECK_STATE_NULL
	if (state) {
#endif
	
	for (i = 0; i < state->heap_count; i++) {
		ivm_heap_free(state->heaps[i]);
	}
	MEM_FREE(state->heaps);
	MEM_FREE(state);

#if IVM_CHECK_STATE_NULL
	}
#endif

	return;
}

void
ivm_vmstate_addHeap(ivm_vmstate_t *state)
{
#if IVM_CHECK_STATE_NULL
	if (state) {
#endif

	state->heaps = MEM_REALLOC(state->heaps,
							  sizeof(ivm_heap_t *)
							  * ++state->heap_count);
	
	state->heaps[state->heap_count - 1]
	= ivm_heap_new(IVM_DEFAULT_INIT_HEAP_SIZE);

#if IVM_CHECK_STATE_NULL
	}
#endif

	return;
}

ivm_heap_t *
ivm_vmstate_findHeap(ivm_vmstate_t *state,
					 ivm_object_t *obj)
{
	ivm_heap_t *ret = IVM_NULL;
	ivm_size_t i;

#if IVM_CHECK_STATE_NULL
	if (state) {
#endif

	for (i = 0; i < state->heap_count; i++) {
		if (ivm_heap_isInHeap(state->heaps[i], obj)) {
			ret = state->heaps[i];
		}
	}

#if IVM_CHECK_STATE_NULL
	}
#endif

	return ret;
}

ivm_object_t *
ivm_vmstate_alloc(ivm_vmstate_t *state)
{
	ivm_object_t *ret = IVM_NULL;

#if IVM_CHECK_STATE_NULL
	if (state) {
#endif

	ret = ivm_heap_alloc(ivm_vmstate_curHeap(state));
	if (!ret) {
		ivm_vmstate_addHeap(state);
		ivm_vmstate_markForGC(state);
	}
	ret = ivm_heap_alloc(ivm_vmstate_curHeap(state));
	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("object in heap"));

#if IVM_CHECK_STATE_NULL
	}
#endif

	return ret;
}

ivm_object_t *
ivm_vmstate_newObject(ivm_vmstate_t *state)
{
	ivm_object_t *ret = IVM_NULL;

#if IVM_CHECK_STATE_NULL
	if (state) {
#endif

	ret = ivm_vmstate_alloc(state);
	ivm_object_init(state, ret);

#if IVM_CHECK_STATE_NULL
	}
#endif

	return ret;
}

void
ivm_vmstate_freeObject(ivm_vmstate_t *state, ivm_object_t *obj)
{
	ivm_heap_t *heap;

#if IVM_CHECK_STATE_NULL
	if (state) {
#endif

	heap = ivm_vmstate_findHeap(state, obj);
	IVM_ASSERT(heap, IVM_ERROR_MSG_CANNOT_FIND_OBJECT_IN_HEAP);
	ivm_heap_freeObject(heap, state, obj);

#if IVM_CHECK_STATE_NULL
	}
#endif

	return;
}
