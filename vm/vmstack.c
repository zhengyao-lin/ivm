#include "pub/type.h"
#include "pub/mem.h"
#include "pub/err.h"

#include "vmstack.h"
#include "obj.h"
#include "coro.h"
#include "call.h"

void
ivm_vmstack_init(ivm_vmstack_t *stack)
{
	stack->size = IVM_DEFAULT_VMSTACK_BUFFER_SIZE;
	stack->edge = (
		(stack->bottom = MEM_ALLOC(
			sizeof(*stack->bottom)
			* IVM_DEFAULT_VMSTACK_BUFFER_SIZE,
			ivm_object_t **
		)) + IVM_DEFAULT_VMSTACK_BUFFER_SIZE
	);

	IVM_ASSERT(stack->bottom, IVM_ERROR_MSG_FAILED_ALLOC_NEW("vm stack"));

	return;
}

ivm_vmstack_t *
ivm_vmstack_new()
{
	ivm_vmstack_t *ret = MEM_ALLOC(sizeof(*ret),
								   ivm_vmstack_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("vm stack"));

	ivm_vmstack_init(ret);

	return ret;
}

void
ivm_vmstack_free(ivm_vmstack_t *stack)
{
	if (stack) {
		MEM_FREE(stack->bottom);
		MEM_FREE(stack);
	}

	return;
}

void
ivm_vmstack_dump(ivm_vmstack_t *stack)
{
	if (stack) {
		MEM_FREE(stack->bottom);
	}

	return;
}

void
ivm_vmstack_inc(ivm_vmstack_t *stack,
				ivm_frame_stack_t *fstack)
{
	ivm_object_t **nst, **ost, **tmp_bp;
	ivm_frame_stack_iterator_t siter;
	ivm_frame_t *tmp;

	stack->size <<= 1;
	ost = stack->bottom;
	nst = MEM_REALLOC(
		ost,
		sizeof(*ost)
		* stack->size,
		ivm_object_t **
	);

	IVM_ASSERT(nst, IVM_ERROR_MSG_FAILED_ALLOC_NEW("expanded vm stack"));

	IVM_FRAME_STACK_EACHPTR(fstack, siter) {
		tmp = IVM_FRAME_STACK_ITER_GET(siter);
		tmp_bp = nst + ivm_vmstack_offset(stack, IVM_FRAME_GET(tmp, BP));
		IVM_FRAME_SET(tmp, BP, tmp_bp);
	}

	stack->bottom = nst;
	stack->edge = nst + stack->size;

	return;
}

void
ivm_vmstack_inc_c(ivm_vmstack_t *stack,
				  struct ivm_coro_t_tag *coro)
{
	ivm_object_t **nst, **ost, **tmp_bp, **tmp_sp;
	ivm_frame_stack_iterator_t siter;
	ivm_frame_t *tmp;
	ivm_runtime_t *runtime = IVM_CORO_GET(coro, RUNTIME);

	stack->size <<= 1;
	ost = stack->bottom;
	nst = MEM_REALLOC(
		ost,
		sizeof(*ost)
		* stack->size,
		ivm_object_t **
	);

	IVM_ASSERT(nst, IVM_ERROR_MSG_FAILED_ALLOC_NEW("expanded vm stack"));

	IVM_FRAME_STACK_EACHPTR(IVM_CORO_GET(coro, FRAME_STACK), siter) {
		tmp = IVM_FRAME_STACK_ITER_GET(siter);
		tmp_bp = nst + ivm_vmstack_offset(stack, IVM_FRAME_GET(tmp, BP));
		IVM_FRAME_SET(tmp, BP, tmp_bp);
	}

	if (runtime) {
		tmp_bp = nst + ivm_vmstack_offset(stack, IVM_RUNTIME_GET(runtime, BP));
		tmp_sp = nst + ivm_vmstack_offset(stack, IVM_RUNTIME_GET(runtime, SP));
		IVM_RUNTIME_SET(runtime, BP, tmp_bp);
		IVM_RUNTIME_SET(runtime, SP, tmp_sp);
	}

	stack->bottom = nst;
	stack->edge = nst + stack->size;

	return;
}

void
ivm_vmstack_push(ivm_coro_t *coro,
				 ivm_object_t *obj)
{
	ivm_vmstack_t *stack = IVM_CORO_GET(coro, STACK);
	ivm_runtime_t *runtime = IVM_CORO_GET(coro, RUNTIME);
	ivm_object_t **sp;

	IVM_ASSERT(runtime, IVM_ERROR_MSG_PUSH_DEAD_CORO_STACK);
	
	sp = IVM_RUNTIME_GET(runtime, SP_INC);

	*sp++ = obj;

	if (sp == stack->edge) {
		ivm_vmstack_inc_c(stack, coro);
	}

	return;
}
