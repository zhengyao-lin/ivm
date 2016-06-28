#include "pub/mem.h"
#include "pub/const.h"
#include "pub/com.h"
#include "pub/err.h"
#include "pub/vm.h"

#include "util/perf.h"

#include "../slot.h"
#include "../obj.h"
#include "../vmstack.h"
#include "../coro.h"
#include "../call.h"
#include "../context.h"

#include "gc.h"

ivm_collector_t *
ivm_collector_new()
{
	ivm_collector_t *ret = MEM_ALLOC(sizeof(*ret),
									 ivm_collector_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("garbage collector"));

	ret->des_log[0] = ivm_destruct_list_new();
	ret->des_log[1] = ivm_destruct_list_new();

	return ret;
}

void
ivm_collector_free(ivm_collector_t *collector, ivm_vmstate_t *state)
{
	if (collector) {
		ivm_destruct_list_free(collector->des_log[0]);
		ivm_destruct_list_free(collector->des_log[1]);
		MEM_FREE(collector);
	}

	return;
}

IVM_PRIVATE
ivm_object_t *
ivm_collector_copyObject(ivm_object_t *obj,
						 ivm_traverser_arg_t *arg)
{
	ivm_object_t *ret = IVM_NULL;
	ivm_traverser_t trav;
	ivm_slot_table_iterator_t siter;
	ivm_slot_table_t *tmp;

	if (!obj) return IVM_NULL;

	if (IVM_OBJECT_GET(obj, COPY))
		return IVM_OBJECT_GET(obj, COPY);
	else if (ivm_heap_isIn(arg->heap, obj))
		return obj;

	ret = ivm_heap_addCopy(arg->heap, obj, IVM_OBJECT_GET(obj, TYPE_SIZE));

	IVM_OBJECT_SET(ret, COPY, IVM_NULL); /* remove the new object's copy */
	IVM_OBJECT_SET(obj, COPY, ret);

	IVM_OBJECT_SET(
		ret, SLOTS,
		tmp = ivm_slot_table_copy(IVM_OBJECT_GET(ret, SLOTS), arg->state, arg->heap)
	);

	if (tmp) {
		IVM_SLOT_TABLE_EACHPTR(tmp, siter) {
			if (IVM_SLOT_TABLE_ITER_GET_KEY(siter)) {
				// IVM_TRACE("copied slot: %s\n", ivm_string_trimHead(IVM_SLOT_TABLE_ITER_GET_KEY(siter)));
				IVM_SLOT_TABLE_ITER_SET_VAL(siter,
											ivm_collector_copyObject(IVM_SLOT_TABLE_ITER_GET_VAL(siter),
																	 arg));
			}
		}
	}

	IVM_OBJECT_SET(ret, PROTO, ivm_collector_copyObject(IVM_OBJECT_GET(ret, PROTO), arg));

	trav = IVM_OBJECT_GET(obj, TYPE_TRAV);
	if (trav) {
		trav(ret, arg);
	}

	return ret;
}

IVM_PRIVATE
IVM_INLINE
void
ivm_collector_updateObject(ivm_object_t **obj,
						   ivm_traverser_arg_t *arg)
{
	*obj = ivm_collector_copyObject(*obj, arg);
	return;
}

IVM_PRIVATE
IVM_INLINE
void
ivm_collector_travContextChain(ivm_ctchain_t *chain,
							   ivm_traverser_arg_t *arg)
{
	ivm_ctchain_iterator_t iter;


	if (chain) {
		IVM_CTCHAIN_EACHPTR(chain, iter) {
			IVM_CTCHAIN_ITER_SET(iter,
								 ivm_collector_copyObject(IVM_CTCHAIN_ITER_GET(iter), arg));

		}
	}

	return;
}

IVM_PRIVATE
IVM_INLINE
void
ivm_collector_travFrame(ivm_frame_t *frame,
						ivm_traverser_arg_t *arg)
{
	if (frame) {
		ivm_collector_travContextChain(IVM_FRAME_GET(frame, CONTEXT), arg);
	}

	return;
}

IVM_PRIVATE
IVM_INLINE
void
ivm_collector_travRuntime(ivm_runtime_t *runtime,
						  ivm_traverser_arg_t *arg)
{
	if (runtime) {
		ivm_collector_travContextChain(IVM_RUNTIME_GET(runtime, CONTEXT), arg);
	}

	return;
}

IVM_PRIVATE
IVM_INLINE
void
ivm_collector_travCoro(ivm_coro_t *coro,
					   ivm_traverser_arg_t *arg)
{
	ivm_vmstack_t *stack = IVM_CORO_GET(coro, STACK);
	ivm_frame_stack_t *frame_st = IVM_CORO_GET(coro, FRAME_STACK);
	ivm_runtime_t *runtime = IVM_CORO_GET(coro, RUNTIME);
	ivm_frame_stack_iterator_t fiter;
	ivm_object_t **i, **sp;

	if (runtime) {
		for (i = ivm_vmstack_bottom(stack),
			 sp = IVM_RUNTIME_GET(runtime, SP);
			 i != sp; i++) {
			ivm_collector_updateObject(i, arg);
		}
	}

	IVM_FRAME_STACK_EACHPTR(frame_st, fiter) {
		ivm_collector_travFrame(IVM_FRAME_STACK_ITER_GET(fiter), arg);
	}

	ivm_collector_travRuntime(runtime, arg);

	return;
}

IVM_PRIVATE
IVM_INLINE
void
ivm_collector_travType(ivm_type_t *type,
					   ivm_traverser_arg_t *arg)
{
	ivm_type_setProto(type,
					  ivm_collector_copyObject(ivm_type_getProto(type),
					  						   arg));
	return;
}

IVM_PRIVATE
IVM_INLINE
void
ivm_collector_travState(ivm_traverser_arg_t *arg)
{
	ivm_coro_list_t *coros = IVM_VMSTATE_GET(arg->state, CORO_LIST);
	ivm_type_list_t *types = IVM_VMSTATE_GET(arg->state, TYPE_LIST);
	ivm_coro_t *tmp_coro, *cur_coro;
	ivm_coro_list_iterator_t citer, cbegin;
	ivm_type_list_iterator_t titer;

	cur_coro = IVM_VMSTATE_GET(arg->state, CUR_CORO);

	cbegin = IVM_CORO_LIST_ITER_BEGIN(coros);
	IVM_CORO_LIST_EACHPTR(coros, citer) {
		tmp_coro = IVM_CORO_LIST_ITER_GET(citer);
		if (ivm_coro_isAsleep(tmp_coro)) {
			ivm_collector_travCoro(tmp_coro, arg);
			IVM_CORO_LIST_ITER_SET(cbegin, tmp_coro);

			if (tmp_coro == cur_coro) {
				IVM_VMSTATE_SET(
					arg->state,
					CUR_CORO,
					IVM_CORO_LIST_ITER_INDEX(coros, cbegin)
				);
			}

			cbegin++;
		} else {
			// assert tmp_coro != cur_coro
			ivm_coro_free(tmp_coro, arg->state);
		}
	}
	ivm_coro_list_setSize(coros, IVM_CORO_LIST_ITER_INDEX(coros, cbegin));
	// IVM_TRACE("remain coro: %ld\n", IVM_CORO_LIST_ITER_INDEX(coros, cbegin));

	IVM_TYPE_LIST_EACHPTR(types, titer) {
		ivm_collector_travType(IVM_TYPE_LIST_ITER_GET(titer),
							   arg);
	}

	return;
}

#define ADD_EMPTY_LIST(collector, obj) \
	(ivm_destruct_list_add((collector)->des_log[1], (obj)))

IVM_PRIVATE
IVM_INLINE
void
ivm_collector_checkIfDestruct(ivm_collector_t *collector,
							  ivm_object_t *obj,
							  ivm_vmstate_t *state)
{
	if (!IVM_OBJECT_GET(obj, COPY)) {
		ivm_object_destruct(obj, state);
	} else {
		/* copy to another log */
		ADD_EMPTY_LIST(collector, IVM_OBJECT_GET(obj, COPY));
	}

	return;
}

IVM_PRIVATE
IVM_INLINE
void
ivm_collector_triggerDestructor(ivm_collector_t *collector,
								ivm_vmstate_t *state)
{
	ivm_destruct_list_t *tmp;
	ivm_destruct_list_iterator_t iter;

	ivm_destruct_list_empty(collector->des_log[1]);

	IVM_DESTRUCT_LIST_EACHPTR(collector->des_log[0], iter) {
		ivm_collector_checkIfDestruct(collector,
									  IVM_DESTRUCT_LIST_ITER_GET(iter),
									  state);
	}

	tmp = collector->des_log[0];
	collector->des_log[0] = collector->des_log[1];
	collector->des_log[1] = tmp;

	return;
}

#if IVM_USE_PERF_PROFILE

clock_t ivm_perf_gc_time = 0;
ivm_size_t ivm_perf_gc_count = 0;

#endif

void
ivm_collector_collect(ivm_collector_t *collector,
					  ivm_vmstate_t *state,
					  ivm_heap_t *heap)
{

#if IVM_USE_PERF_PROFILE
	clock_t time_st = clock();
#endif

	ivm_traverser_arg_t arg;

	arg.state = state;
	arg.heap = IVM_VMSTATE_GET(state, EMPTY_HEAP);
	arg.collector = collector;
	arg.trav_ctchain = ivm_collector_travContextChain;

	ivm_heap_reset(arg.heap);

	// IVM_TRACE("***collecting***\n");

	ivm_collector_travState(&arg);
	ivm_collector_triggerDestructor(collector, state);
	ivm_vmstate_swapHeap(state);
	// ivm_heap_compact(IVM_VMSTATE_GET(state, CUR_HEAP));

	ivm_vmstate_closeGCFlag(state);

#if IVM_USE_PERF_PROFILE
	ivm_perf_gc_time += clock() - time_st;
	ivm_perf_gc_count++;
#endif

	return;
}
