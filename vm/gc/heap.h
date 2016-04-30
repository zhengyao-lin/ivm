#ifndef _IVM_VM_GC_HEAP_H_
#define _IVM_VM_GC_HEAP_H_

#include "../obj.h"
#include "../type.h"
#include "cell.h"

struct ivm_vmstate_t_tag;

/* old heap */
#if 0

typedef struct {
	ivm_size_t size;
	ivm_size_t empty_size;
	void *pre;
	ivm_cell_set_t *empty;
} ivm_heap_t;

ivm_heap_t *
ivm_heap_new(ivm_size_t obj_count);
void
ivm_heap_free(ivm_heap_t *heap);
ivm_object_t *
ivm_heap_alloc(ivm_heap_t *heap);

void
ivm_heap_freeObject(ivm_heap_t *heap, struct ivm_vmstate_t_tag *state, ivm_object_t *obj);
ivm_bool_t
ivm_heap_isInHeap(ivm_heap_t *heap, ivm_object_t *obj);

#endif

typedef struct ivm_heap_t_tag {
	ivm_size_t bcount;
	ivm_size_t bsize;
	ivm_size_t *curs;
	ivm_byte_t **blocks;
} ivm_heap_t;

#define IVM_HEAP_BSIZE(heap) ((heap)->bsize)

ivm_heap_t *
ivm_heap_new(ivm_size_t bsize);
void
ivm_heap_free(ivm_heap_t *heap);
void *
ivm_heap_alloc(ivm_heap_t *heap, ivm_size_t size);
void *
ivm_heap_addCopy(ivm_heap_t *heap, void *ptr, ivm_size_t size);
ivm_size_t /* return: the possible block index + 1 */
ivm_heap_hasSize(ivm_heap_t *heap, ivm_size_t size);
void
ivm_heap_reset(ivm_heap_t *heap);

#endif
