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
	(ivm_vmstack_incTop(STACK, (i)))

#define FRAME_STACK_TOP() (ivm_frame_stack_top(__ivm_coro__->frame_st))
#define AVAIL_STACK (STACK_SIZE() - IVM_FRAME_GET(FRAME_STACK_TOP(), STACK_TOP))
#define CHECK_STACK(req) IVM_ASSERT(AVAIL_STACK >= (req), \
									IVM_ERROR_MSG_INSUFFICIENT_STACK)

#define OP_MAPPING(op, name, args) { IVM_OP(op), OP_PROC_NAME(op), (name), (args) }

#define OP_GEN(o, name, arg, ...) OP_PROC(o) __VA_ARGS__
	#include "op.def"
#undef OP_GEN

IVM_PRIVATE const
ivm_op_table_t
ivm_global_op_table[] = {

#define OP_GEN(o, name, arg, ...) OP_MAPPING(o, name, #arg),
	#include "op.def"
#undef OP_GEN

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
	checkLegal();
	return ivm_global_op_table[op].args;
}

const char *
ivm_op_table_getName(ivm_opcode_t op)
{
	checkLegal();
	return ivm_global_op_table[op].name;
}
