#include "pub/mem.h"
#include "stack.h"
#include "err.h"

ivm_stack_t *
ivm_stack_new()
{
	ivm_stack_t *ret = MEM_ALLOC(sizeof(*ret));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("stack structure"));

	ret->size = IVM_DEFAULT_PREALLOC_STACK_SIZE;
	ret->top = 0;
	ret->st = MEM_ALLOC_INIT(sizeof(*ret->st)
							 * IVM_DEFAULT_PREALLOC_STACK_SIZE);

	IVM_ASSERT(ret->st, IVM_ERROR_MSG_FAILED_ALLOC_NEW("stack"));

	return ret;
}

void
ivm_stack_free(ivm_stack_t *stack)
{
	if (stack) {
		MEM_FREE(stack->st);
		MEM_FREE(stack);
	}

	return;
}

void
ivm_stack_inc(ivm_stack_t *stack)
{
	stack->st = MEM_REALLOC(stack->st,
							sizeof(*stack->st)
							* (stack->size + IVM_DEFAULT_PREALLOC_STACK_SIZE));
	IVM_ASSERT(stack->st, IVM_ERROR_MSG_FAILED_ALLOC_NEW("increased stack space"));
	stack->size += IVM_DEFAULT_PREALLOC_STACK_SIZE;

	return;
}

void
ivm_stack_push(ivm_stack_t *stack, void *p)
{
	if (stack->top >= stack->size)
		ivm_stack_inc(stack);

	stack->st[stack->top++] = p;

	return;
}

void *
ivm_stack_pop(ivm_stack_t *stack)
{
	if (stack->top > 0)
		return stack->st[--stack->top];
	return IVM_NULL;
}

#define VALUE_AT(stack, i) ((stack)->st[i])

void
ivm_stack_foreach(ivm_stack_t *stack, ivm_stack_foreach_proc_t proc)
{
	ivm_size_t i;

	for (i = 0; i < stack->top; i++)
		proc(VALUE_AT(stack, i));

	return;
}
