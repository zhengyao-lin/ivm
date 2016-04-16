#ifndef _IVM_VM_CORO_H_
#define _IVM_VM_CORO_H_

#include "type.h"
#include "obj.h"
#include "stack.h"
#include "func.h"
#include "call.h"
#include "runtime.h"

struct ivm_vmstate_t_tag;

typedef struct ivm_coro_t_tag {
	ivm_vmstack_t *stack;
	ivm_call_stack_t *call_st;
	ivm_runtime_t *runtime;
} ivm_coro_t;

ivm_coro_t *
ivm_coro_new();
void
ivm_coro_free(ivm_coro_t *coro);

#define ivm_coro_stackTop(coro) (ivm_vmstack_topIndex(coro->stack))
ivm_object_t *
ivm_coro_start(ivm_coro_t *coro, struct ivm_vmstate_t_tag *state, ivm_function_t *root);

#endif
