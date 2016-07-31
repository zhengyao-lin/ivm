#include "pub/mem.h"
#include "pub/err.h"
#include "pub/com.h"
#include "pub/inlines.h"

#include "std/heap.h"
#include "std/uid.h"
#include "std/string.h"

#include "gc/gc.h"
#include "vm.h"
#include "obj.h"
#include "coro.h"
#include "func.h"
#include "oprt.h"
#include "proto.h"

#define GC(state) ((state)->gc)

#include "type.req.h"

IVM_PRIVATE
ivm_type_t static_type_list[] = {
#define TYPE_GEN(t, n, s, proto_init, ...) \
	{                                      \
		.tag = t,                          \
		.name = #n,                        \
		.size = s,                         \
		__VA_ARGS__                        \
	},

	#include "type.def.h"

#undef TYPE_GEN
};

#define SET_TYPE_PROTO(state, tag, obj) \
	(ivm_type_setProto(ivm_vmstate_getType((state), (tag)), (obj)))

ivm_vmstate_t *
ivm_vmstate_new()
{
	ivm_vmstate_t *ret = MEM_ALLOC(sizeof(*ret),
								   ivm_vmstate_t *);
	ivm_type_t *tmp_type, *end;
	ivm_int_t i;

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("vm state"));

	for (i = 0; i < IVM_ARRLEN(ret->heaps); i++) {
		ivm_heap_init(ret->heaps + i, IVM_DEFAULT_INIT_HEAP_SIZE);
	}

	ret->cur_coro = 0;
	ivm_coro_list_init(&ret->coro_list);
	
	// ivm_type_list_init(&ret->type_list);
	ivm_func_list_init(&ret->func_list);

	ret->func_pool = ivm_function_pool_new(IVM_DEFAULT_FUNCTION_POOL_SIZE);
	ret->ct_pool = ivm_context_pool_new(IVM_DEFAULT_CONTEXT_POOL_SIZE);
	ivm_coro_pool_init(&ret->cr_pool, IVM_DEFAULT_CORO_POOL_SIZE);

	ret->const_pool
	= ivm_string_pool_new(IVM_FALSE);
	ivm_ref_inc(ret->const_pool);

#define CONST_GEN(name, str) \
	ret->const_str_##name = (const ivm_string_t *)ivm_string_pool_registerRaw(ret->const_pool, (str));
	#include "vm.const.h"
#undef CONST_GEN

	ret->gc_flag = IVM_FALSE;
	ret->gc = ivm_collector_new(ret);

	ivm_vmstate_lockGCFlag(ret);

	for (i = 0, tmp_type = ret->type_list, end = tmp_type + IVM_TYPE_COUNT;
		 tmp_type != end; tmp_type++, i++) {
		ivm_type_init(tmp_type, static_type_list + i);
	}

	for (tmp_type = ret->type_list, end = tmp_type + IVM_TYPE_COUNT;
		 tmp_type != end; tmp_type++) {
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
	ivm_type_t *i, *end;
	ivm_int_t j;

	if (state) {
		ivm_collector_free(GC(state), state);

		for (j = 0; j < IVM_ARRLEN(state->heaps); j++) {
			ivm_heap_dump(state->heaps + j);
		}

		IVM_CORO_LIST_EACHPTR(&state->coro_list, citer) {
			ivm_coro_free(IVM_CORO_LIST_ITER_GET(citer), state);
		}
		ivm_coro_list_dump(&state->coro_list);

		ivm_func_list_dump(&state->func_list, state);

		ivm_function_pool_free(state->func_pool);
		ivm_context_pool_free(state->ct_pool);
		ivm_coro_pool_destruct(&state->cr_pool);

		ivm_string_pool_free(state->const_pool);

		for (i = state->type_list, end = i + IVM_TYPE_COUNT;
			 i != end; i++) {
			ivm_type_dump(i);
		}

		MEM_FREE(state);
	}

	return;
}

void
ivm_vmstate_reinit(ivm_vmstate_t *state)
{
	ivm_coro_list_iterator_t citer;
	ivm_int_t i;

	for (i = 0; i < IVM_ARRLEN(state->heaps); i++) {
		ivm_heap_reset(state->heaps + i);
	}

	state->cur_coro = 0;
	IVM_CORO_LIST_EACHPTR(&state->coro_list, citer) {
		ivm_coro_free(IVM_CORO_LIST_ITER_GET(citer), state);
	}
	ivm_coro_list_empty(&state->coro_list);

	ivm_func_list_empty(&state->func_list, state);

	ivm_function_pool_dumpAll(state->func_pool);
	ivm_context_pool_dumpAll(state->ct_pool);
	ivm_coro_pool_dumpAll(&state->cr_pool);

	ivm_collector_reinit(state->gc);

	return;
}

ivm_size_t
ivm_vmstate_addCoro(ivm_vmstate_t *state,
					ivm_function_object_t *func)
{
	ivm_coro_t *coro = ivm_coro_new(state);
	ivm_coro_setRoot(coro, state, func);
	return ivm_coro_list_add(&state->coro_list, coro);
}

IVM_PRIVATE
IVM_INLINE
ivm_bool_t
_ivm_vmstate_switchCoro(ivm_vmstate_t *state)
{
	ivm_coro_list_t *list = &state->coro_list;
	ivm_coro_list_iterator_t i, end;

	// if (!ivm_coro_list_size(list)) return IVM_FALSE;

	for (i = IVM_CORO_LIST_ITER_AT(list, state->cur_coro + 1),
		 end = IVM_CORO_LIST_ITER_END(list);
		 i != end; i++) {
		if (ivm_coro_isAlive(IVM_CORO_LIST_ITER_GET(i))) {
			state->cur_coro = IVM_CORO_LIST_ITER_INDEX(list, i);
			return IVM_TRUE;
		}
	}

	for (end = i,
		 i = IVM_CORO_LIST_ITER_BEGIN(list);
		 i != end; i++) {
		if (ivm_coro_isAlive(IVM_CORO_LIST_ITER_GET(i))) {
			state->cur_coro = IVM_CORO_LIST_ITER_INDEX(list, i);
			return IVM_TRUE;
		}
	}

	return IVM_FALSE;
}

void
ivm_vmstate_schedule(ivm_vmstate_t *state)
{
	ivm_object_t *ret = IVM_NULL;
	ivm_coro_list_t *coros = &state->coro_list;

	while (ivm_coro_list_size(coros)) {
		ret = ivm_coro_resume(ivm_coro_list_at(coros, state->cur_coro),
							  state, ret);
		if (!_ivm_vmstate_switchCoro(state))
			break;
	}

	state->cur_coro = 0;

	return;
}

/* schedule one round */
ivm_object_t *
ivm_vmstate_schedule_r(ivm_vmstate_t *state,
					   ivm_object_t *ret)
{
	ivm_coro_list_t *coros = &state->coro_list;
	ivm_size_t skip = state->cur_coro;

	while (ivm_coro_list_size(coros)) {
		if (!_ivm_vmstate_switchCoro(state)) {
			IVM_FATAL(IVM_ERROR_MSG_NO_ALIVE_CORO_TO_SCHEDULE);
		}
		if (state->cur_coro == skip) break;
		ret = ivm_coro_resume(ivm_coro_list_at(coros, state->cur_coro),
							  state, ret);
	}

	return ret;
}
