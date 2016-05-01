#ifndef _IVM_VM_GC_GC_H_
#define _IVM_VM_GC_GC_H_

#include "cell.h"
#include "heap.h"
#include "../type.h"

struct ivm_vmstate_t_tag;

typedef ivm_sint64_t ivm_mark_period_t;

typedef struct ivm_collector_t_tag {
	ivm_mark_period_t period;
} ivm_collector_t;

#define IVM_COLLECTOR_PERIOD(collector) ((collector)->period)
#define IVM_MARK_WHITE 0

typedef struct ivm_traverser_arg_t_tag {
	struct ivm_vmstate_t_tag *state;
	ivm_heap_t *heap;
	ivm_collector_t *collector;
} ivm_traverser_arg_t;

ivm_collector_t *
ivm_collector_new();
void
ivm_collector_free(ivm_collector_t *collector);

void
ivm_collector_collect(ivm_collector_t *collector,
					  struct ivm_vmstate_t_tag *state,
					  ivm_heap_t *heap);

#endif
