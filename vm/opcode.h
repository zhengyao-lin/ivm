#ifndef _IVM_VM_OPCODE_H_
#define _IVM_VM_OPCODE_H_

#include "pub/com.h"
#include "pub/type.h"

#include "std/string.h"

#include "vmstack.h"

IVM_COM_HEADER

struct ivm_coro_t_tag;
struct ivm_vmstate_t_tag;
struct ivm_ctchain_t_tag;
struct ivm_exec_t_tag;
struct ivm_instr_t_tag;

#define IVM_OPCODE(name) IVM_OPCODE_##name

typedef enum {
	IVM_OPCODE(FIRST) = -1, /* to make NOP the 0 */
#define OPCODE_GEN(o, name, arg, ...) IVM_OPCODE(o),
	#include "opcode.def.h"
#undef OPCODE_GEN
	IVM_OPCODE(LAST)
} ivm_opcode_t;

typedef ivm_int_t (*ivm_opcode_handler_t)(struct ivm_vmstate_t_tag *state,
										  struct ivm_coro_t_tag *coro,
										  ivm_vmstack_t *stack,
										  struct ivm_ctchain_t_tag *context,
										  ivm_string_pool_t *pool,
										  struct ivm_instr_t_tag **instr);

typedef struct {
	ivm_opcode_t opc;
	const ivm_char_t *name;
	const ivm_char_t *param;
	void *entry; /* in direct threading */
} ivm_opcode_entry_t;

const ivm_char_t *
ivm_opcode_table_getParam(ivm_opcode_t opc);
const ivm_char_t *
ivm_opcode_table_getName(ivm_opcode_t opc);

#if IVM_DISPATCH_METHOD_DIRECT_THREAD

/* must-call when using direct threading */
void
ivm_opcode_table_initOpEntry();
void *
ivm_opcode_table_getEntry(ivm_opcode_t opc);

#endif

/* return LAST if the name is not found */
ivm_opcode_t
ivm_opcode_searchOp(const ivm_char_t *name);

ivm_opcode_t
ivm_opcode_searchOp_len(const ivm_char_t *name,
						ivm_size_t len);

IVM_COM_END

#endif
