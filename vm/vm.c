#include "pub/mem.h"
#include "pub/com.h"
#include "vm.h"
#include "err.h"
#include "obj.h"
#include "coro.h"
#include "func.h"
#include "num.h"
#include "gc/heap.h"
#include "gc/gc.h"

#define GC(state) ((state)->gc)
#define HEAP1(state) ((state)->heaps[0])
#define HEAP2(state) ((state)->heaps[1])
#define HEAP_CUR HEAP1

IVM_PRIVATE
ivm_type_t static_type_list[] = {
	{
		IVM_UNDEFINED_T, "undefined",
			sizeof(ivm_object_t),
			IVM_NULL,
			IVM_NULL,
			ivm_object_alwaysFalse,
			IVM_NULL
	},

	{
		IVM_NULL_T, "null",
			sizeof(ivm_object_t),
			IVM_NULL,
			IVM_NULL,
			ivm_object_alwaysFalse,
			IVM_NULL
	},

	{
		IVM_OBJECT_T, "object",
			sizeof(ivm_object_t),
			IVM_NULL,
			IVM_NULL,
			ivm_object_isTrue,
			IVM_NULL
	},

	{
		IVM_NUMERIC_T, "numeric",
			sizeof(ivm_numeric_t),
			IVM_NULL,
			IVM_NULL,
			ivm_numeric_isTrue,
			IVM_NULL
	},

	{
		IVM_FUNCTION_OBJECT_T, "function",
			sizeof(ivm_function_object_t),
			ivm_function_object_destructor,
			ivm_function_object_traverser,
			ivm_object_isTrue,
			IVM_NULL
	}
};

ivm_vmstate_t *
ivm_vmstate_new()
{
	ivm_vmstate_t *ret = MEM_ALLOC(sizeof(*ret),
								   ivm_vmstate_t *);
	ivm_type_t *tmp_type;
	ivm_int_t i, type_count = sizeof(static_type_list) / sizeof(ivm_type_t);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("vm state"));
	
	ret->heaps[0] = ivm_heap_new(IVM_DEFAULT_INIT_HEAP_SIZE);
	ret->heaps[1] = ivm_heap_new(IVM_DEFAULT_INIT_HEAP_SIZE);

	ret->cur_coro = 0;
	ret->coro_list = ivm_coro_list_new();
	
	ret->exec_list = ivm_exec_list_new();
	ret->type_list = ivm_type_list_new();

	ret->func_pool
	= ivm_function_pool_new(IVM_DEFAULT_FUNCTION_POOL_SIZE);
	ret->ct_pool
	= ivm_context_pool_new(IVM_DEFAULT_CONTEXT_POOL_SIZE);
	ret->fr_pool
	= ivm_frame_pool_new(IVM_DEFAULT_FRAME_POOL_SIZE);

	for (i = 0; i < type_count; i++) {
		tmp_type = ivm_type_new(static_type_list[i]);
		ivm_type_list_register(ret->type_list, tmp_type);
	}

	ret->gc_flag = IVM_FALSE;
	ret->gc = ivm_collector_new(ret);

	return ret;
}

void
ivm_vmstate_free(ivm_vmstate_t *state)
{
	if (state) {
		ivm_collector_free(GC(state), state);

		ivm_heap_free(HEAP1(state));
		ivm_heap_free(HEAP2(state));

		ivm_coro_list_free(state->coro_list);
		ivm_exec_list_free(state->exec_list);

		ivm_function_pool_free(state->func_pool);
		ivm_context_pool_free(state->ct_pool);
		ivm_frame_pool_free(state->fr_pool);

		ivm_type_list_foreach(state->type_list, ivm_type_free);
		ivm_type_list_free(state->type_list);

		MEM_FREE(state);
	}

	return;
}

void *
ivm_vmstate_alloc(ivm_vmstate_t *state, ivm_size_t size)
{
	ivm_bool_t add_block;
	void *ret = ivm_heap_alloc_c(HEAP_CUR(state), size, &add_block);

	if (add_block) {
		ivm_vmstate_openGCFlag(state);
	}

	return ret;
}

void
ivm_vmstate_swapHeap(ivm_vmstate_t *state)
{
	ivm_heap_t *tmp = HEAP1(state);
	HEAP1(state) = HEAP2(state);
	HEAP2(state) = tmp;

	return;
}

#define IS_AVAILABLE(state, i) (ivm_coro_isAsleep(ivm_coro_list_at((state)->coro_list, (i))))

IVM_PRIVATE
ivm_bool_t
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
	ivm_object_t *ret = IVM_NULL;

	while (ivm_coro_list_size(state->coro_list) > 0) {
		ret = ivm_coro_resume(ivm_coro_list_at(state->coro_list,
											   state->cur_coro),
							  state, ret);
		if (!ivm_vmstate_wrapCoro(state))
			break;
	}
	return;
}
