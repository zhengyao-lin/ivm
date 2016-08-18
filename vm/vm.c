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
	ret->max_cgroup = 0;
	ret->cur_cgroup = 0;
	ivm_coro_list_init(&ret->coro_list);
	ivm_cgroup_stack_init(&ret->cgroup_stack);
	
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

	ret->except = IVM_NULL;
	ivm_uid_gen_init(&ret->uid_gen);

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

	ret->coro_list_uid = ivm_uid_gen_nextPtr(ret->uid_gen);

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
		ivm_cgroup_stack_dump(&state->cgroup_stack);

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

ivm_size_t
ivm_vmstate_addCoro(ivm_vmstate_t *state,
					ivm_function_object_t *func)
{
	ivm_coro_t *coro = ivm_coro_new(state);
	ivm_coro_setRoot(coro, state, func);
	return ivm_coro_list_add(&state->coro_list, coro);
}

ivm_cgid_t
ivm_vmstate_addGroup(ivm_vmstate_t *state,
					 ivm_function_object_t *func)
{
	ivm_coro_t *coro = ivm_coro_new(state);

	ivm_coro_setRoot(coro, state, func);
	ivm_coro_list_add(&state->coro_list, coro);

	if (++state->max_cgroup < 0) {
		IVM_FATAL(IVM_ERROR_MSG_GROUP_ID_OVERFLOW);
	}

	return ivm_coro_setGroup(coro, state->max_cgroup);
}

ivm_cgid_t
ivm_vmstate_addToGroup(ivm_vmstate_t *state,
					   ivm_function_object_t *func,
					   ivm_cgid_t gid)
{
	ivm_coro_t *coro = ivm_coro_new(state);

	ivm_coro_setRoot(coro, state, func);
	ivm_coro_list_add(&state->coro_list, coro);

	return ivm_coro_setGroup(coro, gid);
}

ivm_cgid_t
ivm_vmstate_addToCurrentGroup(ivm_vmstate_t *state,
							  ivm_function_object_t *func)
{
	ivm_coro_t *coro = ivm_coro_new(state);

	ivm_coro_setRoot(coro, state, func);
	ivm_coro_list_add(&state->coro_list, coro);

	return ivm_coro_setGroup(coro, state->cur_cgroup);
}

void
ivm_vmstate_yieldTo(ivm_vmstate_t *state,
					ivm_cgid_t gid)
{
	ivm_coro_setCur(ivm_coro_list_at(&state->coro_list, state->cur_coro));
	// IVM_TRACE("*** from coro %d\n", state->cur_coro);
	// IVM_TRACE("yield to %d(cur %d in %d)\n", gid, state->cur_coro, state->cur_cgroup);
	ivm_cgroup_stack_push(&state->cgroup_stack, state->cur_cgroup);
	state->cur_cgroup = gid;

	return;
}

/* switch to the current coroutine in the previous group */
IVM_PRIVATE
IVM_INLINE
ivm_bool_t
_ivm_vmstate_popCGroup(ivm_vmstate_t *state)
{
	ivm_coro_list_t *list = &state->coro_list;
	ivm_coro_list_iterator_t iter;
	ivm_cgid_t tmp_gid;

	while (1) {
		if (ivm_cgroup_stack_isEmpty(&state->cgroup_stack))
			return IVM_FALSE;

		tmp_gid = state->cur_cgroup = ivm_cgroup_stack_pop(&state->cgroup_stack);

		// IVM_TRACE("*** back to group %d\n", tmp_gid);

		// IVM_TRACE("pop %d!\n", tmp_gid);
		/* restore the current coroutine in the previous group */
		IVM_CORO_LIST_EACHPTR(list, iter) {
			/*
			if (ivm_coro_isGroup(IVM_CORO_LIST_ITER_GET(iter), tmp_gid) &&
				ivm_coro_isCur(IVM_CORO_LIST_ITER_GET(iter))) {
				// IVM_TRACE("haaa\n");
				ivm_coro_resetCur(IVM_CORO_LIST_ITER_GET(iter));
				state->cur_coro = IVM_CORO_LIST_ITER_INDEX(list, iter);
				// IVM_TRACE("*** coro %d\n", state->cur_coro);
				return IVM_TRUE;
			}
			*/

			if (ivm_coro_isGroup(IVM_CORO_LIST_ITER_GET(iter), tmp_gid) &&
				ivm_coro_isAlive(IVM_CORO_LIST_ITER_GET(iter))) {
				state->cur_coro = IVM_CORO_LIST_ITER_INDEX(list, iter);
				return IVM_TRUE;
			}
		}

		/* no current coroutine in the  previous group */
		/* pop again */
	}

	IVM_FATAL("impossible");
}

IVM_PRIVATE
IVM_INLINE
ivm_bool_t
_ivm_vmstate_switchCoro(ivm_vmstate_t *state)
{
	ivm_coro_list_t *list = &state->coro_list;
	ivm_coro_list_iterator_t i, end;
	ivm_int_t cur_group = state->cur_cgroup;

	// if (!ivm_coro_list_size(list)) return IVM_FALSE;

	for (i = IVM_CORO_LIST_ITER_AT(list, state->cur_coro + 1),
		 end = IVM_CORO_LIST_ITER_END(list);
		 i != end; i++) {
		if (ivm_coro_isGroup(IVM_CORO_LIST_ITER_GET(i), cur_group) &&
			ivm_coro_isAlive(IVM_CORO_LIST_ITER_GET(i))) {
			state->cur_coro = IVM_CORO_LIST_ITER_INDEX(list, i);
			return IVM_TRUE;
		}
	}

	for (end = i,
		 i = IVM_CORO_LIST_ITER_BEGIN(list);
		 i != end; i++) {
		if (ivm_coro_isGroup(IVM_CORO_LIST_ITER_GET(i), cur_group) &&
			ivm_coro_isAlive(IVM_CORO_LIST_ITER_GET(i))) {
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

	do {
		// while (ivm_coro_list_size(coros)) {
		do {
			ret = ivm_coro_resume(ivm_coro_list_at(coros, state->cur_coro),
								  state, ret);

			if (ivm_vmstate_getException(state)) {
				ivm_vmstate_setException(state, IVM_NULL);
			}
		} while (_ivm_vmstate_switchCoro(state));
	} while (_ivm_vmstate_popCGroup(state));

	state->cur_coro = 0;

	return;
}

/* schedule one round(yield in nested native call) */
ivm_object_t *
ivm_vmstate_schedule_r(ivm_vmstate_t *state,
					   ivm_object_t *ret)
{
	ivm_coro_list_t *coros = &state->coro_list;

	ivm_uid_t uid = state->coro_list_uid;
	ivm_size_t ocoro = state->cur_coro;
	ivm_coro_t *coro, *skip = ivm_coro_list_at(coros, ocoro);

	ivm_cgid_t gid = state->cur_cgroup;

	if (!_ivm_vmstate_switchCoro(state)) {
		goto RET;
	}

	do {
		do {			
			coro = ivm_coro_list_at(coros, state->cur_coro);

			/* back to the original coroutine */
			if (coro == skip) {
				// IVM_TRACE("*** wow return\n");
				goto RET;
			}
			/*
				because the current coroutine mechanism is not involving
				C context saving for portability, so only one coroutine can
				have nested native call, or the yield order will be broken.

				That is, if that happens, the control will fall back to the last
				yield in the coroutine that has nested native call
				(return to the caller of this function)
			 */
			if (IVM_CORO_GET(coro, HAS_NATIVE)) {
				IVM_TRACE("*** coro schedule out of order ***\n");

				// goto NEXT;
#if 1
				state->cur_cgroup = gid;
				if (state->coro_list_uid == uid) {
					/* coro list not changed */
					state->cur_coro = ocoro;
				} else {
					do {
						_ivm_vmstate_switchCoro(state);
					} while (ivm_coro_list_at(coros, state->cur_coro) != coro);
				}

				goto RET;
#endif
			}

			// IVM_TRACE("*** start coro %d\n", state->cur_coro);
			ret = ivm_coro_resume(coro, state, ret);

			if (ivm_vmstate_getException(state)) {
				ivm_vmstate_setException(state, IVM_NULL);
			}
// NEXT:
		} while (_ivm_vmstate_switchCoro(state));
	} while (_ivm_vmstate_popCGroup(state));

RET:
	return ret;
}
