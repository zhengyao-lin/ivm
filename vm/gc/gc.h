#ifndef _IVM_VM_GC_GC_H_
#define _IVM_VM_GC_GC_H_

#include "cell.h"
#include "heap.h"
#include "../type.h"

struct ivm_vmstate_t_tag;

typedef ivm_mark_t ivm_mark_period_t;

typedef struct ivm_collector_t_tag {
	ivm_cell_set_t *des_log;
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
ivm_collector_free(ivm_collector_t *collector, struct ivm_vmstate_t_tag *state);

/* only objects in destructor log will be 'informed' when being destructed */
#define ivm_collector_addDesLog(collector, obj) (ivm_cell_set_addObject((collector)->des_log, (obj)))

/* destruct objects in destructor log */
void
ivm_collector_triggerDestructor(ivm_collector_t *collector,
								struct ivm_vmstate_t_tag *state);

void
ivm_collector_collect(ivm_collector_t *collector,
					  struct ivm_vmstate_t_tag *state,
					  ivm_heap_t *heap);

#endif
