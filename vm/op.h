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
	IVM_OP(NOP) = 0,
	IVM_OP(NEW_NULL),
	IVM_OP(NEW_OBJ),
	IVM_OP(NEW_NUM_I),
	IVM_OP(NEW_NUM_S),
	IVM_OP(NEW_FUNC),
	IVM_OP(GET_SLOT),
	IVM_OP(SET_SLOT),
	IVM_OP(GET_CONTEXT_SLOT),
	IVM_OP(SET_CONTEXT_SLOT),
	IVM_OP(SET_ARG),
	IVM_OP(POP),
	IVM_OP(DUP),
	IVM_OP(PRINT_OBJ),
	IVM_OP(PRINT_NUM),
	IVM_OP(PRINT_TYPE),
	IVM_OP(PRINT_STR),
	IVM_OP(INVOKE),
	IVM_OP(YIELD),
	IVM_OP(JUMP),
	IVM_OP(JUMP_IF_TRUE),
	IVM_OP(JUMP_IF_FALSE),
	IVM_OP(TEST1),
	IVM_OP(TEST2),
	IVM_OP(TEST3),
	IVM_OP(LAST)
} ivm_opcode_t;

#define IVM_OP_OFFSET_NOP					(1)
#define IVM_OP_OFFSET_NEW_NULL				(1)
#define IVM_OP_OFFSET_NEW_OBJ				(1)
#define IVM_OP_OFFSET_NEW_NUM_I				(sizeof(ivm_sint32_t) + 1)
#define IVM_OP_OFFSET_NEW_NUM_S				(IVM_STRING_POOL_INDEX_SIZE + 1)
#define IVM_OP_OFFSET_NEW_FUNC				(sizeof(ivm_sint32_t) + 1)
#define IVM_OP_OFFSET_GET_SLOT				(IVM_STRING_POOL_INDEX_SIZE + 1)
#define IVM_OP_OFFSET_SET_SLOT				(IVM_STRING_POOL_INDEX_SIZE + 1)
#define IVM_OP_OFFSET_GET_CONTEXT_SLOT		(IVM_STRING_POOL_INDEX_SIZE + 1)
#define IVM_OP_OFFSET_SET_CONTEXT_SLOT		(IVM_STRING_POOL_INDEX_SIZE + 1)
#define IVM_OP_OFFSET_SET_ARG				(IVM_STRING_POOL_INDEX_SIZE + 1)
#define IVM_OP_OFFSET_POP					(1)
#define IVM_OP_OFFSET_DUP					(sizeof(ivm_sint32_t) + 1)
#define IVM_OP_OFFSET_PRINT_OBJ				(1)
#define IVM_OP_OFFSET_PRINT_NUM				(1)
#define IVM_OP_OFFSET_PRINT_TYPE			(1)
#define IVM_OP_OFFSET_PRINT_STR				(IVM_STRING_POOL_INDEX_SIZE + 1)
#define IVM_OP_OFFSET_INVOKE				(sizeof(ivm_sint32_t) + 1)
#define IVM_OP_OFFSET_YIELD					(1)
#define IVM_OP_OFFSET_JUMP					(sizeof(ivm_sint32_t) + 1)
#define IVM_OP_OFFSET_JUMP_IF_TRUE			(sizeof(ivm_sint32_t) + 1)
#define IVM_OP_OFFSET_JUMP_IF_FALSE			(sizeof(ivm_sint32_t) + 1)
#define IVM_OP_OFFSET_TEST1					(1)
#define IVM_OP_OFFSET_TEST2					(1)
#define IVM_OP_OFFSET_TEST3					(IVM_STRING_POOL_INDEX_SIZE + 1)
#define IVM_OP_OFFSET_LAST					(1)

#define IVM_OP_OFFSET_OF(op) IVM_OP_OFFSET_##op

typedef enum {
	IVM_ACTION_NONE = 0,
	IVM_ACTION_INVOKE,
	IVM_ACTION_BREAK,
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
	ivm_pc_t offset;
} ivm_op_table_t;

ivm_op_proc_t
ivm_op_table_getProc(ivm_opcode_t op);
const char *
ivm_op_table_getArg(ivm_opcode_t op);
ivm_pc_t
ivm_op_table_getOffset(ivm_opcode_t op);
const char *
ivm_op_table_getName(ivm_opcode_t op);

IVM_COM_END

#endif
