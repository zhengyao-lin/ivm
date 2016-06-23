#ifndef _IVM_VM_CALL_H_
#define _IVM_VM_CALL_H_

#include "pub/com.h"
#include "pub/const.h"
#include "pub/type.h"

#include "std/stack.h"
#include "std/pool.h"

#include "instr.h"
#include "exec.h"

IVM_COM_HEADER

#define IVM_EXEC_INFO_HEAD \
	ivm_exec_t *exec; \
	struct ivm_ctchain_t_tag *context; \
	ivm_instr_t *ip; \
	ivm_size_t bp;

#define IVM_EXEC_INFO_HEAD_SIZE \
	(sizeof(struct { IVM_EXEC_INFO_HEAD } IVM_NOALIGN))

struct ivm_vmstate_t_tag;
struct ivm_ctchain_t_tag;
struct ivm_runtime_t_tag;

typedef struct ivm_frame_t_tag {
	IVM_EXEC_INFO_HEAD
} IVM_NOALIGN ivm_frame_t;

#define IVM_FRAME_GET_EXEC(frame) ((frame) ? (frame)->exec : IVM_NULL)
#define IVM_FRAME_GET_BP(frame) ((frame) ? (frame)->bp : 0)
#define IVM_FRAME_GET_IP(frame) ((frame) ? (frame)->ip : 0)
#define IVM_FRAME_GET_CONTEXT(frame) ((frame) ? (frame)->context : IVM_NULL)

#define IVM_FRAME_GET(obj, member) IVM_GET((obj), IVM_FRAME, member)
#define IVM_FRAME_SET(obj, member, val) IVM_SET((obj), IVM_FRAME, member, (val))

typedef struct {
	ivm_size_t alloc;
	ivm_size_t top;
	ivm_frame_t *frames;
} ivm_frame_stack_t;

typedef ivm_frame_t *ivm_frame_stack_iterator_t;

ivm_frame_stack_t *
ivm_frame_stack_new();

void
ivm_frame_stack_free(ivm_frame_stack_t *stack);

void
ivm_frame_stack_push(ivm_frame_stack_t *stack,
					 struct ivm_runtime_t_tag *runtime);

#define ivm_frame_stack_size(stack) ((stack)->top)
#define ivm_frame_stack_at(stack, i) ((stack)->frames + (i))

ivm_frame_t *
ivm_frame_stack_pop(ivm_frame_stack_t *stack,
					struct ivm_runtime_t_tag *runtime);

#define IVM_FRAME_STACK_ITER_SET(iter, val) (*(iter) = (val))
#define IVM_FRAME_STACK_ITER_GET(iter) (iter)
#define IVM_FRAME_STACK_EACHPTR(stack, iter) \
	ivm_frame_stack_iterator_t __fs_end_##iter##__; \
	for ((iter) = (stack)->frames, \
		 __fs_end_##iter##__ = (iter) + (stack)->top; \
		 (iter) != __fs_end_##iter##__; \
		 (iter)++)

typedef ivm_ptpool_t ivm_frame_pool_t;

#define ivm_frame_pool_new(count) (ivm_ptpool_new((count), sizeof(ivm_frame_t)))
#define ivm_frame_pool_free ivm_ptpool_free
#define ivm_frame_pool_alloc(pool) ((ivm_frame_t *)ivm_ptpool_alloc(pool))
#define ivm_frame_pool_dump ivm_ptpool_dump

IVM_COM_END

#endif
