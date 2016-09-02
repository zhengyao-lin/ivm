#ifndef _IVM_VM_INLINE_CALL_H_
#define _IVM_VM_INLINE_CALL_H_

#include "pub/com.h"
#include "pub/err.h"
#include "pub/vm.h"

#include "std/mem.h"

#include "vm/call.h"
#include "vm/obj.h"

IVM_COM_HEADER

IVM_INLINE
ivm_frame_t *
ivm_frame_new(ivm_vmstate_t *state,
			  ivm_runtime_t *runtime)
{
	ivm_frame_t *ret = ivm_vmstate_allocFrame(state);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("frame"));

	STD_MEMCPY(ret, runtime, sizeof(IVM_FRAME_HEADER_SIZE));

	return ret;
}

IVM_INLINE
void
ivm_frame_free(ivm_frame_t *frame, ivm_vmstate_t *state)
{
	ivm_vmstate_dumpFrame(state, frame);
	return;
}

IVM_INLINE
void
_ivm_frame_stack_expand(ivm_frame_stack_t *stack)
{
	stack->alloc <<= 1;
	stack->frames = STD_REALLOC(stack->frames,
								sizeof(ivm_frame_t)
								* stack->alloc,
								ivm_frame_t *);
	IVM_ASSERT(stack->frames,
			   IVM_ERROR_MSG_FAILED_ALLOC_NEW("expanded frame stack buffer"));

	return;
}

IVM_INLINE
void
ivm_frame_stack_push(ivm_frame_stack_t *stack,
					 ivm_runtime_t *runtime)
{
	if (stack->top >= stack->alloc) {
		_ivm_frame_stack_expand(stack);
	}

	STD_MEMCPY(stack->frames + stack->top++, runtime,
			 IVM_FRAME_HEADER_SIZE);

	return;
}

IVM_INLINE
ivm_frame_t *
ivm_frame_stack_pop(ivm_frame_stack_t *stack,
					ivm_runtime_t *runtime)
{
	ivm_frame_t *ret = IVM_NULL;

	if (stack->top) {
		runtime->sp = runtime->bp;
		STD_MEMCPY(runtime, (ret = stack->frames + --stack->top),
				 IVM_FRAME_HEADER_SIZE);
	}

	return ret;
}

IVM_COM_END

#endif
