#include "pub/const.h"
#include "op.h"
#include "coro.h"
#include "stack.h"
#include "vm.h"
#include "call.h"
#include "err.h"

#define OP_PROC_NAME(op) ivm_op_proc_##op
#define OP_PROC(op) ivm_action_t OP_PROC_NAME(op)(ivm_vmstate_t *state, ivm_coro_t *coro)

#define CORO (coro)
#define RUNTIME (coro->runtime)
#define PC (coro->runtime->pc)
#define STATE (state)
#define CALL_STACK (coro->call_st)

#define STACK (coro->stack)
#define STACK_SIZE() (ivm_vmstack_size(coro->stack))
#define STACK_TOP() (ivm_vmstack_top(coro->stack))
#define STACK_POP() (ivm_vmstack_pop(coro->stack))
#define STACK_PUSH(obj) (ivm_vmstack_push(coro->stack, (obj)))

#define CALL_STACK_TOP() (ivm_call_stack_top(coro->call_st))
#define CHECK_STACK(req) IVM_ASSERT((STACK_SIZE() - ivm_caller_info_stackTop(CALL_STACK_TOP())) \
									 >= (req), \
									IVM_ERROR_MSG_INSUFFICIENT_STACK)

#define OP(op) { IVM_OP_##op, OP_PROC_NAME(op) }

OP_PROC(NONE)
{
	printf("NONE\n");
	PC++;
	return IVM_ACTION_NONE;
}

OP_PROC(NEW_OBJ)
{
	STACK_PUSH(ivm_object_new(STATE));
	PC++;
	return IVM_ACTION_NONE;
}

OP_PROC(GET_SLOT)
{
	printf("GET_SLOT: no implementation\n");
	PC++;
	return IVM_ACTION_NONE;
}

OP_PROC(SET_SLOT)
{
	printf("SET_SLOT: no implementation\n");
	PC++;
	return IVM_ACTION_NONE;
}

OP_PROC(GET_CONTEXT_SLOT)
{
	printf("GET_CONTEXT_SLOT: no implementation\n");
	PC++;
	return IVM_ACTION_NONE;
}

OP_PROC(SET_CONTEXT_SLOT)
{
	printf("SET_CONTEXT_SLOT: no implementation\n");
	PC++;
	return IVM_ACTION_NONE;
}

OP_PROC(PRINT)
{
	ivm_object_t *obj;

	CHECK_STACK(1);

	obj = STACK_POP();
	printf("print: %p\n", (void *)obj);
	PC++;

	return IVM_ACTION_NONE;
}

OP_PROC(INVOKE)
{
	printf("INVOKE: no implementation\n");
	PC++;
	return IVM_ACTION_NONE;
}

OP_PROC(LAST)
{
	printf("LAST\n");
	PC++;
	return IVM_ACTION_NONE;
}

static const
ivm_op_table_t
ivm_global_op_table[] = {
	OP(NONE),
	OP(NEW_OBJ),
	OP(GET_SLOT),
	OP(SET_SLOT),
	OP(GET_CONTEXT_SLOT),
	OP(SET_CONTEXT_SLOT),
	OP(PRINT),
	OP(INVOKE),
	OP(LAST)
};

ivm_op_proc_t
ivm_op_table_getProc(ivm_opcode_t op)
{
#if IVM_DEBUG
	IVM_ASSERT(op < IVM_OP_LAST,
			   IVM_ERROR_MSG_BAD_OP);
	IVM_ASSERT(ivm_global_op_table[op].op == op,
			   IVM_ERROR_MSG_BAD_OP_TABLE);
#endif
	
	return ivm_global_op_table[op].proc;
}
