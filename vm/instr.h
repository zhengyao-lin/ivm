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
	ivm_opcode_arg_t arg;
	ivm_byte_t opc;
} ivm_instr_t;

#else

typedef struct ivm_instr_t_tag {
	ivm_byte_t opc;
	ivm_opcode_arg_t arg;
} ivm_instr_t;

#endif

#define IVM_INSTR_TYPE_N_ARG
#define IVM_INSTR_TYPE_I_ARG ivm_opcode_arg_t arg,
#define IVM_INSTR_TYPE_S_ARG const char *str,

#define IVM_INSTR_GEN(o, ...) \
	(ivm_instr_gen_##o(__VA_ARGS__))

#define OPCODE_GEN(o, name, arg, ...) \
	ivm_instr_t ivm_instr_gen_##o(IVM_INSTR_TYPE_##arg##_ARG \
								  struct ivm_exec_t_tag *exec);

	#include "opcode.def.h"

#undef OPCODE_GEN

IVM_COM_END

#endif
