#include "pub/const.h"
#include "pub/com.h"
#include "pub/err.h"
#include "pub/vm.h"

#include "inline/vm.h"
#include "op.h"
#include "coro.h"
#include "vmstack.h"
#include "call.h"
#include "instr.h"

#include "op.req.h"

#define OP_MAPPING(op, name, args) { IVM_OP(op), IVM_NULL, (name), (args), IVM_NULL }

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
