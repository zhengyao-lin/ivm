#include "pub/const.h"
#include "pub/com.h"
#include "pub/err.h"
#include "pub/vm.h"

#include "inline/vm.h"
#include "opcode.h"
#include "coro.h"
#include "vmstack.h"
#include "call.h"
#include "instr.h"

#include "opcode.req.h"

#define OPCODE_MAPPING(op, name, args) { IVM_OPCODE(op), (name), (args), IVM_NULL }

IVM_PRIVATE
ivm_opcode_entry_t
opcode_table[] = {

#define OPCODE_GEN(o, name, arg, ...) OPCODE_MAPPING(o, name, #arg),
	#include "opcode.def.h"
#undef OPCODE_GEN

};

#if IVM_DEBUG

#define checkLegal(o) \
	IVM_ASSERT((o) < IVM_OPCODE(LAST), \
			   IVM_ERROR_MSG_BAD_OPCODE); \
	IVM_ASSERT(opcode_table[o].opc == (o), \
			   IVM_ERROR_MSG_BAD_OPCODE_TABLE);

#else

#define checkLegal(o)

#endif

const char *
ivm_opcode_table_getArg(ivm_opcode_t opc)
{
	checkLegal(opc);
	return opcode_table[opc].args;
}

const char *
ivm_opcode_table_getName(ivm_opcode_t opc)
{
	checkLegal(opc);
	return opcode_table[opc].name;
}

#if IVM_DISPATCH_METHOD_DIRECT_THREAD

void
ivm_opcode_table_initOpEntry()
{
	void **table = ivm_coro_getOpcodeEntry();
	ivm_int_t i;

	for (i = 0; i <= IVM_OPCODE(LAST); i++) {
		opcode_table[i].entry = table[i];
	}

	return;
}

void *
ivm_opcode_table_getEntry(ivm_opcode_t opc)
{
	checkLegal(opc);
	return opcode_table[opc].entry;
}

#endif
