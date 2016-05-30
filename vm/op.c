#include "pub/const.h"
#include "pub/com.h"
#include "op.h"
#include "coro.h"
#include "vmstack.h"
#include "vm.h"
#include "call.h"
#include "byte.h"
#include "num.h"
#include "instr.h"
#include "err.h"

#define OP_PROC_NAME(op) ivm_op_proc_##op
#define OP_PROC(op) ivm_action_t OP_PROC_NAME(op)(ivm_vmstate_t *__ivm_state__, \
												  ivm_coro_t *__ivm_coro__, \
												  ivm_vmstack_t *__ivm_stack__, \
												  ivm_ctchain_t *__ivm_context__, \
												  ivm_string_pool_t *__ivm_pool__, \
												  ivm_instr_t **__ivm_instr__)
												  /* these arguments shouldn't be used directly */

#define CORO (__ivm_coro__)
#define RUNTIME (__ivm_coro__->runtime)
#define CONTEXT (__ivm_context__)
#define STATE (__ivm_state__)
#define FRAME_STACK (__ivm_coro__->frame_st)
#define STRING_POOL (__ivm_pool__)
#define INSTR (*__ivm_instr__)

#define ARG (INSTR->arg)

#define STACK (__ivm_stack__)
#define STACK_SIZE() (ivm_vmstack_size(STACK))
#define STACK_TOP() (ivm_vmstack_top(STACK))
#define STACK_POP() (ivm_vmstack_pop(STACK))
#define STACK_PUSH(obj) (ivm_vmstack_push(STACK, (obj)))
#define STACK_BEFORE(i) (ivm_vmstack_before(STACK, (i)))
#define STACK_CUT(i) (ivm_vmstack_cut(STACK, (i)))
#define STACK_INC(i) \
	(ivm_vmstack_setTop(STACK, \
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
	INSTR++;

	return IVM_ACTION_NONE;
}

OP_PROC(NEW_NULL)
{
	STACK_PUSH(IVM_NULL_OBJ(STATE));
	INSTR++;

	return IVM_ACTION_NONE;
}

OP_PROC(NEW_OBJ)
{
	STACK_PUSH(ivm_object_new(STATE));
	INSTR++;

	return IVM_ACTION_NONE;
}

OP_PROC(NEW_NUM_I)
{
	STACK_PUSH(ivm_numeric_new(STATE, ARG));
	INSTR++;

	return IVM_ACTION_NONE;
}

OP_PROC(NEW_NUM_S)
{
	INSTR++;
	return IVM_ACTION_NONE;
}

OP_PROC(NEW_FUNC)
{
	ivm_function_t *func = ivm_vmstate_getFunc(STATE, ARG);

	STACK_PUSH(ivm_function_object_new_nc(STATE, CONTEXT, func));
	INSTR++;

	return IVM_ACTION_NONE;
}

OP_PROC(GET_SLOT)
{
	const ivm_char_t *key = ivm_string_pool_get(STRING_POOL, ARG);
	ivm_object_t *obj;

	CHECK_STACK(1);

	obj = ivm_object_getSlotValue(STACK_POP(), STATE, key);
	
	STACK_PUSH(obj ? obj : IVM_UNDEFINED(STATE));

	INSTR++;

	return IVM_ACTION_NONE;
}

OP_PROC(SET_SLOT)
{
	const ivm_char_t *key = ivm_string_pool_get(STRING_POOL, ARG);
	ivm_object_t *obj, *val;

	CHECK_STACK(2);

	obj = STACK_POP();
	val = STACK_POP();
	
	if (obj) {
		ivm_object_setSlot(obj, STATE, key, val);
	}
	
	STACK_PUSH(obj);
	INSTR++;

	return IVM_ACTION_NONE;
}

OP_PROC(GET_CONTEXT_SLOT)
{
	const ivm_char_t *key = ivm_string_pool_get(STRING_POOL, ARG);
	ivm_object_t *found = ivm_ctchain_search(CONTEXT, STATE, key);

	STACK_PUSH(found ? found : IVM_UNDEFINED(STATE));
	INSTR++;

	return IVM_ACTION_NONE;
}

OP_PROC(SET_CONTEXT_SLOT)
{
	const ivm_char_t *key = ivm_string_pool_get(STRING_POOL, ARG);
	ivm_object_t *val;

	CHECK_STACK(1);

	val = STACK_POP();

	if (!ivm_ctchain_setSlotIfExist(CONTEXT, STATE, key, val)) {
		ivm_ctchain_setLocalSlot(CONTEXT, STATE, key, val);
	}

	INSTR++;

	return IVM_ACTION_NONE;
}

OP_PROC(SET_ARG)
{
	const ivm_char_t *key = ivm_string_pool_get(STRING_POOL, ARG);

	ivm_ctchain_setLocalSlot(CONTEXT, STATE, key,
							 AVAIL_STACK >= 1
							 ? STACK_POP()
							 : IVM_UNDEFINED(STATE));

	INSTR++;

	return IVM_ACTION_NONE;
}

OP_PROC(POP)
{
	CHECK_STACK(1);
	STACK_POP();
	INSTR++;

	return IVM_ACTION_NONE;
}

OP_PROC(DUP)
{
	ivm_op_arg_t i = ARG;
	CHECK_STACK(i + 1);
	STACK_PUSH(STACK_BEFORE(i));
	INSTR++;

	return IVM_ACTION_NONE;
}

OP_PROC(PRINT_OBJ)
{
	ivm_object_t *obj;

	CHECK_STACK(1);

	obj = STACK_POP();
	printf("print: %p\n", (void *)obj);
	INSTR++;

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

	INSTR++;

	return IVM_ACTION_NONE;
}

OP_PROC(PRINT_TYPE)
{
	ivm_object_t *obj;

	CHECK_STACK(1);

	obj = STACK_POP();
	printf("type: %s\n", obj ? IVM_OBJECT_GET(obj, TYPE_NAME) : "empty pointer");
	
	INSTR++;

	return IVM_ACTION_NONE;
}

OP_PROC(PRINT_STR)
{
	printf("%s\n", ivm_string_pool_get(STRING_POOL, ARG));
	INSTR++;

	return IVM_ACTION_NONE;
}

OP_PROC(INVOKE)
{
	ivm_function_object_t *obj;
	ivm_function_t *func;
	ivm_sint32_t arg_count = ARG;
	ivm_vmstack_iterator_t args;
	ivm_object_t *ret;

	CHECK_STACK(arg_count + 1);

	obj = IVM_AS(STACK_POP(), ivm_function_object_t);

	IVM_ASSERT(IVM_IS_TYPE(obj, IVM_FUNCTION_OBJECT_T),
			   IVM_ERROR_MSG_NOT_TYPE("function", IVM_OBJECT_GET(obj, TYPE_NAME)));
	
	func = ivm_function_object_getFunc(obj);
	args = STACK_CUT(arg_count);

	IVM_RUNTIME_SET(RUNTIME, IP, INSTR + 1);

	ivm_function_invoke(func, STATE,
						ivm_function_object_getClosure(obj), CORO);

	if (ivm_function_isNative(func)) {
		ret = ivm_function_callNative(func, STATE, CONTEXT,
									  IVM_FUNCTION_SET_ARG_2(arg_count, args));
		STACK_PUSH(ret ? ret : IVM_NULL_OBJ(STATE));
	} else {
		STACK_INC(arg_count);
	}

	return IVM_ACTION_INVOKE;
}

OP_PROC(YIELD)
{
	CHECK_STACK(1);
	INSTR++;

	return IVM_ACTION_YIELD;
}

OP_PROC(JUMP)
{
	INSTR += ARG;
	return IVM_ACTION_NONE;
}

OP_PROC(JUMP_IF_TRUE)
{
	if (ivm_object_toBool(STACK_POP(), STATE)) {
		INSTR += ARG;
	} else {
		INSTR++;
	}

	return IVM_ACTION_NONE;
}

OP_PROC(JUMP_IF_FALSE)
{
	if (!ivm_object_toBool(STACK_POP(), STATE)) {
		INSTR += ARG;
	} else {
		INSTR++;
	}

	return IVM_ACTION_NONE;
}

OP_PROC(TEST1)
{
	printf("test1\n");
	INSTR++;

	return IVM_ACTION_NONE;
}

OP_PROC(TEST2)
{ 
	INSTR++;
	return IVM_ACTION_NONE;
}

OP_PROC(TEST3)
{
	printf("morning! this is test3\n");
	printf("string argument: %s\n", ivm_string_pool_get(STRING_POOL, ARG));
	INSTR++;

	return IVM_ACTION_NONE;
}

OP_PROC(LAST)
{
	printf("LAST\n");
	INSTR++;

	return IVM_ACTION_NONE;
}

IVM_PRIVATE const
ivm_op_table_t
ivm_global_op_table[] = {
	OP_MAPPING(NOP,					"nop",					"N"),
	OP_MAPPING(NEW_NULL,			"new_null",				"N"),
	OP_MAPPING(NEW_OBJ,				"new_obj",				"N"),
	OP_MAPPING(NEW_NUM_I,			"new_num_i",			"I"),
	OP_MAPPING(NEW_NUM_S,			"new_num_s",			"S"),
	OP_MAPPING(NEW_FUNC,			"new_func",				"I"),
	OP_MAPPING(GET_SLOT,			"get_slot",				"S"),
	OP_MAPPING(SET_SLOT,			"set_slot",				"S"),
	OP_MAPPING(GET_CONTEXT_SLOT,	"get_context_slot",		"S"),
	OP_MAPPING(SET_CONTEXT_SLOT,	"set_context_slot",		"S"),
	OP_MAPPING(SET_ARG,				"set_arg",				"S"),
	OP_MAPPING(POP,					"pop",					"N"),
	OP_MAPPING(DUP,					"dup",					"N"),
	OP_MAPPING(PRINT_OBJ,			"print_obj",			"N"),
	OP_MAPPING(PRINT_NUM,			"print_num",			"N"),
	OP_MAPPING(PRINT_TYPE,			"print_type",			"N"),
	OP_MAPPING(PRINT_STR,			"print_str",			"S"),
	OP_MAPPING(INVOKE,				"invoke",				"I"),
	OP_MAPPING(YIELD,				"yield",				"N"),
	OP_MAPPING(JUMP,				"jmp",					"I"),
	OP_MAPPING(JUMP_IF_TRUE,		"jmp_true",				"I"),
	OP_MAPPING(JUMP_IF_FALSE,		"jmp_false",			"I"),
	OP_MAPPING(TEST1,				"test1",				"N"),
	OP_MAPPING(TEST2,				"test2",				"N"),
	OP_MAPPING(TEST3,				"test3",				"S"),
	OP_MAPPING(LAST,				"last",					"N")
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
