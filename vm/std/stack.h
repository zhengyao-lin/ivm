#ifndef _IVM_VM_STD_STACK_H_
#define _IVM_VM_STD_STACK_H_

#include "list.h"
#include "../type.h"

#define IVM_DEFAULT_STACK_BUFFER_SIZE 32

typedef ivm_ptlist_t ivm_stack_t;
typedef ivm_ptlist_foreach_proc_t ivm_stack_foreach_proc_t;

#define ivm_stack_new_c ivm_ptlist_new_c
#define ivm_stack_new() (ivm_stack_new_c(IVM_DEFAULT_STACK_BUFFER_SIZE))
#define ivm_stack_free ivm_ptlist_free
#define ivm_stack_inc ivm_ptlist_inc

#define ivm_stack_top ivm_ptlist_last
#define ivm_stack_size ivm_ptlist_size

#define ivm_stack_push ivm_ptlist_push
#define ivm_stack_pop ivm_ptlist_pop

#define ivm_stack_setTop  ivm_stack_setCur
#define ivm_stack_foreach ivm_ptlist_foreach

#endif
