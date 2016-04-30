#include "pub/mem.h"
#include "vm.h"
#include "err.h"
#include "obj.h"
#include "coro.h"
#include "gc/heap.h"
#include "gc/gc.h"

#define GC(state) ((state)->gc)
#define HEAP(state) ((state)->heap)

static
ivm_type_t static_type_list[] = {
	{ IVM_NULL_T, sizeof(ivm_object_t), IVM_NULL, IVM_NULL },
	{ IVM_OBJECT_T, sizeof(ivm_object_t), IVM_NULL, IVM_NULL },
	{ IVM_NUMERIC_T, sizeof(ivm_numeric_t), IVM_NULL, IVM_NULL }
};

ivm_vmstate_t *
ivm_vmstate_new()
{
	ivm_vmstate_t *ret = MEM_ALLOC_INIT(sizeof(*ret));
	ivm_type_t *tmp_type;
	ivm_int_t i, type_count = sizeof(static_type_list) / sizeof(ivm_type_t);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("vm state"));
	
	ret->heap = ivm_heap_new(IVM_DEFAULT_INIT_HEAP_SIZE);

	ret->cur_coro = 0;
	ret->coro_list = ivm_coro_list_new();
	
	ret->exec_list = ivm_exec_list_new();
	ret->type_list = ivm_type_list_new();

	for (i = 0; i < type_count; i++) {
		tmp_type = ivm_type_new(static_type_list[i]);
		ivm_type_list_register(ret->type_list, tmp_type);
	}

	/* ret->gc_flag = IVM_FALSE; */
	ret->gc = ivm_collector_new(ret);

	return ret;
}

void
ivm_vmstate_free(ivm_vmstate_t *state)
{
	if (state) {
		ivm_collector_free(GC(state));

		ivm_heap_free(HEAP(state));

		ivm_coro_list_free(state->coro_list);
		ivm_exec_list_free(state->exec_list);

		ivm_type_list_foreach(state->type_list, ivm_type_free);
		ivm_type_list_free(state->type_list);

		MEM_FREE(state);
	}

	return;
}

#if 0

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

#endif

void *
ivm_vmstate_alloc(ivm_vmstate_t *state, ivm_size_t size)
{
	if (!ivm_heap_hasSize(HEAP(state), size)) {
		ivm_collector_collect(GC(state), state, HEAP(state));
	}

	return ivm_heap_alloc(HEAP(state), size);
}

#define IS_AVAILABLE(state, i) (ivm_coro_isAsleep(ivm_coro_list_at((state)->coro_list, (i))))

static ivm_bool_t
ivm_vmstate_wrapCoro(ivm_vmstate_t *state)
{
	ivm_size_t i;
	ivm_bool_t ret = IVM_FALSE,
			   restart = IVM_FALSE;

	for (i = state->cur_coro + 1;
		 i != state->cur_coro + 1 || !restart; i++) {
		if (i >= ivm_coro_list_size(state->coro_list)) {
			i = 0;
			restart = IVM_TRUE;
		}
		if (IS_AVAILABLE(state, i)) {
			ret = IVM_TRUE; /* found available coroutine */
			state->cur_coro = i;
			break;
		}
	}

	return ret;
}

void
ivm_vmstate_schedule(ivm_vmstate_t *state)
{
	while (ivm_coro_list_size(state->coro_list) > 0) {
		ivm_coro_resume(ivm_coro_list_at(state->coro_list,
										 state->cur_coro), state);
		if (!ivm_vmstate_wrapCoro(state))
			break;
	}
	return;
}
