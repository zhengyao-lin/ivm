#ifndef _IVM_VM_GC_HEAP_H_
#define _IVM_VM_GC_HEAP_H_

#include "../obj.h"
#include "../type.h"
#include "cell.h"

struct ivm_vmstate_t_tag;

typedef struct {
	ivm_size_t size;
	ivm_size_t empty_size;
	void *pre;
	ivm_cell_set_t *empty;
} ivm_heap_t;

/* heap */
ivm_heap_t *
ivm_new_heap(ivm_size_t obj_count);
void
ivm_free_heap(ivm_heap_t *heap);
ivm_object_t *
ivm_heap_alloc(ivm_heap_t *heap);
ivm_object_t *
ivm_heap_new_object(struct ivm_vmstate_t_tag *state, ivm_heap_t *heap);
void
ivm_heap_free_object(struct ivm_vmstate_t_tag *state, ivm_heap_t *heap, ivm_object_t *obj);

#endif
