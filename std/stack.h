#ifndef _IVM_STD_STACK_H_
#define _IVM_STD_STACK_H_

#include "pub/com.h"
#include "pub/const.h"
#include "pub/type.h"

#include "list.h"

IVM_COM_HEADER

typedef ivm_ptlist_t ivm_stack_t;
typedef ivm_ptlist_foreach_proc_t ivm_stack_foreach_proc_t;

#define ivm_stack_new_c ivm_ptlist_new_c
#define ivm_stack_new() (ivm_stack_new_c(IVM_DEFAULT_STACK_BUFFER_SIZE))
#define ivm_stack_free ivm_ptlist_free
#define ivm_stack_inc ivm_ptlist_inc

#define ivm_stack_top ivm_ptlist_last
#define ivm_stack_before(stack, i) (ivm_ptlist_at((stack), ivm_ptlist_size(stack) - 1 - (i)))
#define ivm_stack_size ivm_ptlist_size

#define ivm_stack_push ivm_ptlist_push
#define ivm_stack_pop ivm_ptlist_pop
#define ivm_stack_at ivm_ptlist_at
#define ivm_stack_ptrAt ivm_ptlist_ptrAt
#define ivm_stack_set ivm_ptlist_set
#define ivm_stack_cut(stack, i) (ivm_ptlist_setCur((stack), ivm_ptlist_size(stack) - (i)), \
								 &ivm_stack_before((stack), -1))

#define ivm_stack_setTop  ivm_ptlist_setCur
#define ivm_stack_incTop  ivm_ptlist_incCur
#define ivm_stack_foreach ivm_ptlist_foreach
#define ivm_stack_foreach_arg ivm_ptlist_foreach_arg

#define IVM_STACK_ITER_TYPE IVM_PTLIST_ITER_TYPE
#define IVM_STACK_ITER_SET IVM_PTLIST_ITER_SET
#define IVM_STACK_ITER_GET IVM_PTLIST_ITER_GET
#define IVM_STACK_EACHPTR IVM_PTLIST_EACHPTR

IVM_INLINE
void
ivm_stack_pushAt(ivm_stack_t *stack,
				  ivm_size_t i,
				  void *p)
{
	if (i >= stack->alloc) {
		ivm_stack_inc(stack);
	}

	stack->lst[i] = p;

	return;
}

IVM_COM_END

#endif
