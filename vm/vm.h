#ifndef _IVM_VM_VM_H_
#define _IVM_VM_VM_H_

#include "pub/const.h"
#include "obj.h"
#include "num.h"
#include "op.h"
#include "coro.h"
#include "context.h"
#include "gc/heap.h"
#include "gc/gc.h"

#define IVM_DEFAULT_INIT_HEAP_SIZE (2 << 22)
#define IVM_CHECK_STATE_NULL (IVM_CHECK_BASE_NULL)

typedef struct ivm_vmstate_t_tag {
	ivm_heap_t *heaps[2];

	ivm_size_t cur_coro;
	ivm_coro_list_t *coro_list;

	ivm_exec_list_t *exec_list; /* executable list: used for function object creating */
	ivm_type_list_t *type_list;

	ivm_int_t gc_flag; /* gc flag:
						  > 0: open
						  = 0: closed
						  < 0: locked */
	ivm_collector_t *gc;
} ivm_vmstate_t;

#define IVM_VMSTATE_GET_CUR_CORO(state) (ivm_coro_list_at((state)->coro_list, (state)->cur_coro))
#define IVM_VMSTATE_GET_CORO_LIST(state) ((state)->coro_list)
#define IVM_VMSTATE_GET_CUR_HEAP(state) ((state)->heaps[0])
#define IVM_VMSTATE_GET_EMPTY_HEAP(state) ((state)->heaps[1])

#define IVM_VMSTATE_GET(obj, member) IVM_GET((obj), IVM_VMSTATE, member)
#define IVM_VMSTATE_SET(obj, member, val) IVM_SET((obj), IVM_VMSTATE, member, (val))

ivm_vmstate_t *
ivm_vmstate_new();
void
ivm_vmstate_free(ivm_vmstate_t *state);

#define ivm_vmstate_isGCFlagOpen(state) ((state)->gc_flag > 0)

#define ivm_vmstate_openGCFlag(state) ((state)->gc_flag < 0 ? 0 : ((state)->gc_flag = 1))
#define ivm_vmstate_closeGCFlag(state) ((state)->gc_flag < 0 ? 0 : ((state)->gc_flag = 0))

#define ivm_vmstate_lockGCFlag(state) ((state)->gc_flag = -1)
#define ivm_vmstate_unlockGCFlag(state) ((state)->gc_flag = 0)

#define ivm_vmstate_resetHeap(state, hp) (ivm_heap_free((state)->heap), (state)->heap = (hp))

#if 0

#define ivm_vmstate_curHeap(state) ((state)->heap_count > 0 ? (state)->heaps[(state)->heap_count - 1] : IVM_NULL)
void
ivm_vmstate_addHeap(ivm_vmstate_t *state);
ivm_heap_t *
ivm_vmstate_findHeap(ivm_vmstate_t *state, ivm_object_t *obj);

#endif

void *
ivm_vmstate_alloc(ivm_vmstate_t *state, ivm_size_t size);
void
ivm_vmstate_swapHeap(ivm_vmstate_t *state);

#if 0

ivm_object_t *
ivm_vmstate_newObject(ivm_vmstate_t *state);
void
ivm_vmstate_freeObject(ivm_vmstate_t *state, ivm_object_t *obj);

#endif

#define ivm_vmstate_registerExec(state, exec) (ivm_exec_list_register((state)->exec_list, (exec)))
#define ivm_vmstate_getExec(state, id) (ivm_exec_list_at((state)->exec_list, (id)))

#define ivm_vmstate_registerType(state, type) (ivm_type_list_register((state)->type_list, (type)))
#define ivm_vmstate_getType(state, tag) (ivm_type_list_at((state)->type_list, (tag)))

#define ivm_vmstate_addDesLog(state, obj) (ivm_collector_addDesLog((state)->gc, (obj)))

#if 0 && IVM_DEBUG

#define ivm_vmstate_checkGC(state) (ivm_collector_collect((state)->gc, (state), (state)->heaps[0]))

#else

#define ivm_vmstate_checkGC(state) \
	if (ivm_vmstate_isGCFlagOpen(state)) { \
		ivm_collector_collect((state)->gc, (state), (state)->heaps[0]); \
		ivm_vmstate_closeGCFlag(state); \
	}

#endif

#define ivm_vmstate_addCoro(state, coro) (ivm_coro_list_add((state)->coro_list, (coro)))

void
ivm_vmstate_schedule(ivm_vmstate_t *state);

/*
ivm_object_t *
ivm_vmstate_execute();
*/

/*
#define ivm_vmstate_alloc(state) ((state) ? ivm_heap_alloc((state)->heaps) : IVM_NULL)
#define ivm_vmstate_freeObject(state, obj) \
	if (state) { \
		ivm_heap_freeObject((state), (state)->heaps, (obj)); \
	}
*/

#endif
