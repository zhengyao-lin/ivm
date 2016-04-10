#ifndef _IVM_VM_VM_H_
#define _IVM_VM_VM_H_

#include "obj.h"
#include "num.h"
#include "op.h"
#include "gc/heap.h"

#define DEFAULT_INIT_HEAP_SIZE (512)

typedef struct ivm_vmstate_t_tag {
	ivm_heap_t *heap;
} ivm_vmstate_t;

ivm_vmstate_t *
ivm_state_new();
void
ivm_state_free(ivm_vmstate_t *state);

#define ivm_state_alloc(state) ((state) ? ivm_heap_alloc((state)->heap) : IVM_NULL)
#define ivm_state_newObject(state) ((state) ? ivm_heap_newObject((state), (state)->heap) : IVM_NULL)
#define ivm_state_freeObject(state, obj) \
	if (state) { \
		ivm_heap_freeObject((state), (state)->heap, (obj)); \
	}

#endif
