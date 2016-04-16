#ifndef _IVM_VM_CORO_H_
#define _IVM_VM_CORO_H_

#include "type.h"
#include "obj.h"
#include "stack.h"
#include "func.h"
#include "call.h"
#include "context.h"

typedef struct {
	ivm_pc_t pc;
	ivm_exec_t *exec;
	ivm_ctchain_t *context;
} ivm_runtime_t;

typedef struct {
	ivm_vmstack_t *stack;
	ivm_call_stack_t *call_st;
	ivm_runtime_t *runtime;
} ivm_coro_t;

ivm_object_t *
ivm_coro_start(ivm_coro_t *coro);

#endif
