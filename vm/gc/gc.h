#ifndef _IVM_VM_GC_GC_H_
#define _IVM_VM_GC_GC_H_

#include "pub/com.h"
#include "pub/type.h"

#include "std/list.h"
#include "std/heap.h"

IVM_COM_HEADER

#define IVM_DEFAULT_DESTRUCT_LIST_BUFFER_SIZE 128

struct ivm_vmstate_t_tag;
struct ivm_ctchain_t_tag;
struct ivm_object_t_tag;

/* typedef ivm_mark_t ivm_mark_period_t; */

typedef ivm_ptlist_t ivm_destruct_list_t;
typedef IVM_PTLIST_ITER_TYPE(struct ivm_object_t_tag *) ivm_destruct_list_iterator_t;

#define ivm_destruct_list_new() (ivm_ptlist_new_c(IVM_DEFAULT_DESTRUCT_LIST_BUFFER_SIZE))
#define ivm_destruct_list_free ivm_ptlist_free
#define ivm_destruct_list_add ivm_ptlist_push
#define ivm_destruct_list_empty ivm_ptlist_empty
#define ivm_destruct_list_at(list, i) ((struct ivm_object_t_tag *)ivm_ptlist_at((list), (i)))

#define IVM_DESTRUCT_LIST_ITER_SET(iter, val) (IVM_PTLIST_ITER_SET((iter), (val)))
#define IVM_DESTRUCT_LIST_ITER_GET(iter) ((struct ivm_object_t_tag *)IVM_PTLIST_ITER_GET(iter))
#define IVM_DESTRUCT_LIST_EACHPTR(list, iter) IVM_PTLIST_EACHPTR((list), iter, struct ivm_object_t_tag *)

typedef struct ivm_collector_t_tag {
	ivm_destruct_list_t *des_log[2];
} ivm_collector_t;

#define IVM_MARK_WHITE 0

#define IVM_COLLECTOR_GET_PERIOD(collector) ((collector)->period)
#define IVM_COLLECTOR_GET_CUR_DES_LOG(collector) ((collector)->des_log[0])
#define IVM_COLLECTOR_GET_EMPTY_DES_LOG(collector) ((collector)->des_log[1])

#define IVM_COLLECTOR_GET(obj, member) IVM_GET((obj), IVM_COLLECTOR, member)
#define IVM_COLLECTOR_SET(obj, member, val) IVM_SET((obj), IVM_COLLECTOR, member, (val))

typedef struct ivm_traverser_arg_t_tag {
	struct ivm_vmstate_t_tag *state;
	ivm_heap_t *heap;
	ivm_collector_t *collector;
	void (*trav_ctchain)(struct ivm_ctchain_t_tag *chain,
						 struct ivm_traverser_arg_t_tag *arg);
} ivm_traverser_arg_t;

ivm_collector_t *
ivm_collector_new();

void
ivm_collector_free(ivm_collector_t *collector, struct ivm_vmstate_t_tag *state);

IVM_INLINE
void
ivm_collector_reinit(ivm_collector_t *collector)
{
	ivm_destruct_list_empty(collector->des_log[0]);
	ivm_destruct_list_empty(collector->des_log[1]);
	return;
}

/* only objects in destructor log will be 'informed' when being destructed */
#define ivm_collector_addDesLog(collector, obj) (ivm_destruct_list_add((collector)->des_log[0], (obj)))

void
ivm_collector_collect(ivm_collector_t *collector,
					  struct ivm_vmstate_t_tag *state,
					  ivm_heap_t *heap);

IVM_COM_END

#endif
