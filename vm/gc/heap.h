#ifndef _IVM_VM_GC_HEAP_H_
#define _IVM_VM_GC_HEAP_H_

#include "../obj.h"
#include "../type.h"
#include "cell.h"

struct ivm_vmstate_t_tag;

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
void
ivm_heap_compact(ivm_heap_t *heap);

#endif
