#include "pub/mem.h"
#include "stack.h"
#include "err.h"

ivm_vmstack_t *
ivm_vmstack_new()
{
	ivm_vmstack_t *ret = MEM_ALLOC(sizeof(*ret));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("stack structure"));

	ret->size = IVM_DEFAULT_PREALLOC_STACK_SIZE;
	ret->top = 0;
	ret->st = MEM_ALLOC_INIT(sizeof(*ret->st)
							 * IVM_DEFAULT_PREALLOC_STACK_SIZE);

	IVM_ASSERT(ret->st, IVM_ERROR_MSG_FAILED_ALLOC_NEW("stack"));

	return ret;
}

void
ivm_vmstack_free(ivm_vmstack_t *stack)
{
	if (stack) {
		MEM_FREE(stack->st);
		MEM_FREE(stack);
	}

	return;
}

void
ivm_vmstack_inc(ivm_vmstack_t *stack)
{
	stack->st = MEM_REALLOC(stack->st,
							sizeof(*stack->st)
							* (stack->size + IVM_DEFAULT_PREALLOC_STACK_SIZE));
	IVM_ASSERT(stack->st, IVM_ERROR_MSG_FAILED_ALLOC_NEW("increased stack space"));
	stack->size += IVM_DEFAULT_PREALLOC_STACK_SIZE;

	return;
}

void
ivm_vmstack_push(ivm_vmstack_t *stack, ivm_object_t *obj)
{
	if (stack->top >= stack->size)
		ivm_vmstack_inc(stack);

	stack->st[stack->top++] = obj;

	return;
}

ivm_object_t *
ivm_vmstack_pop(ivm_vmstack_t *stack)
{
	if (stack->top > 0)
		return stack->st[--stack->top];
	return IVM_NULL;
}
