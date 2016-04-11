#ifndef _IVM_VM_VM_H_
#define _IVM_VM_VM_H_

#include "pub/const.h"
#include "obj.h"
#include "num.h"
#include "op.h"
#include "gc/heap.h"

#define IVM_DEFAULT_INIT_HEAP_SIZE (512)
#define IVM_CHECK_STATE_NULL (IVM_CHECK_BASE_NULL)

typedef struct ivm_vmstate_t_tag {
	ivm_bool_t gc_flag;
	ivm_size_t heap_count;
	ivm_heap_t **heaps;
} ivm_vmstate_t;

#define VMSTATE_GC_FLAG(state) ((state)->gc_flag)
#define VMSTATE_HEAP_COUNT(state) ((state)->heap_count)
#define VMSTATE_HEAP(state) ((state)->heaps)

ivm_vmstate_t *
ivm_vmstate_new();
void
ivm_vmstate_free(ivm_vmstate_t *state);
void
ivm_vmstate_addHeap(ivm_vmstate_t *state);
#define ivm_vmstate_curHeap(state) ((state)->heap_count > 0 ? (state)->heaps[(state)->heap_count - 1] : IVM_NULL)
#define ivm_vmstate_markForGC(state) ((state)->gc_flag = IVM_TRUE)
#define ivm_vmstate_cleanGCMark(state) ((state)->gc_flag = IVM_FALSE)
ivm_heap_t *
ivm_vmstate_findHeap(ivm_vmstate_t *state, ivm_object_t *obj);

ivm_object_t *
ivm_vmstate_alloc(ivm_vmstate_t *state);
ivm_object_t *
ivm_vmstate_newObject(ivm_vmstate_t *state);
void
ivm_vmstate_freeObject(ivm_vmstate_t *state, ivm_object_t *obj);

/*
#define ivm_vmstate_alloc(state) ((state) ? ivm_heap_alloc((state)->heaps) : IVM_NULL)
#define ivm_vmstate_freeObject(state, obj) \
	if (state) { \
		ivm_heap_freeObject((state), (state)->heaps, (obj)); \
	}
*/

#endif
