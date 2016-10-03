#include "std/mem.h"
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

#include "type.req.h"

IVM_PRIVATE
ivm_type_t static_type_list[] = {
#define TYPE_GEN(t, n, s, c, proto_init, ...) \
	{                                      \
		.tag = (t),                        \
		.name = #n,                        \
		.size = (s),                       \
		.cons = (c),                       \
		.is_builtin = IVM_TRUE,            \
		__VA_ARGS__                        \
	},

	#include "type.def.h"

#undef TYPE_GEN
};

#define SET_TYPE_PROTO(state, tag, obj) \
	(ivm_type_setProto(ivm_vmstate_getType((state), (tag)), (obj)))

ivm_vmstate_t *
ivm_vmstate_new(ivm_string_pool_t *const_pool)
{
	ivm_vmstate_t *ret = STD_ALLOC(sizeof(*ret));
	ivm_type_t *tmp_type, *end;
	ivm_cgroup_t *tmp_group;
	ivm_int_t i;

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("vm state"));

	for (i = 0; i < IVM_ARRLEN(ret->heaps); i++) {
		ivm_heap_init(ret->heaps + i, IVM_DEFAULT_INIT_HEAP_SIZE);
	}

	ret->cur_cgroup = 0;
	ivm_cgroup_list_init(&ret->coro_groups);
	ivm_cgroup_list_prepush(&ret->coro_groups, &tmp_group);
	ivm_cgroup_init(tmp_group);

	ivm_type_pool_init(&ret->type_pool);
	ivm_func_list_init(&ret->func_list);

	ret->func_pool = ivm_function_pool_new(IVM_DEFAULT_FUNCTION_POOL_SIZE);
	ret->ct_pool = ivm_context_pool_new(IVM_DEFAULT_CONTEXT_POOL_SIZE);
	ivm_coro_pool_init(&ret->cr_pool, IVM_DEFAULT_CORO_POOL_SIZE);

	ret->const_pool = const_pool;
	ivm_ref_inc(const_pool);

#define CONST_GEN(name, str) \
	ret->const_str_##name = (const ivm_string_t *)ivm_string_pool_registerRaw(const_pool, (str));
	#include "vm.const.h"
#undef CONST_GEN

	ret->wild_size = 0;

	ret->gc_flag = IVM_FALSE;
	ivm_collector_init(&ret->gc);

	ivm_vmstate_lockGCFlag(ret);

	ret->except = IVM_NULL;
	ivm_uid_gen_init(&ret->uid_gen);

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
	ivm_cgroup_list_iterator_t giter;

	ivm_type_t *i, *end;
	ivm_int_t j;

	if (state) {
		ivm_collector_dump(&state->gc, state);

		for (j = 0; j < IVM_ARRLEN(state->heaps); j++) {
			ivm_heap_dump(state->heaps + j);
		}

		IVM_CGROUP_LIST_EACHPTR(&state->coro_groups, giter) {
			ivm_cgroup_dump(IVM_CGROUP_LIST_ITER_GET_PTR(giter), state);
		}
		ivm_cgroup_list_dump(&state->coro_groups);

		ivm_type_pool_dump(&state->type_pool);

		ivm_func_list_dump(&state->func_list, state);

		ivm_function_pool_free(state->func_pool);
		ivm_context_pool_free(state->ct_pool);
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

void
ivm_vmstate_travAndCompactCGroup(ivm_vmstate_t *state,
								 ivm_traverser_arg_t *arg)
{
	ivm_cgroup_list_iterator_t giter;
	ivm_cgroup_t *group;

	IVM_CGROUP_LIST_EACHPTR(&state->coro_groups, giter) {
		group = IVM_CGROUP_LIST_ITER_GET_PTR(giter);
		if (ivm_cgroup_isAlive(group)) {
			ivm_cgroup_travAndCompact(group, arg);
		}
	}

	return;
}

ivm_cgid_t
ivm_vmstate_addCGroup(ivm_vmstate_t *state,
					  ivm_function_object_t *func)
{
	ivm_cgroup_list_iterator_t giter;
	ivm_coro_t *coro;
	ivm_cgroup_t *group, *ngroup = IVM_NULL;
	ivm_cgid_t gid = 0;

	/* find dead group */
	IVM_CGROUP_LIST_EACHPTR(&state->coro_groups, giter) {
		group = IVM_CGROUP_LIST_ITER_GET_PTR(giter);
		if (!ivm_cgroup_isAlive(group)) {
			ngroup = group;
			break;
		}
		gid++;
	}

	if (!ngroup) {
		// create new group
		ivm_cgroup_list_prepush(&state->coro_groups, &ngroup);
		ivm_cgroup_init(ngroup);
	}

	if (gid < 0) {
		IVM_FATAL(IVM_ERROR_MSG_GROUP_ID_OVERFLOW);
	}

	coro = ivm_coro_new(state);
	ivm_coro_setRoot(coro, state, func);
	ivm_cgroup_addCoro(ngroup, coro);

	return gid;
}

ivm_object_t *
ivm_vmstate_schedule_g(ivm_vmstate_t *state,
					   ivm_object_t *val,
					   ivm_cgid_t gid)
{
	ivm_cgroup_t *group = ivm_cgroup_list_at(&state->coro_groups, gid);
	ivm_cgid_t orig = state->cur_cgroup;
	ivm_coro_t *coro;

	if (!ivm_cgroup_isAlive(group)) {
		return val;
	}

	state->cur_cgroup = gid;
	ivm_cgroup_lock(group);

	do {
		coro = ivm_cgroup_curCoro(group);
		val = ivm_coro_resume(coro, state, val);
		
		if (!val) {
			// coro killed by an exception
			ivm_coro_printException(coro, state, ivm_vmstate_popException(state));
			val = IVM_NONE(state);
		}

		group = ivm_cgroup_list_at(&state->coro_groups, gid);
	} while (ivm_cgroup_switchCoro(group));

	ivm_cgroup_unlock(group);
	state->cur_cgroup = orig;

	ivm_cgroup_empty(group, state);

	return val;
}
