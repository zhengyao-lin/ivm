#ifndef _IVM_VM_VM_H_
#define _IVM_VM_VM_H_

#include "pub/com.h"
#include "pub/const.h"
#include "pub/type.h"

#include "std/pool.h"
#include "std/string.h"
#include "std/heap.h"
#include "std/uid.h"

#include "gc/gc.h"
#include "obj.h"
#include "num.h"
#include "coro.h"
#include "context.h"

IVM_COM_HEADER

typedef struct ivm_vmstate_t_tag {
	ivm_type_t type_list[IVM_TYPE_COUNT];		// 240 * 6 = 1440

	ivm_coro_pool_t cr_pool;					// 72

	ivm_function_pool_t *func_pool;				// 8

	ivm_heap_t heaps[3];						// 120

	ivm_context_pool_t *ct_pool;				// 8

	ivm_cgroup_list_t coro_groups;

	// ivm_type_list_t type_list;
	ivm_func_list_t func_list;					// 24

	ivm_string_pool_t *const_pool;				// 8
	ivm_collector_t *gc;						// 8

	ivm_object_t *except;

#define CONST_GEN(name, str) const ivm_string_t *const_str_##name;
	#include "vm.const.h"						// 8
#undef CONST_GEN

	ivm_cgid_t cur_cgroup;
	ivm_int_t gc_flag; /* gc flag:				// 4
						  > 0: open
						  = 0: closed
						  < 0: locked */

	ivm_uid_gen_t uid_gen;						// 4
} ivm_vmstate_t;

#define IVM_VMSTATE_GET_TYPE_LIST(state) ((state)->type_list)
#define IVM_VMSTATE_GET_CONST_POOL(state) ((state)->const_pool)
#define IVM_VMSTATE_GET_CUR_HEAP(state) ((state)->heaps)

#define IVM_VMSTATE_GET(obj, member) IVM_GET((obj), IVM_VMSTATE, member)
#define IVM_VMSTATE_SET(obj, member, val) IVM_SET((obj), IVM_VMSTATE, member, (val))

#define IVM_VMSTATE_CONST(state, name) ((state)->const_str_##name)

#define IVM_CSTR(state, str) ((const ivm_string_t *)ivm_string_pool_registerRaw((state)->const_pool, (str)))

ivm_vmstate_t *
ivm_vmstate_new();

void
ivm_vmstate_free(ivm_vmstate_t *state);

#define ivm_vmstate_isGCFlagOpen(state) ((state)->gc_flag == 1)

IVM_INLINE
void
ivm_vmstate_openGCFlag(ivm_vmstate_t *state)
{
	if (state->gc_flag >= 0)
		state->gc_flag = 1;
	return;
}

IVM_INLINE
void
ivm_vmstate_closeGCFlag(ivm_vmstate_t *state)
{
	if (state->gc_flag >= 0)
		state->gc_flag = 0;
	return;
}

#define ivm_vmstate_lockGCFlag(state) ((state)->gc_flag = -1)
#define ivm_vmstate_unlockGCFlag(state) ((state)->gc_flag = 0)

#if 0

#define ivm_vmstate_curHeap(state) ((state)->heap_count > 0 ? (state)->heaps[(state)->heap_count - 1] : IVM_NULL)
void
ivm_vmstate_addHeap(ivm_vmstate_t *state);
ivm_heap_t *
ivm_vmstate_findHeap(ivm_vmstate_t *state, ivm_object_t *obj);

#endif

#define ivm_vmstate_getHeapAt(state, i) ((state)->heaps + (i))
#define ivm_vmstate_getHeaps(state) ((state)->heaps)

IVM_INLINE
ivm_bool_t
IVM_WBOBJ(ivm_vmstate_t *state,
		  ivm_object_t *parent,
		  ivm_object_t *child)
{
	if (IVM_OBJECT_GET(parent, GEN) &&
		!IVM_OBJECT_GET(child, GEN)) {
		if (!IVM_OBJECT_GET(parent, WB)) {
			ivm_collector_addWBObj(state->gc, parent);
			IVM_OBJECT_SET(parent, WB, 1);
		}
		return IVM_TRUE;
	}

	return IVM_FALSE;
}

IVM_INLINE
ivm_bool_t
IVM_WBOBJ_SLOT(ivm_vmstate_t *state,
			   ivm_object_t *obj,
			   ivm_slot_table_t *table)
{
	if (obj && IVM_OBJECT_GET(obj, GEN) &&
		!ivm_slot_table_getGen(table)) {
		if (!IVM_OBJECT_GET(obj, WB)) {
			ivm_collector_addWBObj(state->gc, obj);
			IVM_OBJECT_SET(obj, WB, 1);
		}
		return IVM_TRUE;
	}

	return IVM_FALSE;
}

IVM_INLINE
ivm_bool_t
IVM_WBSLOT(ivm_vmstate_t *state,
		   ivm_slot_table_t *table,
		   ivm_object_t *value)
{
	// IVM_TRACE("wb %d %d\n", ivm_slot_table_getGen(table), IVM_OBJECT_GET(value, GEN));
	if (ivm_slot_table_getGen(table) && value &&
		!IVM_OBJECT_GET(value, GEN)) {
		if (!ivm_slot_table_getWB(table)) {
			// IVM_TRACE("write %p %p(%s)\n", table, value, IVM_OBJECT_GET(value, TYPE_NAME));
			ivm_collector_addWBSlotTable(state->gc, table);
			ivm_slot_table_setWB(table, 1);
		}
		return IVM_TRUE;
	}

	return IVM_FALSE;
}

IVM_INLINE
ivm_bool_t
IVM_WBCTX(ivm_vmstate_t *state,
		  ivm_context_t *ctx,
		  ivm_slot_table_t *value)
{
	if (ivm_context_getGen(ctx) && value &&
		!ivm_slot_table_getGen(value)) {
		if (!ivm_context_getWB(ctx)) {
			ivm_collector_addWBContext(state->gc, ctx);
			ivm_context_setWB(ctx, 1);
		}
		return IVM_TRUE;
	}

	return IVM_FALSE;
}

IVM_INLINE
void *
ivm_vmstate_alloc(ivm_vmstate_t *state, ivm_size_t size)
{
	ivm_bool_t add_block = IVM_FALSE;
	void *ret = ivm_heap_alloc_c((state)->heaps, size, &add_block);

	if (add_block) {
		ivm_vmstate_openGCFlag(state);
	}

	return ret;
}

IVM_INLINE
void *
ivm_vmstate_addCopy(ivm_vmstate_t *state,
					void *ptr, ivm_size_t size)
{
	ivm_bool_t add_block = IVM_FALSE;
	void *ret = ivm_heap_addCopy_c((state)->heaps, ptr, size, &add_block);

	if (add_block) {
		ivm_vmstate_openGCFlag(state);
	}

	return ret;
}

IVM_INLINE
void *
ivm_vmstate_allocAt(ivm_vmstate_t *state, ivm_size_t size, ivm_int_t heap)
{
	ivm_bool_t add_block = IVM_FALSE;
	void *ret = ivm_heap_alloc_c((state)->heaps + heap, size, &add_block);

	if (add_block) {
		ivm_vmstate_openGCFlag(state);
	}

	return ret;
}

IVM_INLINE
void
ivm_vmstate_swapHeap(ivm_vmstate_t *state,
					 ivm_int_t i, ivm_int_t j)
{
	ivm_heap_t tmp = state->heaps[i];
	state->heaps[i] = state->heaps[j];
	state->heaps[j] = tmp;

	return;
}

/* function pool */
#if IVM_USE_FUNCTION_POOL

#define ivm_vmstate_allocFunc(state) \
	(ivm_function_pool_alloc((state)->func_pool))
#define ivm_vmstate_dumpFunc(state, func) \
	(ivm_function_pool_dump((state)->func_pool, (func)))

#else

#define ivm_vmstate_allocFunc(state) \
	(MEM_ALLOC(sizeof(ivm_function_t), ivm_function_t *))
#define ivm_vmstate_dumpFunc(state, func) \
	(MEM_FREE(func))

#endif

#define ivm_vmstate_allocFrame(state) \
	(MEM_ALLOC(sizeof(ivm_frame_t), ivm_frame_t *))
#define ivm_vmstate_dumpFrame(state, fr) \
	(MEM_FREE(fr))

/* coro pool */
#if IVM_USE_CORO_POOL

#define ivm_vmstate_allocCoro(state) \
	(ivm_coro_pool_alloc(&(state)->cr_pool))
#define ivm_vmstate_dumpCoro(state, cr) \
	(ivm_coro_pool_dump(&(state)->cr_pool, (cr)))

#else

#define ivm_vmstate_allocCoro(state) \
	(MEM_ALLOC(sizeof(ivm_coro_t), ivm_coro_t *))
#define ivm_vmstate_dumpCoro(state, cr) \
	(MEM_FREE(cr))

#endif

/* context pool */
#define ivm_vmstate_allocContext(state) \
	(ivm_context_pool_alloc((state)->ct_pool))
#define ivm_vmstate_dumpContext(state, ctx) \
	(ivm_context_pool_dump((state)->ct_pool, (ctx)))

#if 0

ivm_object_t *
ivm_vmstate_newObject(ivm_vmstate_t *state);
void
ivm_vmstate_freeObject(ivm_vmstate_t *state, ivm_object_t *obj);

#endif

#define ivm_vmstate_getType(state, tag) (&(state)->type_list[tag])
#define ivm_vmstate_getTypeProto(state, tag) \
	(ivm_type_getProto(ivm_vmstate_getType((state), (tag))))
#define ivm_vmstate_getTypeHeader(state, tag) \
	(ivm_type_getHeader(ivm_vmstate_getType((state), (tag))))

#define ivm_vmstate_registerFunc(state, exec) (ivm_func_list_register(&(state)->func_list, (exec)))
#define ivm_vmstate_getFunc(state, id) (ivm_func_list_at(&(state)->func_list, (id)))
#define ivm_vmstate_getFuncID(state, func) (ivm_func_list_find(&(state)->func_list, (func)))
#define ivm_vmstate_getLinkOffset(state) (ivm_func_list_size(&(state)->func_list))

#define ivm_vmstate_addDesLog(state, obj) (ivm_collector_addDesLog((state)->gc, (obj)))

#if 0

#define ivm_vmstate_checkGC(state) (ivm_collector_collect((state)->gc, (state), (state)->cur_heap))

#else

#define ivm_vmstate_checkGC(state) \
	ivm_vmstate_isGCFlagOpen(state)

#define ivm_vmstate_doGC(state) \
	ivm_collector_collect((state)->gc, (state), (state)->heaps); \
	ivm_vmstate_closeGCFlag(state);

#endif

#define ivm_vmstate_setException(state, obj) ((state)->except = (obj))
#define ivm_vmstate_getException(state) ((state)->except)

IVM_INLINE
ivm_object_t *
ivm_vmstate_popException(ivm_vmstate_t *state)
{
	ivm_object_t *tmp = state->except;
	state->except = IVM_NULL;
	return tmp;
}

IVM_INLINE
ivm_size_t
ivm_vmstate_addCoro_c(ivm_vmstate_t *state,
					  ivm_coro_t *coro,
					  ivm_cgid_t gid)
{
	return ivm_cgroup_addCoro(ivm_cgroup_list_at(&state->coro_groups, gid), coro);
}

IVM_INLINE
ivm_size_t
ivm_vmstate_addCoro(ivm_vmstate_t *state,
					ivm_function_object_t *func,
					ivm_cgid_t gid)
{
	ivm_coro_t *coro;

	coro = ivm_coro_new(state);
	ivm_coro_setRoot(coro, state, func);
	
	return ivm_vmstate_addCoro_c(state, coro, gid);
}

IVM_INLINE
ivm_size_t
ivm_vmstate_addCoroToCurCGroup(ivm_vmstate_t *state,
							   ivm_function_object_t *func)
{
	return ivm_vmstate_addCoro(state, func, state->cur_cgroup);
}

IVM_INLINE
ivm_size_t
ivm_vmstate_addCoroToCurCGroup_c(ivm_vmstate_t *state,
								 ivm_coro_t *coro)
{
	return ivm_vmstate_addCoro_c(state, coro, state->cur_cgroup);
}

ivm_cgid_t
ivm_vmstate_addCGroup(ivm_vmstate_t *state,
					  ivm_function_object_t *func);

IVM_INLINE
ivm_cgroup_t *
ivm_vmstate_curCGroup(ivm_vmstate_t *state)
{
	return ivm_cgroup_list_at(&state->coro_groups, state->cur_cgroup);
}

IVM_INLINE
ivm_coro_t *
ivm_vmstate_curCoro(ivm_vmstate_t *state)
{
	return ivm_cgroup_curCoro(ivm_cgroup_list_at(&state->coro_groups, state->cur_cgroup));
}

IVM_INLINE
ivm_bool_t
ivm_vmstate_hasCGroup(ivm_vmstate_t *state, ivm_cgid_t gid)
{
	return gid < ivm_cgroup_list_size(&state->coro_groups);
}

void
ivm_vmstate_travAndCompactCGroup(ivm_vmstate_t *state,
								 ivm_traverser_arg_t *arg);

IVM_INLINE
ivm_bool_t
ivm_vmstate_isCGroupLocked(ivm_vmstate_t *state, ivm_cgid_t gid)
{
	return ivm_cgroup_isLocked(ivm_cgroup_list_at(&state->coro_groups, gid));
}

ivm_object_t *
ivm_vmstate_schedule_g(ivm_vmstate_t *state,
					   ivm_object_t *val,
					   ivm_cgid_t gid);

IVM_INLINE
ivm_object_t *
ivm_vmstate_schedule(ivm_vmstate_t *state)
{
	return ivm_vmstate_schedule_g(state, IVM_NULL, 0);
}

#define ivm_vmstate_genUID(state) (ivm_uid_gen_nextPtr((state)->uid_gen))

/*
ivm_object_t *
ivm_vmstate_execute();
*/

IVM_COM_END

#endif
