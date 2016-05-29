#ifndef _IVM_VM_VMSTACK_H_
#define _IVM_VM_VMSTACK_H_

#include "pub/com.h"
#include "pub/const.h"
#include "std/stack.h"

IVM_COM_HEADER

struct ivm_object_t_tag;

typedef ivm_stack_t ivm_vmstack_t;
typedef IVM_PTLIST_ITER_TYPE(struct ivm_object_t_tag *) ivm_vmstack_iterator_t;

#define ivm_vmstack_new() (ivm_stack_new_c(IVM_DEFAULT_VMSTACK_BUFFER_SIZE))
#define ivm_vmstack_free ivm_stack_free
#define ivm_vmstack_inc ivm_stack_inc
#define ivm_vmstack_push ivm_stack_push
#define ivm_vmstack_top(stack) ((struct ivm_object_t_tag *)ivm_stack_top(stack))
#define ivm_vmstack_before ivm_stack_before
#define ivm_vmstack_size ivm_stack_size
#define ivm_vmstack_pop(stack) ((struct ivm_object_t_tag *)ivm_stack_pop(stack))
#define ivm_vmstack_cut(stack, i) ((struct ivm_object_t_tag **)ivm_stack_cut((stack), (i)))
#define ivm_vmstack_setTop ivm_stack_setTop
#define ivm_vmstack_foreach ivm_stack_foreach
#define ivm_vmstack_foreach_arg ivm_stack_foreach_arg

#define IVM_VMSTACK_ITER_SET(iter, val) (IVM_STACK_ITER_SET((iter), (val)))
#define IVM_VMSTACK_ITER_GET(iter) ((struct ivm_object_t_tag *)IVM_STACK_ITER_GET(iter))
#define IVM_VMSTACK_EACHPTR(stack, ptr) IVM_STACK_EACHPTR((stack), (ptr), struct ivm_object_t_tag *)

IVM_COM_END

#endif
