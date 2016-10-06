#ifndef _IVM_VM_LISTOBJ_H_
#define _IVM_VM_LISTOBJ_H_

#include "pub/com.h"
#include "pub/const.h"
#include "pub/type.h"

#include "std/list.h"

#include "obj.h"

IVM_COM_HEADER

struct ivm_vmstate_t_tag;
struct ivm_heap_t_tag;
struct ivm_traverser_arg_t_tag;

typedef struct {
	IVM_OBJECT_HEADER
	
	ivm_size_t alloc;
	ivm_size_t size;
	ivm_object_t **lst;
} ivm_list_object_t;

ivm_object_t *
ivm_list_object_new(struct ivm_vmstate_t_tag *state,
					ivm_size_t size);

ivm_object_t *
ivm_list_object_new_b(struct ivm_vmstate_t_tag *state,
					  ivm_size_t buf);

ivm_object_t *
ivm_list_object_new_c(struct ivm_vmstate_t_tag *state,
					  ivm_object_t **init,
					  ivm_size_t size);

ivm_object_t *
ivm_list_object_iter_new(struct ivm_vmstate_t_tag *state,
						 ivm_list_object_t *list);

#define ivm_list_object_core(list) ((list)->lst)

IVM_INLINE
ivm_size_t
ivm_list_object_getSize(ivm_list_object_t *list)
{
	return list->size;
}

IVM_INLINE
ivm_long_t
ivm_list_object_realIndex(ivm_list_object_t *list,
						  ivm_long_t i)
{
	return ivm_list_realIndex(list->size, i);
}

// return 0 if error
ivm_size_t
ivm_list_object_push(ivm_list_object_t *list,
					 struct ivm_vmstate_t_tag *state,
					 ivm_object_t *obj);

IVM_INLINE
ivm_object_t *
ivm_list_object_get(ivm_list_object_t *list,
					ivm_long_t i)
{
	if (i >= 0) {
		if (i < list->size) {
			return list->lst[i];
		}
	} else if (list->size) {
		i = -i % list->size;

		if (i) {
			return list->lst[list->size - i];
		} else {
			return *list->lst;
		}
	}

	return IVM_NULL;
}

IVM_INLINE
ivm_object_t *
_ivm_list_object_get_c(ivm_list_object_t *list,
					   ivm_long_t i)
{
	return list->lst[i];
}

/* return NULL if error */
ivm_object_t *
ivm_list_object_set(ivm_list_object_t *list,
					struct ivm_vmstate_t_tag *state,
					ivm_long_t i,
					ivm_object_t *obj);

/* return NULL if error */
ivm_object_t *
ivm_list_object_link(ivm_list_object_t *list1,
					 ivm_list_object_t *list2,
					 struct ivm_vmstate_t_tag *state);

void
ivm_list_object_reverse(ivm_list_object_t *list);

void
ivm_list_object_step(ivm_list_object_t *list,
					 ivm_long_t step);

void
_ivm_list_object_unpackTo(ivm_list_object_t *list,
						  struct ivm_vmstate_t_tag *state,
						  ivm_object_t **sp,
						  ivm_size_t req);

/* return false if error */
ivm_bool_t
ivm_list_object_multiply(ivm_list_object_t *list,
						 struct ivm_vmstate_t_tag *state,
						 ivm_size_t times);

void
ivm_list_object_destructor(ivm_object_t *obj,
						   struct ivm_vmstate_t_tag *state);

void
ivm_list_object_cloner(ivm_object_t *obj,
					   struct ivm_vmstate_t_tag *state);

void
ivm_list_object_traverser(ivm_object_t *obj,
						  struct ivm_traverser_arg_t_tag *arg);

typedef struct {
	IVM_OBJECT_HEADER

	ivm_list_object_t *list;
	ivm_size_t cur;
} ivm_list_object_iter_t;

ivm_object_t *
ivm_list_object_iter_next(ivm_list_object_iter_t *iter,
						  struct ivm_vmstate_t_tag *state);

void
ivm_list_object_iter_traverser(ivm_object_t *obj,
							   struct ivm_traverser_arg_t_tag *arg);

IVM_COM_END

#endif
