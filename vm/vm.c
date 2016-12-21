#include "std/mem.h"
#include "pub/err.h"
#include "pub/com.h"
#include "pub/inlines.h"

#include "std/heap.h"
#include "std/uid.h"
#include "std/string.h"
#include "std/time.h"
#include "std/thread.h"

#include "gc/gc.h"

#include "vm.h"
#include "obj.h"
#include "coro.h"
#include "func.h"
#include "oprt.h"
#include "proto.h"

#include "type.req.h"

#define SET_TYPE_PROTO(state, tag, obj) \
	(ivm_type_setProto(ivm_vmstate_getType((state), (tag)), (obj)))

ivm_vmstate_t *
ivm_vmstate_new(ivm_string_pool_t *const_pool)
{
	ivm_vmstate_t *ret = STD_ALLOC(sizeof(*ret));
	ivm_type_t *tmp_type, *end;
	// ivm_cgroup_t *tmp_group;
	ivm_int_t i;

	IVM_MEMCHECK(ret);

	for (i = 0; i < IVM_ARRLEN(ret->heaps); i++) {
		ivm_heap_init(ret->heaps + i, IVM_DEFAULT_INIT_HEAP_SIZE);
	}

	// ret->cur_cgroup = 0;
	// ivm_cgroup_list_init(&ret->coro_groups);
	// ivm_cgroup_list_prepush(&ret->coro_groups, &tmp_group);
	// ivm_cgroup_init(tmp_group);
	// ret->last_gid = 1;

	ivm_type_pool_init(&ret->type_pool);
	ivm_func_list_init(&ret->func_list);

#if IVM_USE_BLOCK_POOL
	ivm_block_pool_init(&ret->block_pool);
#endif

	ret->func_pool = ivm_function_pool_new(IVM_DEFAULT_FUNCTION_POOL_SIZE);

	ivm_context_pool_init(&ret->ct_pool, IVM_DEFAULT_CONTEXT_POOL_SIZE);
	ivm_coro_pool_init(&ret->cr_pool, IVM_DEFAULT_CORO_POOL_SIZE);

	ret->cur_coro = IVM_NULL;
	ret->main_coro = IVM_NULL;

	ret->const_pool = const_pool;
	ivm_ref_inc(const_pool);

#define CONST_GEN(name, str) \
	ret->const_str_##name = (const ivm_string_t *)ivm_string_pool_registerRaw(const_pool, (str));
	#include "vm.const.h"
#undef CONST_GEN

	ret->wild_size = 0;

	ret->gc_flag = IVM_FALSE;
	ivm_collector_init(&ret->gc);

	ret->thread_init = IVM_FALSE;
	ret->int_next = ret->int_head = 0;

#if IVM_USE_MULTITHREAD
	ivm_thread_mutex_init(&ret->int_lock);
#endif

	ivm_vmstate_lockGCFlag(ret);

	ret->except = IVM_NULL;
	ivm_uid_gen_init(&ret->uid_gen);

#define _STATE ret
	
	ivm_type_t static_type_list[] = {
	#define TYPE_GEN(t, n, s, c, proto_init, ...) \
		((ivm_type_t) {                          \
			.tag = (t),                          \
			.name = #n,                          \
			.size = (s),                         \
			.cons = (c),                         \
			.uid = (ivm_int_t *)(ivm_ptr_t)(t),  \
			.is_builtin = IVM_TRUE,              \
			__VA_ARGS__                          \
		}),

		#include "type.def.h"

	#undef TYPE_GEN
	};

	for (i = 0, tmp_type = ret->type_list, end = tmp_type + IVM_TYPE_COUNT;
		 tmp_type != end; tmp_type++, i++) {
		ivm_type_init(tmp_type, static_type_list + i);
	}

	for (tmp_type = ret->type_list, end = tmp_type + IVM_TYPE_COUNT;
		 tmp_type != end; tmp_type++) {
		ivm_proto_initType(tmp_type, ret);
	}

	ivm_oprt_initType(ret);

	ret->loaded_mod = ivm_object_new(ret);
	// ret->tp_type = ivm_object_new(ret);
	ret->obj_none = ivm_none_new(ret);

	ivm_vmstate_unlockGCFlag(ret);

	ret->cur_path = IVM_NULL;
	// ret->coro_list_uid = ivm_uid_gen_nextPtr(ret->uid_gen);

	return ret;
}

void
ivm_vmstate_free(ivm_vmstate_t *state)
{
	// ivm_cgroup_list_iterator_t giter;

	ivm_type_t *i, *end;
	ivm_int_t j;

	if (state) {
		ivm_vmstate_cleanThread(state);

		ivm_coro_free(state->main_coro, state);

		ivm_collector_dump(&state->gc, state);

		for (j = 0; j < IVM_ARRLEN(state->heaps); j++) {
			ivm_heap_dump(state->heaps + j);
		}

		/*
		IVM_CGROUP_LIST_EACHPTR(&state->coro_groups, giter) {
			ivm_cgroup_dump(IVM_CGROUP_LIST_ITER_GET_PTR(giter), state);
		}
		*/
		// ivm_cgroup_list_dump(&state->coro_groups);

		ivm_type_pool_dump(&state->type_pool);

		ivm_func_list_dump(&state->func_list, state);
#if IVM_USE_BLOCK_POOL
		ivm_block_pool_dump(&state->block_pool);
#endif

		if (state->thread_init) {
			ivm_coro_set_dump(&state->thread_pool);
		}

		ivm_function_pool_free(state->func_pool);
		ivm_context_pool_destruct(&state->ct_pool);
		ivm_coro_pool_destruct(&state->cr_pool);

		ivm_string_pool_free(state->const_pool);

		for (i = state->type_list, end = i + IVM_TYPE_COUNT;
			 i != end; i++) {
			ivm_type_dump(i);
		}

		STD_FREE(state->cur_path);

		STD_FREE(state);
	}

	return;
}

#if IVM_USE_MULTITHREAD

IVM_PRIVATE
void *
_thread_clock(void *arg)
{
	ivm_vmstate_t *state = (ivm_vmstate_t *)arg;

	while (1) {
		ivm_time_msleep(10);
		// IVM_TRACE("########signal!\n");
		ivm_vmstate_unsetCSL(state);
		ivm_vmstate_setInt(state, IVM_CORO_INT_THREAD_YIELD);
		while (!ivm_vmstate_getCSL(state)) {
			// IVM_TRACE("wait for csl\n");
			ivm_thread_cancelPoint();
		}
	}

	return IVM_NULL;
}

#endif

void
ivm_vmstate_initThread(ivm_vmstate_t *state)
{
	if (!state->thread_init) {
		ivm_coro_set_init(&state->thread_pool);
		state->thread_init = IVM_TRUE;

#if IVM_USE_MULTITHREAD
		ivm_thread_mutex_init(&state->thread_gil);
		ivm_thread_mutex_init(&state->thread_csl_prot);
		state->thread_csl = IVM_FALSE;
		// ivm_thread_enableThread(state);

		ivm_thread_init(&state->thread_clock, _thread_clock, (void *)state);
#endif
	}

	return;
}

void
ivm_vmstate_cleanThread(ivm_vmstate_t *state)
{
#if IVM_USE_MULTITHREAD
	if (state->thread_init) {
		ivm_thread_cancel(&state->thread_clock);
		// IVM_TRACE("wait!!\n");
		ivm_thread_wait(&state->thread_clock);
		// IVM_TRACE("wait done!!\n");
		ivm_thread_dump(&state->thread_clock);
	}
#endif

	return;
}

IVM_INLINE
ivm_object_t *
_ivm_vmstate_spawnThread_c(ivm_vmstate_t *state,
						   ivm_coro_t *coro,
						   ivm_object_t *init)
{
	ivm_object_t *ret;

	// IVM_TRACE("hello!\n");

	ivm_coro_setSpawned(coro);

	// IVM_TRACE("wait for GIL\n");
	ivm_vmstate_threadStart(state);
	// IVM_TRACE("start!\n");
	ret = ivm_coro_resume(coro, state, init);
	ivm_vmstate_threadEnd(state);

	ivm_coro_unsetSpawned(coro);

	ivm_coro_set_remove(&state->thread_pool, coro);

	return ret;
}

struct spawn_arg_t {
	ivm_vmstate_t *state;
	ivm_coro_t *coro;
	ivm_object_t *arg;
};

void *
_spawn_sub(void *tmp)
{
	struct spawn_arg_t arg = *(struct spawn_arg_t *)tmp;
	return (void *)_ivm_vmstate_spawnThread_c(arg.state, arg.coro, arg.arg);
}

ivm_object_t *
ivm_vmstate_spawnThread(ivm_vmstate_t *state,
						ivm_coro_t *coro,
						ivm_object_t *init)
{
	struct spawn_arg_t *arg;
	ivm_thread_t th;

	if (!state->thread_init) {
		state->except
		= ivm_coro_newStringException(state->cur_coro, state, IVM_ERROR_MSG_DISABLED_THREAD);
		return IVM_NULL;
	}

	if (!ivm_coro_set_insert(&state->thread_pool, coro)) {
		state->except
		= ivm_coro_newStringException(state->cur_coro, state, IVM_ERROR_MSG_DUP_THREAD_CORO);
		return IVM_NULL;
	}

	arg = STD_ALLOC(sizeof(*arg));

	*arg = (struct spawn_arg_t) {
		state, coro, init
	};
	ivm_thread_init(&th, _spawn_sub, (void *)arg);

	// TODO: free the arg
	// TODO: thread pool

	return IVM_NONE(state);
}
