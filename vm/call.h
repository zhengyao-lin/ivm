#ifndef _IVM_VM_CALL_H_
#define _IVM_VM_CALL_H_

#include "pub/com.h"
#include "pub/const.h"
#include "type.h"
#include "exec.h"
#include "std/stack.h"
#include "std/pool.h"

IVM_COM_HEADER

#if IVM_DEBUG

#define IVM_DEFAULT_FRAME_STACK_BUFFER_SIZE 1

#else

#define IVM_DEFAULT_FRAME_STACK_BUFFER_SIZE 32

#endif

#define IVM_EXEC_INFO_HEAD \
	ivm_pc_t pc; \
	ivm_exec_t *exec; \
	struct ivm_ctchain_t_tag *context;

struct ivm_vmstate_t_tag;
struct ivm_ctchain_t_tag;
struct ivm_runtime_t_tag;

typedef struct ivm_frame_t_tag {
	IVM_EXEC_INFO_HEAD

	ivm_size_t st_top;
} ivm_frame_t;

#define IVM_FRAME_GET_EXEC(frame) ((frame) ? (frame)->exec : IVM_NULL)
#define IVM_FRAME_GET_STACK_TOP(frame) ((frame) ? (frame)->st_top : 0)
#define IVM_FRAME_GET_PC(frame) ((frame) ? (frame)->pc : 0)
#define IVM_FRAME_GET_CONTEXT(frame) ((frame) ? (frame)->context : IVM_NULL)

#define IVM_FRAME_GET(obj, member) IVM_GET((obj), IVM_FRAME, member)
#define IVM_FRAME_SET(obj, member, val) IVM_SET((obj), IVM_FRAME, member, (val))

ivm_frame_t *
ivm_frame_new(struct ivm_vmstate_t_tag *state,
			  struct ivm_runtime_t_tag *runtime,
			  ivm_size_t st_top);
void
ivm_frame_free(ivm_frame_t *frame,
			   struct ivm_vmstate_t_tag *state);

typedef ivm_stack_t ivm_frame_stack_t;

#define ivm_frame_stack_new() (ivm_stack_new_c(IVM_DEFAULT_FRAME_STACK_BUFFER_SIZE))
#define ivm_frame_stack_free ivm_stack_free
#define ivm_frame_stack_inc ivm_stack_inc
#define ivm_frame_stack_push ivm_stack_push
#define ivm_frame_stack_top(stack) ((ivm_frame_t *)ivm_stack_top(stack))
#define ivm_frame_stack_size ivm_stack_size
#define ivm_frame_stack_pop(stack) ((ivm_frame_t *)ivm_stack_pop(stack))
#define ivm_frame_stack_setTop ivm_stack_setTop
#define ivm_frame_stack_foreach ivm_stack_foreach
#define ivm_frame_stack_foreach_arg ivm_stack_foreach_arg

typedef ivm_ptpool_t ivm_frame_pool_t;

#define ivm_frame_pool_new(count) (ivm_ptpool_new((count), sizeof(ivm_frame_t)))
#define ivm_frame_pool_free ivm_ptpool_free
#define ivm_frame_pool_alloc(pool) ((ivm_frame_t *)ivm_ptpool_alloc(pool))
#define ivm_frame_pool_dump ivm_ptpool_dump

IVM_COM_END

#endif
