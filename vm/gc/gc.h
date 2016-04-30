#ifndef _IVM_VM_GC_GC_H_
#define _IVM_VM_GC_GC_H_

#include "cell.h"
#include "heap.h"
#include "../type.h"

struct ivm_vmstate_t_tag;

#if 0

typedef ivm_sint64_t ivm_mark_period_t;

typedef struct ivm_collector_t_tag {
	ivm_size_t obj_count;
	ivm_cell_set_t *obj_set;
	ivm_mark_period_t period;
	struct ivm_vmstate_t_tag *state;
} ivm_collector_t;

#define IVM_COLLECTOR_OBJ_SET(collector) ((collector)->obj_set)
#define IVM_COLLECTOR_PERIOD(collector) ((collector)->period)
#define IVM_COLLECTOR_STATE(collector) ((collector)->state)

#define IVM_MARK_WHITE 0
/*
#define IVM_MARK_GREY 1
#define IVM_MARK_BLACK 2
*/

ivm_collector_t *
ivm_collector_new(struct ivm_vmstate_t_tag *state);

/* just clean the container */
void
ivm_collector_free(ivm_collector_t *collector);

/* clean every object it contains */
void
ivm_collector_dispose(ivm_collector_t *collector,
					  struct ivm_vmstate_t_tag *state);

#define ivm_collector_addObject(collector, obj) \
	(ivm_cell_set_addObject((collector)->obj_set, (obj)), \
	 (collector)->obj_count++)

void
ivm_collector_markState(ivm_collector_t *collector,
						struct ivm_vmstate_t_tag *state);

#define ivm_collector_incPeriod(collector) ((collector)->period++)
void
ivm_collector_collect(ivm_collector_t *collector);

#endif

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
					  struct ivm_vmstate_t_tag *root,
					  ivm_heap_t *heap);

#endif
