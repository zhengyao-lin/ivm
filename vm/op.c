#include "pub/const.h"
#include "pub/com.h"
#include "pub/err.h"
#include "pub/vm.h"

#include "op.h"
#include "coro.h"
#include "vmstack.h"
#include "call.h"
#include "instr.h"

#include "op.req.h"

#if IVM_DISPATCH_METHOD_CALL_THREAD
	/* code for call threading */
	#define OP_PROC_NAME(op) ivm_op_proc_##op
	#define OP_PROC(op) ivm_action_t OP_PROC_NAME(op)(ivm_vmstate_t *__ivm_state__, \
													  ivm_coro_t *__ivm_coro__, \
													  ivm_vmstack_t *__ivm_stack__, \
													  ivm_ctchain_t *__ivm_context__, \
													  ivm_string_pool_t *__ivm_pool__, \
													  ivm_instr_t **__ivm_instr__)
													  /* these arguments shouldn't be used directly */

	#include "dispatch/call.h"

	#define OP_MAPPING(op, name, args) { IVM_OP(op), OP_PROC_NAME(op), (name), (args), IVM_NULL }

	#define OP_GEN(o, name, arg, ...) OP_PROC(o) __VA_ARGS__
		#include "op.def.h"
	#undef OP_GEN
#else
	/* non-call threading: don't need handler/proc */
	#define OP_MAPPING(op, name, args) { IVM_OP(op), IVM_NULL, (name), (args), IVM_NULL }
#endif

IVM_PRIVATE
ivm_op_table_t
op_table[] = {

#define OP_GEN(o, name, arg, ...) OP_MAPPING(o, name, #arg),
	#include "op.def.h"
#undef OP_GEN

};

#if IVM_DEBUG

#define checkLegal() \
	IVM_ASSERT(op < IVM_OP_LAST, \
			   IVM_ERROR_MSG_BAD_OP); \
	IVM_ASSERT(op_table[op].op == op, \
			   IVM_ERROR_MSG_BAD_OP_TABLE);

#else

#define checkLegal()

#endif

ivm_op_proc_t
ivm_op_table_getProc(ivm_opcode_t op)
{
	checkLegal();
	return op_table[op].proc;
}

const char *
ivm_op_table_getArg(ivm_opcode_t op)
{
	checkLegal();
	return op_table[op].args;
}

const char *
ivm_op_table_getName(ivm_opcode_t op)
{
	checkLegal();
	return op_table[op].name;
}

#if IVM_DISPATCH_METHOD_DIRECT_THREAD

void
ivm_op_table_initOpEntry()
{
	void **table = ivm_coro_getOpEntry();
	ivm_int_t i;

	for (i = 0; i <= IVM_OP(LAST); i++) {
		op_table[i].entry = table[i];
	}

	return;
}

void *
ivm_op_table_getEntry(ivm_opcode_t op)
{
	checkLegal();
	return op_table[op].entry;
}

#endif
