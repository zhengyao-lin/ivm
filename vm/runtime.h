#ifndef _IVM_VM_RUNTIME_H_
#define _IVM_VM_RUNTIME_H_

#include "pub/mem.h"
#include "type.h"
#include "exec.h"
#include "context.h"

typedef struct {
	ivm_pc_t pc;
	ivm_exec_t *exec;
	ivm_ctchain_t *context;
} ivm_runtime_t;

ivm_runtime_t *
ivm_runtime_new(ivm_exec_t *exec, ivm_ctchain_t *context);
void
ivm_runtime_free(ivm_runtime_t *runtime);

#endif
