#include "pub/mem.h"
#include "pub/err.h"
#include "pub/com.h"
#include "pub/inlines.h"

#include "std/heap.h"
#include "std/uid.h"

#include "gc/gc.h"
#include "vm.h"
#include "obj.h"
#include "coro.h"
#include "func.h"
#include "num.h"
#include "strobj.h"
#include "oprt.h"
#include "proto.h"

#define GC(state) ((state)->gc)
#define HEAP1(state) ((state)->cur_heap)
#define HEAP2(state) ((state)->empty_heap)
#define HEAP_CUR HEAP1

IVM_PRIVATE
ivm_type_t static_type_list[] = {
	{
		IVM_UNDEFINED_T, "undefined", sizeof(ivm_object_t),

		.const_bool = IVM_FALSE,
		.to_bool = ivm_object_alwaysFalse
	},

	{
		IVM_NULL_T, "null", sizeof(ivm_object_t),

		.const_bool = IVM_FALSE,
		.to_bool = ivm_object_alwaysFalse
	},

	{
		IVM_OBJECT_T, "object", sizeof(ivm_object_t),

		.const_bool = IVM_TRUE,
	},

	{
		IVM_NUMERIC_T, "numeric", sizeof(ivm_numeric_t),

		.const_bool = IVM_TRUE,
		.to_bool = ivm_numeric_isTrue
	},

	{
		IVM_STRING_OBJECT_T, "string", sizeof(ivm_string_object_t),

		.trav = ivm_string_object_traverser,
		.const_bool = IVM_TRUE
	},

	{
		IVM_FUNCTION_OBJECT_T, "function", sizeof(ivm_function_object_t),

		.des = ivm_function_object_destructor,
		.trav = ivm_function_object_traverser,
		.const_bool = IVM_TRUE
	}
};

#define SET_TYPE_PROTO(state, tag, obj) \
	(ivm_type_setProto(ivm_vmstate_getType((state), (tag)), (obj)))

ivm_vmstate_t *
ivm_vmstate_new()
{
	ivm_vmstate_t *ret = MEM_ALLOC(sizeof(*ret),
								   ivm_vmstate_t *);
	ivm_type_t *tmp_type;
	ivm_int_t i, type_count = sizeof(static_type_list) / sizeof(ivm_type_t);
	ivm_type_list_iterator_t titer;

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("vm state"));
	
	ret->cur_heap = ivm_heap_new(IVM_DEFAULT_INIT_HEAP_SIZE);
	ret->empty_heap = ivm_heap_new(IVM_DEFAULT_INIT_HEAP_SIZE);

	ret->cur_coro = 0;
	ret->coro_list = ivm_coro_list_new();
	
	ret->type_list = ivm_type_list_new();
	ret->func_list = ivm_func_list_new();

	ret->func_pool
	= ivm_function_pool_new(IVM_DEFAULT_FUNCTION_POOL_SIZE);
	ret->ct_pool
	= ivm_context_pool_new(IVM_DEFAULT_CONTEXT_POOL_SIZE);
	ret->cr_pool
	= ivm_coro_pool_new(IVM_DEFAULT_CORO_POOL_SIZE);

	ret->const_pool
	= ivm_string_pool_new(IVM_FALSE);
	ivm_ref_inc(ret->const_pool);

	ret->gc_flag = IVM_FALSE;
	ret->gc = ivm_collector_new(ret);

	ivm_vmstate_lockGCFlag(ret);

	for (i = 0; i < type_count; i++) {
		tmp_type = ivm_type_new(static_type_list[i]);
		ivm_type_list_register(ret->type_list, tmp_type);
	}

	IVM_TYPE_LIST_EACHPTR(ret->type_list, titer) {
		tmp_type = IVM_TYPE_LIST_ITER_GET(titer);
		ivm_proto_initType(tmp_type, ret);
	}

	ivm_oprt_initType(ret);

	ivm_vmstate_unlockGCFlag(ret);

	ivm_uid_gen_init(&ret->uid_gen);

	return ret;
}

void
ivm_vmstate_free(ivm_vmstate_t *state)
{
	ivm_coro_list_iterator_t citer;

	if (state) {
		ivm_collector_free(GC(state), state);

		ivm_heap_free(HEAP1(state));
		ivm_heap_free(HEAP2(state));


		IVM_CORO_LIST_EACHPTR(state->coro_list, citer) {
			ivm_coro_free(IVM_CORO_LIST_ITER_GET(citer), state);
		}
		ivm_coro_list_free(state->coro_list);

		ivm_func_list_free(state->func_list);

		ivm_function_pool_free(state->func_pool);
		ivm_context_pool_free(state->ct_pool);
		ivm_coro_pool_free(state->cr_pool);

		ivm_string_pool_free(state->const_pool);

		ivm_type_list_foreach(state->type_list, ivm_type_free);
		ivm_type_list_free(state->type_list);

		MEM_FREE(state);
	}

	return;
}

IVM_PRIVATE
ivm_bool_t
ivm_vmstate_wrapCoro(ivm_vmstate_t *state)
{
	ivm_coro_list_t *list = state->coro_list;
	ivm_coro_list_iterator_t i, end;

	// if (!ivm_coro_list_size(list)) return IVM_FALSE;

	for (i = IVM_CORO_LIST_ITER_AT(list, state->cur_coro + 1),
		 end = IVM_CORO_LIST_ITER_END(list);
		 i != end; i++) {
		if (ivm_coro_isAsleep(IVM_CORO_LIST_ITER_GET(i))) {
			state->cur_coro = IVM_CORO_LIST_ITER_INDEX(list, i);
			return IVM_TRUE;
		}
	}

	for (end = i,
		 i = IVM_CORO_LIST_ITER_BEGIN(list);
		 i != end; i++) {
		if (ivm_coro_isAsleep(IVM_CORO_LIST_ITER_GET(i))) {
			state->cur_coro = IVM_CORO_LIST_ITER_INDEX(list, i);
			return IVM_TRUE;
		}
	}

	return IVM_FALSE;
}

ivm_size_t
ivm_vmstate_addCoro(ivm_vmstate_t *state,
					ivm_function_object_t *func)
{
	ivm_coro_t *coro = ivm_coro_new(state);
	ivm_coro_setRoot(coro, state, func);
	return ivm_coro_list_add(state->coro_list, coro);
}

void
ivm_vmstate_schedule(ivm_vmstate_t *state)
{
	ivm_object_t *ret = IVM_NULL;
	ivm_coro_list_t *coros = state->coro_list;

	while (ivm_coro_list_size(coros)) {
		ret = ivm_coro_resume(ivm_coro_list_at(coros, state->cur_coro),
							  state, ret);
		if (!ivm_vmstate_wrapCoro(state))
			break;
	}

	state->cur_coro = 0;

	return;
}
