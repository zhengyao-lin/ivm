#include "pub/const.h"
#include "pub/mem.h"
#include "pub/err.h"

#include "call.h"
#include "runtime.h"

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

IVM_INLINE
void
ivm_frame_stack_expand(ivm_frame_stack_t *stack)
{
	stack->alloc <<= 1;
	stack->frames = MEM_REALLOC(stack->frames,
							  sizeof(ivm_frame_t)
							  * stack->alloc,
							  ivm_frame_t *);
	IVM_ASSERT(stack->frames,
			   IVM_ERROR_MSG_FAILED_ALLOC_NEW("expanded frame stack buffer"));

	return;
}

void
ivm_frame_stack_push(ivm_frame_stack_t *stack,
					 ivm_runtime_t *runtime)
{
	if (stack->top >= stack->alloc) {
		ivm_frame_stack_expand(stack);
	}

	MEM_COPY(stack->frames + stack->top++, runtime,
			 IVM_FRAME_HEADER_SIZE);

	return;
}

ivm_frame_t *
ivm_frame_stack_pop(ivm_frame_stack_t *stack,
					ivm_runtime_t *runtime)
{
	ivm_frame_t *ret = IVM_NULL;

	if (stack->top) {
		runtime->sp = runtime->bp;
		MEM_COPY(runtime, (ret = stack->frames + --stack->top),
				 IVM_FRAME_HEADER_SIZE);
	}

	return ret;
}
