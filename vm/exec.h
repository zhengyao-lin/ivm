#ifndef _IVM_VM_EXEC_H_
#define _IVM_VM_EXEC_H_

#include "std/list.h"
#include "type.h"
#include "op.h"

#define IVM_DEFAULT_EXEC_BUFFER_SIZE 32
#define IVM_DEFAULT_EXEC_LIST_BUFFER_SIZE 32

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

#define ivm_exec_opAt(exec, pc) ((ivm_opcode_t)exec->code[pc])
#define ivm_exec_length(exec) (exec->cur)

typedef ivm_size_t ivm_exec_id_t;
typedef ivm_ptlist_t ivm_exec_list_t;

#define ivm_exec_list_new() (ivm_ptlist_new_c(IVM_DEFAULT_EXEC_LIST_BUFFER_SIZE))
#define ivm_exec_list_free ivm_ptlist_free
#define ivm_exec_list_register ivm_ptlist_push
#define ivm_exec_list_size ivm_ptlist_size
#define ivm_exec_list_at ivm_ptlist_at
#define ivm_exec_list_foreach ivm_ptlist_foreach

#endif
