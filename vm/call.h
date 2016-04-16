#ifndef _IVM_VM_CALL_H_
#define _IVM_VM_CALL_H_

#include "type.h"
#include "func.h"
#include "stack.h"

struct ivm_ctchain_t_tag;

typedef struct {
	ivm_function_t *caller;
	
	ivm_size_t st_top;
	ivm_pc_t pc;
	struct ivm_ctchain_t_tag *context;
} ivm_caller_info_t;

ivm_caller_info_t *
ivm_caller_info_new(ivm_function_t *caller,
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
#define ivm_call_stack_top ivm_stack_top
#define ivm_call_stack_pop(stack) ((ivm_caller_info_t *)ivm_stack_pop(stack))
#define ivm_call_stack_setTop ivm_stack_setTop
#define ivm_call_stack_foreach ivm_stack_foreach

#endif
