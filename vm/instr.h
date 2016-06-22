#ifndef _IVM_VM_INSTR_H_
#define _IVM_VM_INSTR_H_

#include "pub/const.h"
#include "pub/type.h"

#include "std/string.h"

#include "opcode.h"

IVM_COM_HEADER

struct ivm_exec_t_tag;

#if IVM_DISPATCH_METHOD_DIRECT_THREAD

typedef struct ivm_instr_t_tag {
	void *entry;
	ivm_byte_t opc;
	ivm_opcode_arg_t arg;
	ivm_opcode_cache_t cache;
} ivm_instr_t;

#else

typedef struct ivm_instr_t_tag {
	ivm_byte_t opc;
	ivm_opcode_arg_t arg;
	ivm_opcode_cache_t cache;
} ivm_instr_t;

#endif

#if IVM_DISPATCH_METHOD_DIRECT_THREAD

#define ivm_instr_build(opc, ...) ((ivm_instr_t) { .entry = ivm_opcode_table_getEntry(opc), .opc = (opc), __VA_ARGS__ })

#define ivm_instr_entry(instr) ((instr)->entry)

#else

#define ivm_instr_build(opc, ...) ((ivm_instr_t) { .opc = (opc), ##__VA_ARGS__ })

#endif

#define ivm_instr_arg(instr) ((instr)->arg)
#define ivm_instr_setArg(instr, val) ((instr)->arg = (val))
#define ivm_instr_opcode(instr) ((instr)->opc)

#define IVM_INSTR_TYPE_N_ARG 						/* none */
#define IVM_INSTR_TYPE_I_ARG ivm_long_t arg,		/* int */
#define IVM_INSTR_TYPE_X_ARG ivm_long_t arg,		/* exec */
#define IVM_INSTR_TYPE_F_ARG ivm_double_t arg,		/* float */
#define IVM_INSTR_TYPE_S_ARG const char *str,		/* string */

#define IVM_INSTR_GEN(o, ...) \
	(ivm_instr_gen_##o(__VA_ARGS__))

#define OPCODE_GEN(o, name, arg, ...) \
	ivm_instr_t ivm_instr_gen_##o(IVM_INSTR_TYPE_##arg##_ARG \
								  struct ivm_exec_t_tag *exec);

	#include "opcode.def.h"

#undef OPCODE_GEN

IVM_COM_END

#endif
