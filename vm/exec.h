#ifndef _IVM_VM_EXEC_H_
#define _IVM_VM_EXEC_H_

#include "type.h"
#include "op.h"

#define IVM_DEFAULT_PREALLOC_EXEC_SIZE 50

typedef struct {
	ivm_size_t length;
	ivm_size_t cur;
	ivm_byte_t *code;
} ivm_exec_t;

ivm_exec_t *
ivm_exec_new();
void
ivm_exec_free(ivm_exec_t *exec);
void
ivm_exec_compact(ivm_exec_t *exec);

void
ivm_exec_addBuffer(ivm_exec_t *exec);
void
ivm_exec_addCode(ivm_exec_t *exec,
				 ivm_opcode_t op,
				 ivm_size_t arg_count, ...);

#endif
