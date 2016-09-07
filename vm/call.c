#include "pub/const.h"
#include "pub/err.h"

#include "std/mem.h"

#include "call.h"
#include "runtime.h"
#include "instr.h"

ivm_instr_t *
ivm_frame_popToCatch(ivm_frame_t *frame,
					 ivm_object_t ***sp_p)
{
	ivm_instr_t *catc;

	do {
		catc = ivm_frame_popCurCatch(frame);
		if (catc) return catc;
	} while (ivm_frame_popBlock(frame, sp_p));

	return IVM_NULL;
}

void
ivm_frame_popAllCatch(ivm_frame_t *frame,
					  ivm_object_t ***sp_p)
{
	do {
		if (!ivm_frame_popCurCatch(frame)) return;
	} while (ivm_frame_popBlock(frame, sp_p));

	return;
}

void
ivm_frame_stack_init(ivm_frame_stack_t *stack)
{
	stack->alloc = IVM_DEFAULT_FRAME_STACK_BUFFER_SIZE;
	stack->top = 0;
	stack->frames = STD_ALLOC(sizeof(ivm_frame_t) * IVM_DEFAULT_FRAME_STACK_BUFFER_SIZE);

	IVM_ASSERT(stack->frames, IVM_ERROR_MSG_FAILED_ALLOC_NEW("frame stack buffer"));

	return;
}

ivm_frame_stack_t *
ivm_frame_stack_new()
{
	ivm_frame_stack_t *ret = STD_ALLOC(sizeof(ivm_frame_stack_t));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("frame stack"));

	ivm_frame_stack_init(ret);

	return ret;
}

void
ivm_frame_stack_free(ivm_frame_stack_t *stack)
{
	if (stack) {
		STD_FREE(stack->frames);
		STD_FREE(stack);
	}

	return;
}

void
ivm_frame_stack_dump(ivm_frame_stack_t *stack)
{
	if (stack) {
		STD_FREE(stack->frames);
	}

	return;
}
