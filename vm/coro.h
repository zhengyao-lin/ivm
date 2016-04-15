#ifndef _IVM_VM_CORO_H_
#define _IVM_VM_CORO_H_

#include "type.h"
#include "obj.h"
#include "stack.h"
#include "func.h"

typedef struct {
	ivm_vmstack_t *stack;

	ivm_size_t pc;
	ivm_exec_t *exec;
} ivm_coro_t;

#endif
