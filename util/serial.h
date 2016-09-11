#ifndef _IVM_UTIL_SERIAL_H_
#define _IVM_UTIL_SERIAL_H_

#include "pub/com.h"
#include "pub/const.h"
#include "pub/type.h"
#include "pub/vm.h"

#include "std/string.h"
#include "std/io.h"

IVM_COM_HEADER

typedef struct {
	ivm_byte_t opc;
	ivm_opcode_arg_t arg;
} IVM_NOALIGN ivm_serial_instr_t;

typedef struct {
	ivm_uint64_t pool;
	ivm_uint64_t size;
	ivm_serial_instr_t *instrs;
} IVM_NOALIGN ivm_serial_exec_t;

typedef struct {
	ivm_string_pool_list_t *pool_list;
	ivm_uint64_t size;
	ivm_serial_exec_t *execs;
} IVM_NOALIGN ivm_serial_exec_list_t;

typedef struct {
	ivm_uint32_t offset;
	ivm_uint64_t root;
	ivm_serial_exec_list_t *list;
} IVM_NOALIGN ivm_serial_exec_unit_t;

ivm_serial_exec_unit_t *
ivm_serial_serializeExecUnit(ivm_exec_unit_t *unit, ivm_vmstate_t *state);

ivm_exec_unit_t *
ivm_serial_unserializeExecUnit(ivm_serial_exec_unit_t *unit);

void
ivm_serial_exec_unit_free(ivm_serial_exec_unit_t *unit);

ivm_size_t
ivm_serial_execUnitToFile(ivm_serial_exec_unit_t *unit,
						  ivm_file_t *file);

ivm_serial_exec_unit_t *
ivm_serial_execUnitFromFile(ivm_file_t *file);

IVM_INLINE
ivm_exec_unit_t *
ivm_serial_parseCacheFile(ivm_file_t *file)
{
	ivm_exec_unit_t *ret = IVM_NULL;
	ivm_serial_exec_unit_t *s_unit;

	s_unit = ivm_serial_execUnitFromFile(file);
	if (s_unit) {
		ret = ivm_serial_unserializeExecUnit(s_unit);
		ivm_serial_exec_unit_free(s_unit);
	}

	return ret;
}

ivm_object_t *
ivm_serial_mod_loadCache(const ivm_char_t *path,
						 ivm_char_t **err,
						 ivm_bool_t *is_const,
						 ivm_vmstate_t *state,
						 ivm_coro_t *coro,
						 ivm_context_t *context);

IVM_COM_END

#endif
