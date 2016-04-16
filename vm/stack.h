#ifndef _IVM_VM_STACK_H_
#define _IVM_VM_STACK_H_

#include "type.h"

#define IVM_DEFAULT_PREALLOC_STACK_SIZE 50

struct ivm_object_t_tag;

typedef struct {
	ivm_size_t size;
	ivm_size_t top;
	void **st;
} ivm_stack_t;

ivm_stack_t *
ivm_stack_new();
void
ivm_stack_free(ivm_stack_t *stack);
void
ivm_stack_inc(ivm_stack_t *stack);

void
ivm_stack_push(ivm_stack_t *stack, void *p);
#define ivm_stack_top(stack) ((stack)->top > 0 ? (stack)->st[(stack)->top - 1] : IVM_NULL)
void *
ivm_stack_pop(ivm_stack_t *stack);
#define ivm_stack_setTop(stack, t) ((stack)->top = t)

typedef ivm_stack_t ivm_vmstack_t;

#define ivm_vmstack_new ivm_stack_new
#define ivm_vmstack_free ivm_stack_free
#define ivm_vmstack_inc ivm_stack_inc
#define ivm_vmstack_push ivm_stack_push
#define ivm_vmstack_top ivm_stack_top
#define ivm_vmstack_pop(stack) ((struct ivm_object_t_tag *)ivm_stack_pop(stack))
#define ivm_vmstack_setTop ivm_stack_setTop

#endif
