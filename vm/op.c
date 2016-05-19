#include "pub/const.h"
#include "pub/com.h"
#include "op.h"
#include "coro.h"
#include "vmstack.h"
#include "vm.h"
#include "call.h"
#include "byte.h"
#include "num.h"
#include "err.h"

#define OP_PROC_NAME(op) ivm_op_proc_##op
#define OP_PROC(op) ivm_action_t OP_PROC_NAME(op)(ivm_vmstate_t *__ivm_state__, ivm_coro_t *__ivm_coro__)
												  /* these arguments shouldn't be used directly */

#define CORO (__ivm_coro__)
#define RUNTIME (__ivm_coro__->runtime)
#define CONTEXT (__ivm_coro__->runtime->context)
#define PC (__ivm_coro__->runtime->pc)
#define STATE (__ivm_state__)
#define FRAME_STACK (__ivm_coro__->frame_st)

/* use this before increase pc */
#define ARG_OFFSET(i) (&__ivm_coro__->runtime->exec->code[__ivm_coro__->runtime->pc + 1 + i])
#define ARG_START (ARG_OFFSET(0))

#define STRING_POOL (__ivm_coro__->runtime->exec->pool)

#define STACK (__ivm_coro__->stack)
#define STACK_SIZE() (ivm_vmstack_size(__ivm_coro__->stack))
#define STACK_TOP() (ivm_vmstack_top(__ivm_coro__->stack))
#define STACK_POP() (ivm_vmstack_pop(__ivm_coro__->stack))
#define STACK_PUSH(obj) (ivm_vmstack_push(__ivm_coro__->stack, (obj)))
#define STACK_BEFORE(i) (ivm_vmstack_before(__ivm_coro__->stack, (i)))
#define STACK_CUT(i) (ivm_vmstack_cut(__ivm_coro__->stack, (i)))
#define STACK_INC(i) \
	(ivm_vmstack_setTop(__ivm_coro__->stack, \
						(i) + IVM_FRAME_GET(FRAME_STACK_TOP(), STACK_TOP)))

#define FRAME_STACK_TOP() (ivm_frame_stack_top(__ivm_coro__->frame_st))
#define AVAIL_STACK (STACK_SIZE() - IVM_FRAME_GET(FRAME_STACK_TOP(), STACK_TOP))
#define CHECK_STACK(req) IVM_ASSERT(AVAIL_STACK >= (req), \
									IVM_ERROR_MSG_INSUFFICIENT_STACK)

#define OP_OFFSET IVM_OP_OFFSET_OF
#define OP_MAPPING(op, name, args) { IVM_OP(op), OP_PROC_NAME(op), (name), (args), OP_OFFSET(op) }

OP_PROC(NOP)
{
	printf("NOP\n");
	PC += OP_OFFSET(NOP);

	return IVM_ACTION_NONE;
}

OP_PROC(NEW_NULL)
{
	printf("new null\n");
	STACK_PUSH(IVM_NULL_OBJ(STATE));
	PC += OP_OFFSET(NEW_NULL);

	return IVM_ACTION_NONE;
}

OP_PROC(NEW_OBJ)
{
	STACK_PUSH(ivm_object_new(STATE));
	PC += OP_OFFSET(NEW_OBJ);

	return IVM_ACTION_NONE;
}

OP_PROC(NEW_NUM_i)
{
	ivm_sint32_t val = ivm_byte_readSInt32(ARG_START);
	STACK_PUSH(ivm_numeric_new(STATE, val));
	PC += OP_OFFSET(NEW_NUM_i);

	return IVM_ACTION_NONE;
}

OP_PROC(NEW_NUM_s)
{
	PC += OP_OFFSET(NEW_NUM_s);
	return IVM_ACTION_NONE;
}

OP_PROC(NEW_FUNC)
{
	ivm_sint32_t exec_id = ivm_byte_readSInt32(ARG_START);
	ivm_exec_t *exec = ivm_vmstate_getExec(STATE, exec_id);

	STACK_PUSH(ivm_function_object_new_nc(STATE,
										  ivm_function_new(STATE, CONTEXT, exec,
														   IVM_INTSIG_NONE)));
	PC += OP_OFFSET(NEW_FUNC);

	return IVM_ACTION_NONE;
}

OP_PROC(GET_SLOT)
{
	const ivm_char_t *key = ivm_byte_readStringFromPool(ARG_START, STRING_POOL);
	ivm_object_t *obj;

	CHECK_STACK(1);

	obj = STACK_POP();
	if (obj) {
		STACK_PUSH(ivm_object_getSlotValue(obj, STATE, key));
	}

	PC += OP_OFFSET(GET_SLOT);

	return IVM_ACTION_NONE;
}

OP_PROC(SET_SLOT)
{
	const ivm_char_t *key = ivm_byte_readStringFromPool(ARG_START, STRING_POOL);
	ivm_object_t *obj, *val;

	CHECK_STACK(2);

	obj = STACK_POP();
	val = STACK_POP();
	
	if (obj) {
		ivm_object_setSlot(obj, STATE, key, val);
	}
	
	STACK_PUSH(obj);
	PC += OP_OFFSET(SET_SLOT);

	return IVM_ACTION_NONE;
}

OP_PROC(GET_CONTEXT_SLOT)
{
	const ivm_char_t *key = ivm_byte_readStringFromPool(ARG_START, STRING_POOL);
	ivm_object_t *found = ivm_ctchain_search(CONTEXT, STATE, key);

	STACK_PUSH(found ? found : IVM_UNDEFINED(STATE));
	PC += OP_OFFSET(GET_CONTEXT_SLOT);

	return IVM_ACTION_NONE;
}

OP_PROC(SET_CONTEXT_SLOT)
{
	const ivm_char_t *key = ivm_byte_readStringFromPool(ARG_START, STRING_POOL);
	ivm_slot_t *slot;

	CHECK_STACK(1);
	slot = ivm_ctchain_searchSlot(CONTEXT, STATE, key);

	if (slot) {
		ivm_slot_setValue(slot, STATE, STACK_POP());
	} else { 
		ivm_ctchain_setLocalSlot(CONTEXT, STATE, key, STACK_POP());
	}

	PC += OP_OFFSET(SET_CONTEXT_SLOT);

	return IVM_ACTION_NONE;
}

OP_PROC(SET_ARG)
{
	const ivm_char_t *key = ivm_byte_readStringFromPool(ARG_START, STRING_POOL);

	ivm_ctchain_setLocalSlot(CONTEXT, STATE, key,
							 AVAIL_STACK >= 1
							 ? STACK_POP()
							 : IVM_UNDEFINED(STATE));

	PC += OP_OFFSET(SET_ARG);

	return IVM_ACTION_NONE;
}

OP_PROC(POP)
{
	CHECK_STACK(1);
	STACK_POP();
	PC += OP_OFFSET(POP);

	return IVM_ACTION_NONE;
}

OP_PROC(DUP)
{
	CHECK_STACK(1);
	STACK_PUSH(STACK_TOP());
	PC += OP_OFFSET(DUP);

	return IVM_ACTION_NONE;
}

OP_PROC(PRINT_OBJ)
{
	ivm_object_t *obj;

	CHECK_STACK(1);

	obj = STACK_POP();
	printf("print: %p\n", (void *)obj);
	PC += OP_OFFSET(PRINT_OBJ);

	return IVM_ACTION_NONE;
}

OP_PROC(PRINT_NUM)
{
	ivm_object_t *obj;

	CHECK_STACK(1);

	obj = STACK_POP();
	if (IVM_OBJECT_GET(obj, TYPE_TAG) == IVM_NUMERIC_T)
		printf("print num: %f\n", IVM_AS(obj, ivm_numeric_t)->val);
	else
		printf("cannot print number of object of type <%s>\n", IVM_OBJECT_GET(obj, TYPE_NAME));

	PC += OP_OFFSET(PRINT_NUM);

	return IVM_ACTION_NONE;
}

OP_PROC(PRINT_TYPE)
{
	ivm_object_t *obj;

	CHECK_STACK(1);

	obj = STACK_POP();
	printf("type: %s\n", IVM_OBJECT_GET(obj, TYPE_NAME));
	
	PC += OP_OFFSET(PRINT_TYPE);

	return IVM_ACTION_NONE;
}

OP_PROC(PRINT_STR)
{
	printf("%s\n", ivm_byte_readStringFromPool(ARG_START, STRING_POOL));
	PC += OP_OFFSET(PRINT_STR);

	return IVM_ACTION_NONE;
}

OP_PROC(INVOKE)
{
	ivm_function_t *func;
	ivm_sint32_t arg_count = ivm_byte_readSInt32(ARG_START);
	ivm_vmstack_iterator_t args;
	ivm_object_t *ret;

	CHECK_STACK(arg_count + 1);

	func = IVM_AS(STACK_POP(), ivm_function_object_t)->val;
	args = STACK_CUT(arg_count);

	PC += OP_OFFSET(INVOKE);

	ivm_function_invoke(func, STATE, CORO);

	if (ivm_function_isNative(func)) {
		ret = ivm_function_callNative(func, STATE, CONTEXT,
									  IVM_FUNCTION_SET_ARG_2(arg_count, args));
		STACK_PUSH(ret ? ret : IVM_NULL_OBJ(STATE));
	} else {
		STACK_INC(arg_count);
	}

	return IVM_ACTION_NONE;
}

OP_PROC(YIELD)
{
	CHECK_STACK(1);
	PC += OP_OFFSET(YIELD);

	return IVM_ACTION_YIELD;
}

OP_PROC(JUMP_i)
{
	PC = ivm_byte_readSInt32(ARG_START);
	return IVM_ACTION_NONE;
}

OP_PROC(JUMP_IF_TRUE_i)
{
	ivm_sint32_t addr = ivm_byte_readSInt32(ARG_START);

	if (ivm_object_toBool(STACK_POP(), STATE)) {
		printf("jump true!\n");
		PC = addr;
	} else {
		printf("no jump true\n");
		PC += OP_OFFSET(JUMP_IF_TRUE_i);
	}

	return IVM_ACTION_NONE;
}

OP_PROC(JUMP_IF_FALSE_i)
{
	ivm_sint32_t addr = ivm_byte_readSInt32(ARG_START);

	if (!ivm_object_toBool(STACK_POP(), STATE)) {
		printf("jump false!\n");
		PC = addr;
	} else {
		printf("no jump false\n");
		PC += OP_OFFSET(JUMP_IF_FALSE_i);
	}

	return IVM_ACTION_NONE;
}

OP_PROC(TEST1)
{
	printf("test1\n");
	PC += OP_OFFSET(TEST1);

	return IVM_ACTION_NONE;
}

OP_PROC(TEST2)
{ 
	PC += OP_OFFSET(TEST2);
	return IVM_ACTION_NONE;
}

OP_PROC(TEST3)
{
	printf("morning! this is test3\n");
	printf("string argument: %s\n", ivm_byte_readStringFromPool(ARG_START, STRING_POOL));
	PC += OP_OFFSET(TEST3);

	return IVM_ACTION_NONE;
}

OP_PROC(LAST)
{
	printf("LAST\n");
	PC += OP_OFFSET(LAST);

	return IVM_ACTION_NONE;
}

IVM_PRIVATE const
ivm_op_table_t
ivm_global_op_table[] = {
	OP_MAPPING(NOP,					"nop",					""),
	OP_MAPPING(NEW_NULL,			"new_null",				""),
	OP_MAPPING(NEW_OBJ,				"new_obj",				""),
	OP_MAPPING(NEW_NUM_i,			"new_num_i",			"$i32"),
	OP_MAPPING(NEW_NUM_s,			"new_num_s",			"$s"),
	OP_MAPPING(NEW_FUNC,			"new_func",				"$i32"),
	OP_MAPPING(GET_SLOT,			"get_slot",				"$s"),
	OP_MAPPING(SET_SLOT,			"set_slot",				"$s"),
	OP_MAPPING(GET_CONTEXT_SLOT,	"get_context_slot",		"$s"),
	OP_MAPPING(SET_CONTEXT_SLOT,	"set_context_slot",		"$s"),
	OP_MAPPING(SET_ARG,				"set_arg",				"$s"),
	OP_MAPPING(POP,					"pop",					""),
	OP_MAPPING(DUP,					"dup",					""),
	OP_MAPPING(PRINT_OBJ,			"print_obj",			""),
	OP_MAPPING(PRINT_NUM,			"print_num",			""),
	OP_MAPPING(PRINT_TYPE,			"print_type",			""),
	OP_MAPPING(PRINT_STR,			"print_str",			"$s"),
	OP_MAPPING(INVOKE,				"invoke",				"$i32"),
	OP_MAPPING(YIELD,				"yield",				""),
	OP_MAPPING(JUMP_i,				"jmp",					"$i32"),
	OP_MAPPING(JUMP_IF_TRUE_i,		"jmp_true",				"$i32"),
	OP_MAPPING(JUMP_IF_FALSE_i,		"jmp_false",			"$i32"),
	OP_MAPPING(TEST1,				"test1",				""),
	OP_MAPPING(TEST2,				"test2",				"$i8$i16"),
	OP_MAPPING(TEST3,				"test3",				"$s"),
	OP_MAPPING(LAST,				"last",					"")
};

#if IVM_DEBUG

#define checkLegal() \
	IVM_ASSERT(op < IVM_OP_LAST, \
			   IVM_ERROR_MSG_BAD_OP); \
	IVM_ASSERT(ivm_global_op_table[op].op == op, \
			   IVM_ERROR_MSG_BAD_OP_TABLE);

#else

#define checkLegal()

#endif

ivm_op_proc_t
ivm_op_table_getProc(ivm_opcode_t op)
{
	checkLegal();
	return ivm_global_op_table[op].proc;
}

const char *
ivm_op_table_getArg(ivm_opcode_t op)
{
	checkLegal()
	return ivm_global_op_table[op].args;
}

ivm_pc_t
ivm_op_table_getOffset(ivm_opcode_t op)
{
	checkLegal()
	return ivm_global_op_table[op].offset;
}

const char *
ivm_op_table_getName(ivm_opcode_t op)
{
	checkLegal()
	return ivm_global_op_table[op].name;
}
