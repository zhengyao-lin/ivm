#ifndef _IVM_VM_GC_GC_H_
#define _IVM_VM_GC_GC_H_

#include "pub/com.h"
#include "pub/type.h"

#include "std/list.h"
#include "std/heap.h"

#include "vm/obj.h"

IVM_COM_HEADER

#define IVM_DEFAULT_DESTRUCT_LIST_BUFFER_SIZE 128

struct ivm_vmstate_t_tag;
struct ivm_ctchain_t_tag;

/* typedef ivm_mark_t ivm_mark_period_t; */

typedef ivm_ptlist_t ivm_destruct_list_t;
typedef IVM_PTLIST_ITER_TYPE(ivm_object_t *) ivm_destruct_list_iterator_t;

#define ivm_destruct_list_init(list) (ivm_ptlist_init_c((list), IVM_DEFAULT_DESTRUCT_LIST_BUFFER_SIZE))
#define ivm_destruct_list_dump ivm_ptlist_dump
#define ivm_destruct_list_add ivm_ptlist_push
#define ivm_destruct_list_empty ivm_ptlist_empty
#define ivm_destruct_list_at(list, i) ((ivm_object_t *)ivm_ptlist_at((list), (i)))

#define IVM_DESTRUCT_LIST_ITER_SET(iter, val) (IVM_PTLIST_ITER_SET((iter), (val)))
#define IVM_DESTRUCT_LIST_ITER_GET(iter) ((ivm_object_t *)IVM_PTLIST_ITER_GET(iter))
#define IVM_DESTRUCT_LIST_EACHPTR(list, iter) IVM_PTLIST_EACHPTR((list), iter, ivm_object_t *)

typedef struct ivm_collector_t_tag {
	ivm_int_t live_ratio; // 0 - 100
	ivm_long_t skip_time;
	ivm_double_t bc_weight;
	ivm_destruct_list_t des_log[2];
} ivm_collector_t;

#define IVM_MARK_WHITE 0

// #define IVM_COLLECTOR_GET_PERIOD(collector) ((collector)->period)
#define IVM_COLLECTOR_GET_CUR_DES_LOG(collector) ((collector)->des_log)
#define IVM_COLLECTOR_GET_EMPTY_DES_LOG(collector) (&(collector)->des_log[1])

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
	ivm_destruct_list_empty(&collector->des_log[0]);
	ivm_destruct_list_empty(&collector->des_log[1]);
	return;
}

/* only objects in destructor log will be 'informed' when being destructed */
#define ivm_collector_addDesLog(collector, obj) (ivm_destruct_list_add((collector)->des_log, (obj)))

void
ivm_collector_collect(ivm_collector_t *collector,
					  struct ivm_vmstate_t_tag *state,
					  ivm_heap_t *heap);

ivm_object_t *
ivm_collector_copyObject_c(ivm_object_t *obj,
						   ivm_traverser_arg_t *arg);

IVM_INLINE
ivm_bool_t /* true for not trav */
ivm_collector_quickCheck(ivm_object_t *obj,
						 ivm_traverser_arg_t *arg,
						 ivm_object_t **addr)
{
	ivm_object_t *copy;

	if (!obj) {
		*addr = IVM_NULL;
		return IVM_TRUE;
	}

	copy = IVM_OBJECT_GET(obj, COPY);
	if (copy) {
		*addr = copy;
		return IVM_TRUE;
	}

	/*if (ivm_heap_isIn(arg->heap, obj)) {
		*addr = obj;
		return IVM_TRUE;
	}*/

	return IVM_FALSE;
}

IVM_INLINE
ivm_object_t *
ivm_collector_copyObject(ivm_object_t *obj,
						 ivm_traverser_arg_t *arg)
{
	ivm_object_t *ret;
	if (ivm_collector_quickCheck(obj, arg, &ret))
		return ret;
	return ivm_collector_copyObject_c(obj, arg);
}

IVM_INLINE
void
ivm_collector_copyObject_p(ivm_object_t *obj,
						   ivm_traverser_arg_t *arg,
						   ivm_object_t **addr)
{
	if (ivm_collector_quickCheck(obj, arg, addr)) return;
	*addr = ivm_collector_copyObject_c(obj, arg);
	return;
}

IVM_COM_END

#endif
