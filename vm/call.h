#ifndef _IVM_VM_CALL_H_
#define _IVM_VM_CALL_H_

#include "pub/const.h"
#include "type.h"
#include "exec.h"
#include "std/stack.h"

#if IVM_DEBUG

#define IVM_DEFAULT_CALLER_INFO_BUFFER_SIZE 1

#else

#define IVM_DEFAULT_CALLER_INFO_BUFFER_SIZE 32

#endif

#define IVM_EXEC_INFO_HEAD \
	ivm_pc_t pc; \
	ivm_exec_t *exec; \
	struct ivm_ctchain_t_tag *context;

struct ivm_ctchain_t_tag;
struct ivm_runtime_t_tag;

typedef struct ivm_caller_info_t_tag {
	IVM_EXEC_INFO_HEAD

	ivm_size_t st_top;
} ivm_caller_info_t;

#define IVM_CALLER_INFO_GET_EXEC(info) ((info) ? (info)->exec : IVM_NULL)
#define IVM_CALLER_INFO_GET_STACK_TOP(info) ((info) ? (info)->st_top : 0)
#define IVM_CALLER_INFO_GET_PC(info) ((info) ? (info)->pc : 0)
#define IVM_CALLER_INFO_GET_CONTEXT(info) ((info) ? (info)->context : IVM_NULL)

#define IVM_CALLER_INFO_GET(obj, member) IVM_GET((obj), IVM_CALLER_INFO, member)
#define IVM_CALLER_INFO_SET(obj, member, val) IVM_SET((obj), IVM_CALLER_INFO, member, (val))

ivm_caller_info_t *
ivm_caller_info_new(struct ivm_runtime_t_tag *runtime,
					ivm_size_t st_top);
void
ivm_caller_info_free(ivm_caller_info_t *info);

typedef ivm_stack_t ivm_call_stack_t;

#define ivm_call_stack_new() (ivm_stack_new_c(IVM_DEFAULT_CALLER_INFO_BUFFER_SIZE))
#define ivm_call_stack_free ivm_stack_free
#define ivm_call_stack_inc ivm_stack_inc
#define ivm_call_stack_push ivm_stack_push
#define ivm_call_stack_top(stack) ((ivm_caller_info_t *)ivm_stack_top(stack))
#define ivm_call_stack_size ivm_stack_size
#define ivm_call_stack_pop(stack) ((ivm_caller_info_t *)ivm_stack_pop(stack))
#define ivm_call_stack_setTop ivm_stack_setTop
#define ivm_call_stack_foreach ivm_stack_foreach
#define ivm_call_stack_foreach_arg ivm_stack_foreach_arg

#endif
