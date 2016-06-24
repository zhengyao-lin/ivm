#ifndef _IVM_VM_VMSTACK_H_
#define _IVM_VM_VMSTACK_H_

#include "pub/com.h"
#include "pub/const.h"

#include "std/stack.h"

IVM_COM_HEADER

struct ivm_object_t_tag;

typedef ivm_stack_t ivm_vmstack_t;

#define ivm_vmstack_new() (ivm_stack_new_c(IVM_DEFAULT_VMSTACK_BUFFER_SIZE))
#define ivm_vmstack_free ivm_stack_free
#define ivm_vmstack_pushAt ivm_stack_pushAt
#define ivm_vmstack_at(stack, i) ((struct ivm_object_t_tag *)ivm_stack_at((stack), (i)))
#define ivm_vmstack_ptrAt(stack, i) ((struct ivm_object_t_tag **)ivm_stack_ptrAt((stack), (i)))
#define ivm_vmstack_cut(stack, i) ((struct ivm_object_t_tag **)ivm_stack_cut((stack), (i)))

IVM_COM_END

#endif
