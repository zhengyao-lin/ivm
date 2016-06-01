#ifndef _IVM_VM_OP_H_
#define _IVM_VM_OP_H_

#include "pub/com.h"
#include "type.h"
#include "vmstack.h"
#include "str.h"

IVM_COM_HEADER

struct ivm_coro_t_tag;
struct ivm_vmstate_t_tag;
struct ivm_ctchain_t_tag;
struct ivm_exec_t_tag;
struct ivm_instr_t_tag;

#define IVM_OP(name) IVM_OP_##name

typedef enum {

#define OP_GEN(o, name, arg) IVM_OP(o),
	#include "op.def"
#undef OP_GEN

} ivm_opcode_t;

typedef enum {
	IVM_ACTION_NONE = 0,
	IVM_ACTION_INVOKE,
	IVM_ACTION_RETURN,
	IVM_ACTION_YIELD
} ivm_action_t;

typedef ivm_action_t (*ivm_op_proc_t)(struct ivm_vmstate_t_tag *state,
									  struct ivm_coro_t_tag *coro,
									  ivm_vmstack_t *stack,
									  struct ivm_ctchain_t_tag *context,
									  ivm_string_pool_t *pool,
									  struct ivm_instr_t_tag **instr);

typedef struct {
	ivm_opcode_t op;
	ivm_op_proc_t proc;
	const char *name;
	const char *args;
} ivm_op_table_t;

ivm_op_proc_t
ivm_op_table_getProc(ivm_opcode_t op);
const char *
ivm_op_table_getArg(ivm_opcode_t op);
const char *
ivm_op_table_getName(ivm_opcode_t op);

IVM_COM_END

#endif
