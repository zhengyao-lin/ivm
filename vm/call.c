#include "pub/const.h"
#include "pub/mem.h"
#include "pub/err.h"

#include "call.h"
#include "runtime.h"
#include "instr.h"

ivm_instr_t *
ivm_frame_popCatch(ivm_frame_t *frame)
{
	ivm_instr_t *ret;

	if (frame->nested_cat) {
		ret = ivm_instr_stack_pop(frame->cat.astack);
		
		if (!ret) {
			ivm_instr_stack_free(frame->cat.astack);
			frame->cat.astack = IVM_NULL;
			frame->nested_cat = IVM_FALSE;
		}

		return ret;
	}

	ret = frame->cat.addr;
	frame->cat.addr = IVM_NULL;

	return ret;
}

void
ivm_frame_pushCatch(ivm_frame_t *frame, ivm_instr_t *cat)
{
	if (frame->cat.addr) {
		if (!frame->nested_cat) {
			ivm_instr_stack_t *stack = ivm_instr_stack_new();
			
			ivm_instr_stack_push(stack, frame->cat.addr);

			frame->cat.astack = stack;
			frame->nested_cat = IVM_TRUE;
		}

		ivm_instr_stack_push(frame->cat.astack, cat);

		return;
	}

	frame->cat.addr = cat;

	return;
}

void
ivm_frame_stack_init(ivm_frame_stack_t *stack)
{
	stack->alloc = IVM_DEFAULT_FRAME_STACK_BUFFER_SIZE;
	stack->top = 0;
	stack->frames = MEM_ALLOC(sizeof(ivm_frame_t)
							  * IVM_DEFAULT_FRAME_STACK_BUFFER_SIZE,
							  ivm_frame_t *);

	IVM_ASSERT(stack->frames, IVM_ERROR_MSG_FAILED_ALLOC_NEW("frame stack buffer"));

	return;
}

ivm_frame_stack_t *
ivm_frame_stack_new()
{
	ivm_frame_stack_t *ret = MEM_ALLOC(sizeof(ivm_frame_stack_t),
									   ivm_frame_stack_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("frame stack"));

	ivm_frame_stack_init(ret);

	return ret;
}

void
ivm_frame_stack_free(ivm_frame_stack_t *stack)
{
	if (stack) {
		MEM_FREE(stack->frames);
		MEM_FREE(stack);
	}

	return;
}

void
ivm_frame_stack_dump(ivm_frame_stack_t *stack)
{
	if (stack) {
		MEM_FREE(stack->frames);
	}

	return;
}
