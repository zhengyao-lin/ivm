#ifndef _IVM_VM_OP_H_
#define _IVM_VM_OP_H_

#include "type.h"

struct ivm_coro_t_tag;
struct ivm_vmstate_t_tag;

#define IVM_OP(name) IVM_OP_##name

typedef enum {
	IVM_OP(NONE) = 0,
	IVM_OP(NEW_OBJ),
	IVM_OP(GET_SLOT),
	IVM_OP(SET_SLOT),
	IVM_OP(GET_CONTEXT_SLOT),
	IVM_OP(SET_CONTEXT_SLOT),
	IVM_OP(PRINT),
	IVM_OP(INVOKE),
	IVM_OP(TEST1),
	IVM_OP(TEST2),
	IVM_OP(TEST3),
	IVM_OP(LAST)
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
