#ifndef _IVM_VM_CALL_H_
#define _IVM_VM_CALL_H_

#include "type.h"
#include "exec.h"
#include "std/stack.h"

struct ivm_ctchain_t_tag;

typedef struct ivm_caller_info_t_tag {
	ivm_exec_t *exec;
	
	ivm_size_t st_top;
	ivm_pc_t pc;
	struct ivm_ctchain_t_tag *context;
} ivm_caller_info_t;

#define ivm_caller_info_exec(info) ((info) ? (info)->exec : IVM_NULL)
#define ivm_caller_info_stackTop(info) ((info) ? (info)->st_top : 0)
#define ivm_caller_info_pc(info) ((info) ? (info)->pc : 0)
#define ivm_caller_info_context(info) ((info) ? (info)->context : IVM_NULL)

ivm_caller_info_t *
ivm_caller_info_new(ivm_exec_t *exec,
					ivm_size_t st_top,
					ivm_pc_t pc,
					struct ivm_ctchain_t_tag *context);
void
ivm_caller_info_free(ivm_caller_info_t *info);

typedef ivm_stack_t ivm_call_stack_t;

#define ivm_call_stack_new ivm_stack_new
#define ivm_call_stack_free ivm_stack_free
#define ivm_call_stack_inc ivm_stack_inc
#define ivm_call_stack_push ivm_stack_push
#define ivm_call_stack_top(stack) ((ivm_caller_info_t *)ivm_stack_top(stack))
#define ivm_call_stack_size ivm_stack_size
#define ivm_call_stack_pop(stack) ((ivm_caller_info_t *)ivm_stack_pop(stack))
#define ivm_call_stack_setTop ivm_stack_setTop
#define ivm_call_stack_foreach ivm_stack_foreach

#endif
