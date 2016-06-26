#include "pub/const.h"
#include "pub/mem.h"
#include "pub/err.h"

#include "std/string.h"

#include "opcode.h"
#include "instr.h"
#include "exec.h"

#define INSTR_TYPE_N_ARG_INIT(instr, exec) \
	ivm_opcode_arg_fromInt(0)

#define INSTR_TYPE_I_ARG_INIT(instr, exec) \
	ivm_opcode_arg_fromInt(arg)

#define INSTR_TYPE_X_ARG_INIT(instr, exec) \
	ivm_opcode_arg_fromInt(arg)

#define INSTR_TYPE_F_ARG_INIT(instr, exec) \
	ivm_opcode_arg_fromFloat(arg)

#define INSTR_TYPE_S_ARG_INIT(instr, exec) \
	ivm_opcode_arg_fromPointer(ivm_exec_registerString((exec), str))

#if IVM_DISPATCH_METHOD_DIRECT_THREAD
	#define OPCODE_GEN(o, name, arg, ...) \
		ivm_instr_t ivm_instr_gen_##o(IVM_INSTR_TYPE_##arg##_ARG \
									  ivm_exec_t *exec) \
		{ \
			return (ivm_instr_t) { \
				IVM_OPCODE(o), \
				INSTR_TYPE_##arg##_ARG_INIT((ret), (exec)), \
				ivm_opcode_table_getEntry(IVM_OPCODE(o)) \
			}; \
		}

		#include "opcode.def.h"

	#undef OPCODE_GEN
#else
	#define OPCODE_GEN(o, name, arg, ...) \
		ivm_instr_t ivm_instr_gen_##o(IVM_INSTR_TYPE_##arg##_ARG \
									  ivm_exec_t *exec) \
		{ \
			return (ivm_instr_t) { \
				IVM_OPCODE(o), \
				INSTR_TYPE_##arg##_ARG_INIT((ret), (exec)) \
			}; \
		}

		#include "opcode.def.h"

	#undef OPCODE_GEN
#endif
