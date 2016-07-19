#ifndef _IVM_VM_INSTR_H_
#define _IVM_VM_INSTR_H_

#include "pub/const.h"
#include "pub/type.h"

#include "std/string.h"
#include "std/uid.h"

#include "opcode.h"

IVM_COM_HEADER

struct ivm_exec_t_tag;

/*
typedef struct {
	ivm_uid_t id;
	ivm_ptr_t data;
} ivm_instr_cache_t;

#define ivm_instr_cache_build(id, data) ((ivm_instr_cache_t) { (ivm_uid_t)(id), (ivm_ptr_t)(data) })

#define ivm_instr_cache_id(cache) ((cache)->id)
#define ivm_instr_cache_data(cache) ((cache)->data)
*/

#if IVM_DISPATCH_METHOD_DIRECT_THREAD

typedef struct ivm_instr_t_tag {
	void *entry;
	ivm_opcode_arg_t arg;
	ivm_ptr_t cc_data;
	ivm_uid_t cc_id;
	ivm_byte_t opc;
} ivm_instr_t;

#define ivm_instr_cacheID(instr) ((instr)->cc_id)
#define ivm_instr_cacheData(instr) ((instr)->cc_data)

#define ivm_instr_setCacheID(instr, id) ((instr)->cc_id = (id))
#define ivm_instr_setCacheData(instr, data) ((instr)->cc_data = (data))

IVM_INLINE
void
ivm_instr_setCache(ivm_instr_t *instr, ivm_uid_t id, ivm_ptr_t data)
{
	instr->cc_id = id;
	instr->cc_data = data;
	return;
}

#else

typedef struct ivm_instr_t_tag {
	ivm_byte_t opc;
	ivm_opcode_arg_t arg;
	ivm_instr_cache_t cache;
} ivm_instr_t;

#endif

#if IVM_DISPATCH_METHOD_DIRECT_THREAD

#define ivm_instr_build(o, a) ((ivm_instr_t) { .opc = (o), .arg = (a), .entry = ivm_opcode_table_getEntry(o) })

#define ivm_instr_entry(instr) ((instr)->entry)

#else

#define ivm_instr_build(o, a) ((ivm_instr_t) { .opc = (o), .arg = (a), ##__VA_ARGS__ })

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

#define OPCODE_GEN(o, name, arg, st_inc, ...) \
	ivm_instr_t ivm_instr_gen_##o(IVM_INSTR_TYPE_##arg##_ARG \
								  struct ivm_exec_t_tag *exec);

	#include "opcode.def.h"

#undef OPCODE_GEN

IVM_COM_END

#endif
