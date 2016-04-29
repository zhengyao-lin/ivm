#include "pub/mem.h"
#include "gc.h"
#include "cell.h"
#include "../slot.h"
#include "../obj.h"
#include "../vmstack.h"
#include "../coro.h"
#include "../call.h"
#include "../context.h"
#include "../vm.h"
#include "../err.h"

ivm_collector_t *
ivm_collector_new(ivm_vmstate_t *state)
{
	ivm_collector_t *ret = MEM_ALLOC(sizeof(*ret));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("garbage collector"));

	ret->obj_count = 0;
	ret->obj_set = ivm_cell_set_new();
	ret->period = 1;
	ret->state = state;

	return ret;
}

void
ivm_collector_free(ivm_collector_t *collector)
{
	if (collector) {
		ivm_cell_set_free(collector->obj_set);
		MEM_FREE(collector);
	}

	return;
}

void
ivm_collector_dispose(ivm_collector_t *collector,
					  ivm_vmstate_t *state)
{
	if (collector) {
		ivm_cell_set_dispose(collector->obj_set, state);
		MEM_FREE(collector);
	}
	
	return;
}

static void
ivm_collector_markObject(ivm_object_t *obj,
						 ivm_collector_t *collector);

static void
ivm_collector_markSlot(ivm_slot_t *slot,
					   ivm_collector_t *collector)
{
	ivm_collector_markObject(ivm_slot_getValue(slot, IVM_COLLECTOR_STATE(collector)),
							 collector);
	return;
}

static void
ivm_collector_markObject(ivm_object_t *obj,
						 ivm_collector_t *collector)
{
	ivm_marker_t marker;

	if (!obj
		|| IVM_OBJECT_MARK(obj)
		   == IVM_COLLECTOR_PERIOD(collector)) {
		/* NULL object or has been marked */
		return;
	}

	IVM_OBJECT_MARK(obj)
	= IVM_COLLECTOR_PERIOD(collector);

	ivm_slot_table_foreach(IVM_OBJECT_SLOTS(obj), 
						   (ivm_slot_table_foreach_proc_t)
						   ivm_collector_markSlot,
						   collector);

	marker = IVM_TYPE_MARKER_OF(obj);
	if (marker) {
		marker(obj, collector);
	}

	return;
}

static void
ivm_collector_markContextChain(ivm_ctchain_t *chain,
							   ivm_collector_t *collector)
{
	if (chain)
		ivm_ctchain_foreach(chain,
							(ivm_ctchain_foreach_proc_t)
							ivm_collector_markObject,
							collector);
	return;
}

static void
ivm_collector_markCallerInfo(ivm_caller_info_t *info,
							 ivm_collector_t *collector)
{
	if (info)
		ivm_collector_markContextChain(IVM_CALLER_INFO_CONTEXT(info),
									   collector);
	return;
}

static void
ivm_collector_markRuntime(ivm_runtime_t *runtime,
						  ivm_collector_t *collector)
{
	if (runtime)
		ivm_collector_markContextChain(IVM_RUNTIME_CONTEXT(runtime),
									   collector);
	return;
}

static void
ivm_collector_markCoro(ivm_coro_t *coro,
					   ivm_collector_t *collector)
{
	ivm_vmstack_t *stack = IVM_CORO_STACK(coro);
	ivm_call_stack_t *call_st = IVM_CORO_CALL_STACK(coro);
	ivm_runtime_t *runtime = IVM_CORO_RUNTIME(coro);

	ivm_vmstack_foreach_arg(stack,
	 						(ivm_ptlist_foreach_proc_arg_t)
	 						ivm_collector_markObject,
	 						collector);
	ivm_call_stack_foreach_arg(call_st,
							   (ivm_ptlist_foreach_proc_arg_t)
	 						   ivm_collector_markCallerInfo,
	 						   collector);
	ivm_collector_markRuntime(runtime, collector);

	return;
}

void
ivm_collector_markState(ivm_collector_t *collector,
						ivm_vmstate_t *state)
{
	ivm_coro_list_t *coros = IVM_VMSTATE_CORO_LIST(state);

	ivm_coro_list_foreach_arg(coros,
							  (ivm_ptlist_foreach_proc_arg_t)
							  ivm_collector_markCoro,
							  collector);

	return;
}

static
void
ivm_collector_disposeCellIfNoMark(ivm_cell_t *cell,
								  ivm_cell_set_t *set,
								  ivm_collector_t *collector,
								  ivm_vmstate_t *state)
{
	if (IVM_OBJECT_MARK(IVM_CELL_OBJ(cell))
		!= IVM_COLLECTOR_PERIOD(collector)) {
		ivm_cell_removeFrom(cell, set);
		ivm_cell_dispose(cell, state);
	}
	return;
}

static
void
ivm_collector_cleanGarbage(ivm_collector_t *collector,
						   ivm_vmstate_t *state)
{
	ivm_cell_set_foreach(IVM_COLLECTOR_OBJ_SET(collector),
						 (ivm_cell_set_foreach_proc_t)
						 ivm_collector_disposeCellIfNoMark,
						 collector, state);
	return;
}

void
ivm_collector_collect(ivm_collector_t *collector)
{
	ivm_collector_markState(collector, collector->state);
	ivm_collector_cleanGarbage(collector, collector->state);
	ivm_collector_incPeriod(collector);
	return;
}
