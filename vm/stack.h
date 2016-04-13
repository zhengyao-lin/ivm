#ifndef _IVM_VM_STACK_H_
#define _IVM_VM_STACK_H_

#include "type.h"
#include "obj.h"

#define IVM_DEFAULT_PREALLOC_STACK_SIZE 50

typedef struct {
	ivm_size_t size;
	ivm_size_t top;
	ivm_object_t **st;
} ivm_vmstack_t;

ivm_vmstack_t *
ivm_vmstack_new();
void
ivm_vmstack_free(ivm_vmstack_t *stack);
void
ivm_vmstack_inc(ivm_vmstack_t *stack);

void
ivm_vmstack_push(ivm_vmstack_t *stack, ivm_object_t *obj);
#define ivm_vmstack_top(stack) (stack->top > 0 ? stack->st[stack->top - 1] : IVM_NULL)
ivm_object_t *
ivm_vmstack_pop(ivm_vmstack_t *stack);

#endif
