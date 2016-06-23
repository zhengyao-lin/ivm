#include "pub/const.h"
#include "pub/com.h"
#include "pub/err.h"
#include "pub/vm.h"
#include "pub/inlines.h"

#include "std/string.h"

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

const ivm_char_t *
ivm_opcode_table_getParam(ivm_opcode_t opc)
{
	checkLegal(opc);
	return opcode_table[opc].param;
}

const ivm_char_t *
ivm_opcode_table_getName(ivm_opcode_t opc)
{
	checkLegal(opc);
	return opcode_table[opc].name;
}

#define TABLE_SIZE (sizeof(opcode_table) / sizeof(*opcode_table))

#if IVM_DISPATCH_METHOD_DIRECT_THREAD

void
ivm_opcode_table_initOpEntry()
{
	void **table = ivm_coro_getOpcodeEntry();
	ivm_int_t i;

	for (i = 0; i < TABLE_SIZE; i++) {
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

ivm_opcode_t
ivm_opcode_searchOp(const ivm_char_t *name)
{
	ivm_int_t i;

	for (i = 0; i < TABLE_SIZE; i++) {
		if (!IVM_STRCMP(opcode_table[i].name, name)) {
			return opcode_table[i].opc;
		}
	}

	return IVM_OPCODE(LAST);
}

ivm_opcode_t
ivm_opcode_searchOp_len(const ivm_char_t *name,
						ivm_size_t len)
{
	ivm_int_t i;

	for (i = 0; i < TABLE_SIZE; i++) {
		if (!IVM_STRNCMP(opcode_table[i].name,
						 IVM_STRLEN(opcode_table[i].name),
						 name, len)) {
			return opcode_table[i].opc;
		}
	}

	return IVM_OPCODE(LAST);
}
