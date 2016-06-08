#ifndef _IVM_VM_INSTR_H_
#define _IVM_VM_INSTR_H_

#include "pub/const.h"
#include "pub/type.h"

#include "std/string.h"
#include "op.h"

IVM_COM_HEADER

struct ivm_exec_t_tag;

#if IVM_DISPATCH_METHOD_DIRECT_THREAD

typedef struct ivm_instr_t_tag {
	void *entry;
	ivm_op_arg_t arg;
	ivm_byte_t op;
} ivm_instr_t;

#elif IVM_DISPATCH_METHOD_CALL_THREAD

typedef struct ivm_instr_t_tag {
	ivm_op_proc_t proc;
	ivm_op_arg_t arg;
	ivm_byte_t op;
} ivm_instr_t;

#endif

#define IVM_INSTR_TYPE_N_ARG
#define IVM_INSTR_TYPE_I_ARG ivm_op_arg_t arg,
#define IVM_INSTR_TYPE_S_ARG const char *str,

#define IVM_INSTR_GEN(op, ...) \
	(ivm_instr_gen_##op(__VA_ARGS__))

#define OP_GEN(op, name, arg, ...) \
	ivm_instr_t ivm_instr_gen_##op(IVM_INSTR_TYPE_##arg##_ARG \
								   struct ivm_exec_t_tag *exec);

	#include "op.def.h"

#undef OP_GEN

IVM_COM_END

#endif
