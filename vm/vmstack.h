#ifndef _IVM_VM_VMSTACK_H_
#define _IVM_VM_VMSTACK_H_

#include "std/stack.h"

struct ivm_object_t_tag;

typedef ivm_stack_t ivm_vmstack_t;

#define ivm_vmstack_new ivm_stack_new
#define ivm_vmstack_free ivm_stack_free
#define ivm_vmstack_inc ivm_stack_inc
#define ivm_vmstack_push ivm_stack_push
#define ivm_vmstack_top(stack) ((struct ivm_object_t_tag *)ivm_stack_top(stack))
#define ivm_vmstack_size ivm_stack_size
#define ivm_vmstack_pop(stack) ((struct ivm_object_t_tag *)ivm_stack_pop(stack))
#define ivm_vmstack_setTop ivm_stack_setTop
#define ivm_vmstack_foreach ivm_stack_foreach

#endif
