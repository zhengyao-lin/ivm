#include "pub/const.h"
#include "op.h"
#include "coro.h"
#include "vmstack.h"
#include "vm.h"
#include "call.h"
#include "byte.h"
#include "err.h"

#define OP_PROC_NAME(op) ivm_op_proc_##op
#define OP_PROC(op) ivm_action_t OP_PROC_NAME(op)(ivm_vmstate_t *__ivm_state__, ivm_coro_t *__ivm_coro__)
												  /* these arguments shouldn't be used directly */

#define CORO (__ivm_coro__)
#define RUNTIME (__ivm_coro__->runtime)
#define PC (__ivm_coro__->runtime->pc)
#define STATE (__ivm_state__)
#define CALL_STACK (__ivm_coro__->call_st)

/* use this before increase pc */
#define ARG_START (&__ivm_coro__->runtime->exec->code[__ivm_coro__->runtime->pc + 1])

#define STACK (__ivm_coro__->stack)
#define STACK_SIZE() (ivm_vmstack_size(__ivm_coro__->stack))
#define STACK_TOP() (ivm_vmstack_top(__ivm_coro__->stack))
#define STACK_POP() (ivm_vmstack_pop(__ivm_coro__->stack))
#define STACK_PUSH(obj) (ivm_vmstack_push(__ivm_coro__->stack, (obj)))

#define CALL_STACK_TOP() (ivm_call_stack_top(__ivm_coro__->call_st))
#define CHECK_STACK(req) IVM_ASSERT((STACK_SIZE() - ivm_caller_info_stackTop(CALL_STACK_TOP())) \
									 >= (req), \
									IVM_ERROR_MSG_INSUFFICIENT_STACK)

#define OP_MAPPING(op) { IVM_OP(op), OP_PROC_NAME(op) }

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

OP_PROC(NEW_FUNC)
{
	printf("NEW_FUNC: no implementation\n");
	PC++;
	return IVM_ACTION_NONE;
}

OP_PROC(GET_SLOT)
{
	ivm_size_t size;
	const ivm_char_t *key = ivm_byte_readString(ARG_START, &size);
	ivm_object_t *obj;

	CHECK_STACK(1);

	obj = STACK_POP();
	if (obj) {
		STACK_PUSH(ivm_object_getSlotValue(obj, STATE, key));
	}

	PC += size + 1;

	return IVM_ACTION_NONE;
}

OP_PROC(SET_SLOT)
{
	ivm_size_t size;
	const ivm_char_t *key = ivm_byte_readString(ARG_START, &size);
	ivm_object_t *obj, *val;

	CHECK_STACK(2);

	val = STACK_POP();
	obj = STACK_TOP();
	if (obj) {
		ivm_object_setSlot(obj, STATE, key, val);
	}

	PC += size + 1;

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

OP_PROC(PRINT_OBJ)
{
	ivm_object_t *obj;

	/* printf("%ld, %ld\n", STACK_SIZE(), ivm_caller_info_stackTop(CALL_STACK_TOP())); */

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

OP_PROC(YIELD)
{
	CHECK_STACK(1);
	PC++;
	return IVM_ACTION_YIELD;
}

OP_PROC(TEST1)
{
#if 0
	ivm_exec_t *exec = ivm_exec_new();
	ivm_function_t *func = ivm_function_new(IVM_NULL, exec, IVM_INTSIG_NONE);

	ivm_exec_addCode(exec, IVM_OP(NEW_OBJ), "");
	ivm_exec_addCode(exec, IVM_OP(PRINT), "");
	ivm_exec_addCode(exec, IVM_OP(TEST2), "");
	
	PC++;
	ivm_call_stack_push(CALL_STACK, ivm_function_invoke(func, CORO));
#endif
	printf("test1\n");

	PC++;
	return IVM_ACTION_NONE;
}

OP_PROC(TEST2)
{ 
	ivm_exec_t *exec = ivm_exec_new();
	ivm_function_t *func = ivm_function_new(IVM_NULL, exec, IVM_INTSIG_NONE);

	ivm_exec_addCode(exec, IVM_OP(NEW_OBJ), "");
	ivm_exec_addCode(exec, IVM_OP(PRINT_OBJ), "");
	ivm_exec_addCode(exec, IVM_OP(TEST3), "");
	printf("this is a test string in test2\n");
	
	PC++;
	ivm_call_stack_push(CALL_STACK, ivm_function_invoke(func, CORO));

	return IVM_ACTION_NONE;
}

OP_PROC(TEST3)
{
	ivm_size_t size;

	printf("morning! this is test3\n");
	printf("string argument: %s\n", ivm_byte_readString(ARG_START, &size));

	PC += size + 1;

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
	OP_MAPPING(NONE),
	OP_MAPPING(NEW_OBJ),
	OP_MAPPING(NEW_FUNC),
	OP_MAPPING(GET_SLOT),
	OP_MAPPING(SET_SLOT),
	OP_MAPPING(GET_CONTEXT_SLOT),
	OP_MAPPING(SET_CONTEXT_SLOT),
	OP_MAPPING(PRINT_OBJ),
	OP_MAPPING(INVOKE),
	OP_MAPPING(YIELD),
	OP_MAPPING(TEST1),
	OP_MAPPING(TEST2),
	OP_MAPPING(TEST3),
	OP_MAPPING(LAST)
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
