#ifndef _IVM_VM_OP_H_
#define _IVM_VM_OP_H_

#include "type.h"

struct ivm_coro_t_tag;
struct ivm_vmstate_t_tag;

typedef enum {
	IVM_OP_NONE = 0,
	IVM_OP_NEW_OBJ,
	IVM_OP_GET_SLOT,
	IVM_OP_SET_SLOT,
	IVM_OP_GET_CONTEXT_SLOT,
	IVM_OP_SET_CONTEXT_SLOT,
	IVM_OP_PRINT,
	IVM_OP_INVOKE,
	IVM_OP_LAST
} ivm_opcode_t;

typedef enum {
	IVM_ACTION_NONE = 0,
	IVM_ACTION_BREAK
} ivm_action_t;

typedef ivm_action_t (*ivm_op_proc_t)(struct ivm_vmstate_t_tag *state, struct ivm_coro_t_tag *coro);

typedef struct {
	ivm_opcode_t op;
	ivm_op_proc_t proc;
} ivm_op_table_t;

ivm_op_proc_t
ivm_op_table_getProc(ivm_opcode_t op);

#endif
