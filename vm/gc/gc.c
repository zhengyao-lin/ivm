#include "pub/mem.h"
#include "pub/const.h"
#include "pub/com.h"
#include "pub/err.h"
#include "pub/vm.h"
#include "pub/inlines.h"

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

	ret->live_ratio = 0;
	ret->skip_time = 0;
	ret->bc_weight = IVM_DEFAULT_GC_BC_WEIGHT;
	
	ivm_destruct_list_init(&ret->des_log[0]);
	ivm_destruct_list_init(&ret->des_log[1]);
	
	ivm_wbobj_list_init(&ret->wb_obj);
	ivm_wbslot_list_init(&ret->wb_slot);
	ivm_wbctx_list_init(&ret->wb_ctx);
	
	ret->gen = 0;

	return ret;
}

#define ADD_EMPTY_LIST(collector, obj) \
	(ivm_destruct_list_add(&(collector)->des_log[1], (obj)))

IVM_PRIVATE
IVM_INLINE
void
ivm_collector_checkIfDestruct(ivm_collector_t *collector,
							  ivm_object_t *obj,
							  ivm_vmstate_t *state,
							  ivm_traverser_arg_t *arg)
{
	// IVM_TRACE("gen: %d\n", IVM_OBJECT_GET(obj, GEN));
	if (!IVM_OBJECT_GET(obj, COPY)) {
		if (IVM_OBJECT_GET(obj, GEN) <= arg->gen) {
			ivm_object_destruct(obj, state);
		} else {
			ADD_EMPTY_LIST(collector, obj);
		}
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
								ivm_vmstate_t *state,
								ivm_traverser_arg_t *arg)
{
	ivm_destruct_list_t tmp;
	ivm_destruct_list_iterator_t iter;

	ivm_destruct_list_empty(&collector->des_log[1]);

	IVM_DESTRUCT_LIST_EACHPTR(&collector->des_log[0], iter) {
		ivm_collector_checkIfDestruct(collector,
									  IVM_DESTRUCT_LIST_ITER_GET(iter),
									  state, arg);
	}

	tmp = collector->des_log[0];
	collector->des_log[0] = collector->des_log[1];
	collector->des_log[1] = tmp;

	return;
}

IVM_PRIVATE
IVM_INLINE
void
ivm_collector_triggerAllDestructor(ivm_collector_t *collector,
								   ivm_vmstate_t *state)
{
	ivm_destruct_list_iterator_t iter;

	IVM_DESTRUCT_LIST_EACHPTR(&collector->des_log[0], iter) {
		ivm_object_destruct(IVM_DESTRUCT_LIST_ITER_GET(iter), state);
	}

	return;
}

void
ivm_collector_free(ivm_collector_t *collector, ivm_vmstate_t *state)
{
	if (collector) {
		ivm_collector_triggerAllDestructor(collector, state);
		
		ivm_destruct_list_dump(&collector->des_log[0]);
		ivm_destruct_list_dump(&collector->des_log[1]);

		ivm_wbobj_list_dump(&collector->wb_obj);
		ivm_wbslot_list_dump(&collector->wb_slot);
		ivm_wbctx_list_dump(&collector->wb_ctx);
		
		MEM_FREE(collector);
	}

	return;
}

IVM_PRIVATE
ivm_slot_table_t *
ivm_collector_copySlotTable(ivm_slot_table_t *table,
							ivm_traverser_arg_t *arg)
{
	ivm_slot_table_t *ret = IVM_NULL;
	ivm_slot_table_iterator_t siter;
	ivm_object_t **oops, **end;
	ivm_int_t gen;
	ivm_int_t count;

	if (table) {
		gen = ivm_slot_table_getGen(table);

		if (gen > arg->gen) {
			return table;
		}

		gen++;

		// IVM_TRACE("gen %d\n", arg->gen);

		ret = ivm_slot_table_getCopy(table);
		if (ret) return ret;
		else if (ivm_heap_isIn(arg->heap, table)) {
			return table;
		}
	} else {
		return IVM_NULL;
	}

	ret = ivm_slot_table_copy(table, arg->state, arg->heap);

	if (gen < 2)
		ivm_slot_table_setGen(ret, gen);
	ivm_slot_table_setWB(table, 0);
	ivm_slot_table_setCopy(table, ret);
	// ivm_slot_table_updateUID(table, arg->state);

	// IVM_TRACE("pa %p -> %p\n", table, ret);

	IVM_SLOT_TABLE_EACHPTR(ret, siter) {
		if (IVM_SLOT_TABLE_ITER_GET_KEY(siter)) {
			// IVM_TRACE("  copied slot: %s\n", ivm_string_trimHead(IVM_SLOT_TABLE_ITER_GET_KEY(siter)));
			IVM_SLOT_TABLE_ITER_SET_VAL(siter,
										ivm_collector_copyObject(IVM_SLOT_TABLE_ITER_GET_VAL(siter),
																 arg));
		}
	}

	oops = ivm_slot_table_getOops(ret);
	if (oops) {
		count = ivm_slot_table_getOopCount(ret);
		for (end = oops + count;
			 oops != end; oops++, count--) {
			*oops = ivm_collector_copyObject(*oops, arg);
		}
	}

	// IVM_TRACE("end %p -> %p\n", table, ret);

	return ret;
}

IVM_PRIVATE
ivm_slot_table_t *
ivm_collector_copySlotTable_ng(ivm_slot_table_t *table,
							   ivm_traverser_arg_t *arg)
{
	ivm_object_t *tmp;
	ivm_slot_table_iterator_t siter;
	ivm_object_t **oops, **end;
	ivm_int_t count;
	// ivm_int_t gen;

	// IVM_TRACE("hey there? %p\n", table);

	ivm_slot_table_setWB(table, 0);
	// ivm_slot_table_updateUID(table, arg->state);

	IVM_SLOT_TABLE_EACHPTR(table, siter) {
		if (IVM_SLOT_TABLE_ITER_GET_KEY(siter)) {
			// IVM_TRACE("  copied slot: %s: %p -> ",
			//		  ivm_string_trimHead(IVM_SLOT_TABLE_ITER_GET_KEY(siter)),
			//		  IVM_SLOT_TABLE_ITER_GET_VAL(siter));
			IVM_SLOT_TABLE_ITER_SET_VAL(siter,
										(tmp = ivm_collector_copyObject(IVM_SLOT_TABLE_ITER_GET_VAL(siter), arg)));
			//IVM_TRACE("%p\n", tmp);
		}
	}

	oops = ivm_slot_table_getOops(table);
	if (oops) {
		count = ivm_slot_table_getOopCount(table);
		for (end = oops + count;
			 oops != end; oops++) {
			*oops = ivm_collector_copyObject(*oops, arg);
		}
	}

	return table;
}

ivm_object_t *
ivm_collector_copyObject_c(ivm_object_t *obj,
						   ivm_traverser_arg_t *arg)
{
	ivm_object_t *ret;
	ivm_traverser_t trav;
	ivm_int_t gen = IVM_OBJECT_GET(obj, GEN) + 1;

	ret = ivm_heap_addCopy(arg->heap, obj, IVM_OBJECT_GET(obj, TYPE_SIZE));

	IVM_OBJECT_SET(ret, COPY, IVM_NULL); /* remove the new object's copy */
	if (gen < 2)
		IVM_OBJECT_SET(ret, GEN, gen);
	IVM_OBJECT_SET(ret, WB, 0);
	IVM_OBJECT_SET(obj, COPY, ret);

	IVM_OBJECT_SET(
		ret, SLOTS,
		ivm_collector_copySlotTable(IVM_OBJECT_GET(ret, SLOTS), arg)
	);

	// IVM_OBJECT_SET(ret, PROTO, ivm_collector_copyObject(IVM_OBJECT_GET(ret, PROTO), arg));
	ivm_object_setProto(
		ret, arg->state,
		ivm_collector_copyObject(ivm_object_getProto(ret), arg)
	);

	trav = IVM_OBJECT_GET(obj, TYPE_TRAV);
	if (trav) {
		trav(ret, arg);
	}

	return ret;
}

IVM_PRIVATE
IVM_INLINE
ivm_object_t *
ivm_collector_copyObject_ng(ivm_object_t *obj,
							ivm_traverser_arg_t *arg)
{
	ivm_traverser_t trav;
	// ivm_slot_table_t *tmp;
	// ivm_int_t gen = IVM_OBJECT_GET(obj, GEN);

	IVM_OBJECT_SET(obj, WB, 0);

	IVM_OBJECT_SET(
		obj, SLOTS,
		ivm_collector_copySlotTable(IVM_OBJECT_GET(obj, SLOTS), arg)
	);

	ivm_object_setProto(
		obj, arg->state,
		ivm_collector_copyObject(ivm_object_getProto(obj), arg)
	);

	trav = IVM_OBJECT_GET(obj, TYPE_TRAV);
	if (trav) {
		trav(obj, arg);
	}

	return obj;
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
		if (ivm_ctchain_getGen(chain) > arg->gen &&
			!ivm_ctchain_getWB(chain))
			return;

		if (!ivm_ctchain_getGen(chain))
			ivm_ctchain_setGen(chain, 1);
			
		ivm_ctchain_setWB(chain, 0);
		IVM_CTCHAIN_EACHPTR(chain, iter) {
			IVM_CTCHAIN_ITER_SET(
				iter,
				ivm_collector_copySlotTable(
					IVM_CTCHAIN_ITER_GET(iter),
					arg
				)
			);
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
	ivm_type_t *types = IVM_VMSTATE_GET(arg->state, TYPE_LIST), *end;
	ivm_coro_t *tmp_coro, *cur_coro;
	ivm_coro_list_iterator_t citer, cbegin;
	ivm_bool_t compacted = IVM_FALSE;

	cur_coro = IVM_VMSTATE_GET(arg->state, CUR_CORO);

	cbegin = IVM_CORO_LIST_ITER_BEGIN(coros);
	IVM_CORO_LIST_EACHPTR(coros, citer) {
		tmp_coro = IVM_CORO_LIST_ITER_GET(citer);
		if (ivm_coro_isAlive(tmp_coro)) {
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
			compacted = IVM_TRUE;
		}
	}
	ivm_coro_list_setSize(coros, IVM_CORO_LIST_ITER_INDEX(coros, cbegin));
	// IVM_TRACE("remain coro: %ld\n", IVM_CORO_LIST_ITER_INDEX(coros, cbegin));
	if (compacted) {
		ivm_vmstate_coroCompacted(arg->state);
	}

	for (end = types + IVM_TYPE_COUNT;
		 types != end; types++) {
		ivm_collector_travType(types,
							   arg);
	}

	return;
}

IVM_PRIVATE
IVM_INLINE
void
ivm_collector_checkWriteBarrier(ivm_collector_t *collector,
								ivm_traverser_arg_t *arg)
{
	ivm_wbobj_list_iterator_t oiter;
	ivm_wbslot_list_iterator_t siter;
	ivm_wbctx_list_iterator_t citer;

	if (!arg->gen) {
		IVM_WBOBJ_LIST_EACHPTR(&collector->wb_obj, oiter) {
			ivm_collector_copyObject_ng(IVM_WBOBJ_LIST_ITER_GET(oiter), arg);
		}
	}

	ivm_wbobj_list_empty(&collector->wb_obj);

	if (!arg->gen) {
		IVM_WBSLOT_LIST_EACHPTR(&collector->wb_slot, siter) {
			ivm_collector_copySlotTable_ng(IVM_WBSLOT_LIST_ITER_GET(siter), arg);
		}
	}

	ivm_wbslot_list_empty(&collector->wb_slot);

	if (!arg->gen) {
		IVM_WBCTX_LIST_EACHPTR(&collector->wb_ctx, citer) {
			// IVM_TRACE("catch! %p\n", IVM_WBCTX_LIST_ITER_GET(citer));
			ivm_collector_travContextChain(IVM_WBCTX_LIST_ITER_GET(citer), arg);
		}
	}

	ivm_wbctx_list_empty(&collector->wb_ctx);

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
	ivm_int_t orig = 0;
	ivm_heap_t *heap1 = ivm_vmstate_getHeapAt(state, 0);
	ivm_heap_t *heap2 = ivm_vmstate_getHeapAt(state, 1);
	ivm_heap_t *swap = ivm_vmstate_getHeapAt(state, 2);
	ivm_int_t tmp_ratio;
	// ivm_size_t heap2_orig = IVM_HEAP_GET(heap2, BLOCK_USED);

	arg.state = state;
	arg.collector = collector;
	arg.trav_ctchain = ivm_collector_travContextChain;
	arg.gen = collector->gen;

	// IVM_TRACE("gen: %d, live ratio: %d, %.20f\n", arg.gen, collector->live_ratio, collector->bc_weight);

	if (collector->live_ratio > IVM_DEFAULT_GC_MAX_LIVE_RATIO &&
		collector->skip_time < IVM_DEFAULT_GC_MAX_SKIP) {
		// IVM_TRACE("skip! %d\n", collector->live_ratio);
		collector->live_ratio -= IVM_HEAP_GET(heap2, BLOCK_TOP) * collector->bc_weight;
		collector->skip_time++;
		return;
	}

	collector->skip_time = 0;

	if (arg.gen) {
		// full gc
		// second gen need gc too -> all copy to swap heap
		arg.heap = swap;
		ivm_heap_reset(swap);
	} else {
		// partial gc
		arg.heap = heap2;
		orig = IVM_HEAP_GET(heap2, BLOCK_TOP);
	}

	// IVM_TRACE("***collecting*** %d\n", arg.gen);

	ivm_collector_checkWriteBarrier(collector, &arg);
	ivm_collector_travState(&arg);

	if (arg.gen) {
		tmp_ratio = IVM_HEAP_GET(swap, BLOCK_USED) * 100 /
					(IVM_HEAP_GET(heap1, BLOCK_USED) + IVM_HEAP_GET(heap2, BLOCK_USED));
		//collector->live_ratio = tmp_ratio;
		// IVM_TRACE("1, live ratio: %d\n", tmp_ratio);
		collector->live_ratio = tmp_ratio;

		if (tmp_ratio > IVM_DEFAULT_GC_MAX_LIVE_RATIO) {
			collector->bc_weight /= tmp_ratio;
		} else collector->bc_weight = IVM_DEFAULT_GC_BC_WEIGHT;
	}

	//
	// IVM_TRACE("live ratio: %ld %ld %d\n", IVM_HEAP_GET(arg.heap, BLOCK_USED), IVM_HEAP_GET(heap, BLOCK_USED), collector->live_ratio);

	ivm_collector_triggerDestructor(collector, state, &arg);

	// ivm_heap_compact(IVM_VMSTATE_GET(state, CUR_HEAP));
	ivm_heap_reset(ivm_vmstate_getHeapAt(state, 0));

	if (arg.gen) {
		ivm_heap_reset(ivm_vmstate_getHeapAt(state, 1));
		ivm_vmstate_swapHeap(state, 1, 2);
		collector->gen = 0;
	} else if (IVM_HEAP_GET(heap2, BLOCK_TOP) > orig) {
		// IVM_TRACE("hi\n");
		IVM_TRACE("heap2 is full\n");
		collector->gen = 1;
	}

	ivm_vmstate_closeGCFlag(state);

#if IVM_USE_PERF_PROFILE
	ivm_perf_gc_time += clock() - time_st;
	ivm_perf_gc_count++;
#endif

	return;
}
