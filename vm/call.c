#include "pub/const.h"
#include "pub/err.h"

#include "std/mem.h"

#include "call.h"
#include "runtime.h"
#include "instr.h"

void
ivm_frame_stack_init(ivm_frame_stack_t *stack)
{
	stack->alloc = IVM_DEFAULT_FRAME_STACK_BUFFER_SIZE;
	stack->top = 0;
	stack->frames = STD_ALLOC(sizeof(ivm_frame_t) * IVM_DEFAULT_FRAME_STACK_BUFFER_SIZE);

	IVM_MEMCHECK(stack->frames);

	return;
}

ivm_frame_stack_t *
ivm_frame_stack_new()
{
	ivm_frame_stack_t *ret = STD_ALLOC(sizeof(ivm_frame_stack_t));

	IVM_MEMCHECK(ret);

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
