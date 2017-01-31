#include "std/mem.h"
#include "pub/err.h"
#include "pub/com.h"
#include "pub/inlines.h"

#include "std/heap.h"
#include "std/uid.h"
#include "std/string.h"
#include "std/time.h"
#include "std/thread.h"

#include "gc.h"
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

	ret->main_coro = IVM_NULL;
	ivm_coro_set_init(&ret->coro_set);

	ret->const_pool = const_pool;
	ivm_ref_inc(const_pool);

#define CONST_GEN(name, str) \
	ret->const_str_##name = ivm_string_pool_registerRaw(const_pool, (str));
	#include "vm.const.h"
#undef CONST_GEN
	
	ret->wild_size = 0;

	ret->gc_flag = IVM_FALSE;
	ivm_collector_init(&ret->gc);

	ret->thread_enabled = IVM_FALSE;
	ret->int_next = ret->int_head = 0;
	ret->has_int = IVM_FALSE;

#if IVM_USE_MULTITHREAD
	ivm_thread_mutex_init(&ret->int_lock);
#endif

	ivm_vmstate_lockGCFlag(ret);

	ret->except = IVM_NULL;
	ivm_uid_gen_init(&ret->st_uid_gen);

#define _STATE ret
	
	ivm_type_t static_type_list[] = {
	#define TYPE_GEN(t, n, s, c, proto_init, slots_init, ...) \
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
		ivm_proto_initProto(tmp_type, ret);
	}

	for (tmp_type = ret->type_list, end = tmp_type + IVM_TYPE_COUNT;
		 tmp_type != end; tmp_type++) {
		ivm_proto_initSlots(tmp_type, ret);
	}

	ivm_oprt_initType(ret);

	ret->loaded_mod = ivm_object_new(ret);
	// ret->tp_type = ivm_object_new(ret);
	ret->obj_none = ivm_vmstate_getTypeProto(ret, IVM_NONE_T); // ivm_none_new(ret);

	ivm_vmstate_unlockGCFlag(ret);

	ret->cur_path = IVM_VMSTATE_CONST(ret, C_EMPTY);
	// ret->coro_list_uid = ivm_uid_gen_nextPtr(ret->uid_gen);
	
	ret->join_start = IVM_FALSE;

	return ret;
}

void
ivm_vmstate_free(ivm_vmstate_t *state)
{
	// ivm_cgroup_list_iterator_t giter;

	ivm_type_t *i, *end;
	ivm_int_t j;

	if (state) {
		// ivm_vmstate_cleanThread(state);
		ivm_coro_free(state->main_coro, state);

		ivm_collector_dump(&state->gc, state);

		if (ivm_coro_set_size(&state->coro_set)) {
			IVM_FATAL("impossible");
		}

		ivm_coro_set_dump(&state->coro_set);

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

#if IVM_USE_MULTITHREAD
		if (state->thread_enabled) {
			ivm_cthread_set_dump(&state->thread_set);
			ivm_cthread_pool_destruct(&state->thread_pool);
		}
#endif

		ivm_function_pool_free(state->func_pool);
		ivm_context_pool_destruct(&state->ct_pool);
		ivm_coro_pool_destruct(&state->cr_pool);

		ivm_string_pool_free(state->const_pool);

		for (i = state->type_list, end = i + IVM_TYPE_COUNT;
			 i != end; i++) {
			ivm_type_dump(i);
		}

		// STD_FREE(state->cur_path);
		STD_FREE(state);
	}

	return;
}

#if IVM_USE_MULTITHREAD

void
ivm_vmstate_threadYield(ivm_vmstate_t *state)
{
	const ivm_string_t *cur_path = ivm_vmstate_getCurPath(state);

	// IVM_TRACE("interrupted\n");
	ivm_vmstate_unlockGIL(state);
	ivm_vmstate_setCSL(state);

	ivm_time_msleep(1);

	// IVM_TRACE("try to lock!\n");

	ivm_vmstate_lockGIL(state);

	// IVM_TRACE("arised!\n");

	ivm_vmstate_setCurPath_c(state, cur_path);

	return;
}

IVM_INLINE
void
_clock_round(ivm_vmstate_t *state)
{
	ivm_time_msleep(10);
	// IVM_TRACE("########signal! %d\n", ivm_cthread_set_size(&state->thread_set));
	ivm_vmstate_setInt(state, IVM_CORO_INT_THREAD_YIELD);

	// IVM_TRACE("wait for csl\n");
	while (!ivm_vmstate_getCSL(state)) {
		ivm_thread_cancelPoint();
		// ivm_time_msleep(10);
	}
	// IVM_TRACE("wait for csl\n");

	ivm_vmstate_unsetCSL(state);

	return;
}

IVM_PRIVATE
void *
_thread_clock(void *arg)
{
	ivm_vmstate_t *state = (ivm_vmstate_t *)arg;

	while (1) {
		_clock_round(state);
	}

	return IVM_NULL;
}

#endif

void
ivm_vmstate_enableThread(ivm_vmstate_t *state)
{
#if IVM_USE_MULTITHREAD
	if (!state->thread_enabled) {
		ivm_cthread_set_init(&state->thread_set);
		ivm_cthread_pool_init(&state->thread_pool, IVM_DEFAULT_CTHREAD_POOL_SIZE);
		ivm_thread_mutex_init(&state->thread_gil);
		ivm_thread_mutex_init(&state->thread_csl_prot);
		state->thread_csl = IVM_FALSE;

		ivm_thread_init(&state->thread_clock, _thread_clock, (void *)state);

		state->thread_enabled = IVM_TRUE;
		ivm_vmstate_threadStart(state);
	}
#endif

	return;
}

IVM_INLINE
void
_ivm_vmstate_joinClock(ivm_vmstate_t *state)
{
#if IVM_USE_MULTITHREAD
	if (state->thread_enabled) {
		ivm_thread_cancel(&state->thread_clock);
		ivm_thread_wait(&state->thread_clock);
		ivm_thread_dump(&state->thread_clock);
	}
#endif

	return;
}

#if IVM_USE_MULTITHREAD

IVM_INLINE
ivm_object_t *
_ivm_vmstate_spawnThread_c(ivm_vmstate_t *state,
						   ivm_coro_t *coro,
						   ivm_coro_thread_t *cthread,
						   const ivm_string_t *cur_path)
{
	ivm_object_t *ret;

	// IVM_TRACE("wait for GIL\n");

	// lock GIL
	ivm_vmstate_threadStart(state);

	ivm_vmstate_setCurPath_c(state, cur_path);

	// real execution start
	ret = ivm_coro_resume(coro, state, IVM_NULL);

	// execution end
	ivm_coro_unsetSpawned(coro);
	if (!ivm_cthread_set_remove(&state->thread_set, cthread)) {
		IVM_FATAL("impossible");
	}

	ivm_coro_thread_free(cthread, state);

	// IVM_TRACE("release lock %d\n", ivm_cthread_set_size(&state->thread_set));

	// unlock GIL
	ivm_vmstate_threadEnd(state);

	return ret;
}

struct spawn_arg_t {
	ivm_vmstate_t *state;
	ivm_coro_t *coro;
	ivm_coro_thread_t *cthread;
	const ivm_string_t *cur_path;
};

void *
_spawn_sub(void *tmp)
{
	struct spawn_arg_t arg = *(struct spawn_arg_t *)tmp;

	STD_FREE(tmp);

	return (void *)_ivm_vmstate_spawnThread_c(arg.state, arg.coro, arg.cthread, arg.cur_path);
}

#endif // IVM_USE_MULTITHREAD

ivm_object_t *
ivm_vmstate_spawnThread(ivm_vmstate_t *state,
						ivm_coro_t *coro,
						ivm_object_t *init)
{
#if IVM_USE_MULTITHREAD

	struct spawn_arg_t *arg;
	ivm_coro_thread_t *new_th;
	ivm_thread_t *tid;

	if (!state->thread_enabled) {
		state->except
		= ivm_exception_new(state, IVM_ERROR_MSG_DISABLED_THREAD, "<core>", 0);
		return IVM_NULL;
	}

	new_th = ivm_coro_thread_new(state, coro);
	ivm_cthread_set_insert(&state->thread_set, new_th);

	tid = ivm_coro_thread_getTID(new_th);

	arg = STD_ALLOC(sizeof(*arg));

	*arg = (struct spawn_arg_t) {
		state, coro, new_th, ivm_vmstate_getCurPath(state)
	};

	// push argument ahead of time in case of GC
	if (init) {
		ivm_vmstack_push(coro, init);
	}

	ivm_coro_setSpawned(coro);
	ivm_thread_init(tid, _spawn_sub, (void *)arg);

	return ivm_coro_object_new(state, coro);

#else

	state->except
	= ivm_exception_new(state, IVM_ERROR_MSG_DISABLED_THREAD, "<core>", 0);
	return IVM_NULL;

#endif
}

ivm_bool_t
ivm_vmstate_joinAllThread(ivm_vmstate_t *state,
						  ivm_bool_t stop_clock)
{
#if IVM_USE_MULTITHREAD

	ivm_cthread_set_t *tset = &state->thread_set;

	// IVM_TRACE("join!\n");
	if (state->join_start) {
		// there has already one thread on join
		return IVM_FALSE;
	}

	state->join_start = IVM_TRUE;

	if (ivm_vmstate_hasThread(state)) {
		ivm_vmstate_threadEnd(state);

		while (ivm_cthread_set_size(tset)) {
			ivm_time_msleep(1);
			// IVM_TRACE("%d\n", ivm_cthread_set_size(tset));
		}

		ivm_vmstate_lockGIL(state);

		if (stop_clock) {
			_ivm_vmstate_joinClock(state);
		}
	}

	state->join_start = IVM_FALSE;

#endif

	return IVM_TRUE;
}

void
ivm_vmstate_initInstrCache(ivm_vmstate_t *state)
{
	ivm_func_list_iterator_t iter;
	ivm_function_t *func;

	IVM_FUNC_LIST_EACHPTR(&state->func_list, iter) {
		func = IVM_FUNC_LIST_ITER_GET(iter);
		ivm_function_initInstrCache(func);
	}

	ivm_uid_gen_init(&(state)->st_uid_gen);

	return;
}
