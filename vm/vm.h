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

#define IVM_DEFAULT_INIT_HEAP_SIZE (512)
#define IVM_CHECK_STATE_NULL (IVM_CHECK_BASE_NULL)

typedef struct ivm_vmstate_t_tag {
	ivm_size_t heap_count;
	ivm_heap_t **heaps;

	ivm_size_t cur_coro;
	ivm_coro_list_t *coro_list;

	ivm_exec_list_t *exec_list; /* executable list: used for function object creating */
	ivm_type_list_t *type_list;

	ivm_bool_t gc_flag;
	ivm_collector_t *gc;
} ivm_vmstate_t;

#define IVM_VMSTATE_GC_FLAG(state) ((state)->gc_flag)
#define IVM_VMSTATE_HEAP_COUNT(state) ((state)->heap_count)
#define IVM_VMSTATE_HEAP(state) ((state)->heaps)
#define IVM_VMSTATE_CORO_LIST(state) ((state)->coro_list)

ivm_vmstate_t *
ivm_vmstate_new();
void
ivm_vmstate_free(ivm_vmstate_t *state);
void
ivm_vmstate_addHeap(ivm_vmstate_t *state);
#define ivm_vmstate_curHeap(state) ((state)->heap_count > 0 ? (state)->heaps[(state)->heap_count - 1] : IVM_NULL)
#define ivm_vmstate_openGCFlag(state) ((state)->gc_flag = IVM_TRUE)
#define ivm_vmstate_closeGCFlag(state) ((state)->gc_flag = IVM_FALSE)
ivm_heap_t *
ivm_vmstate_findHeap(ivm_vmstate_t *state, ivm_object_t *obj);

ivm_object_t *
ivm_vmstate_alloc(ivm_vmstate_t *state);
ivm_object_t *
ivm_vmstate_newObject(ivm_vmstate_t *state);
void
ivm_vmstate_freeObject(ivm_vmstate_t *state, ivm_object_t *obj);

#define ivm_vmstate_registerExec(state, exec) (ivm_exec_list_register((state)->exec_list, (exec)))
#define ivm_vmstate_getExec(state, id) (ivm_exec_list_at((state)->exec_list, (id)))

#define ivm_vmstate_registerType(state, type) (ivm_type_list_register((state)->type_list, (type)))
#define ivm_vmstate_getType(state, tag) (ivm_type_list_at((state)->type_list, (tag)))

#ifdef IVM_DEBUG

#define ivm_vmstate_checkGC(state) (ivm_collector_collect((state)->gc))

#else

#define ivm_vmstate_checkGC(state) \
	if ((state)->gc_flag) { \
		ivm_collector_collect((state)->gc); \
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
