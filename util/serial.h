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
	ivm_uint64_t root;
	ivm_serial_exec_list_t *list;
} IVM_NOALIGN ivm_serial_exec_unit_t;

ivm_serial_exec_unit_t *
ivm_serial_serializeExecUnit(ivm_exec_unit_t *unit);

ivm_exec_unit_t *
ivm_serial_unserializeExecUnit(ivm_serial_exec_unit_t *unit);

void
ivm_serial_exec_unit_free(ivm_serial_exec_unit_t *unit);

ivm_size_t
ivm_serial_execUnitToFile(ivm_serial_exec_unit_t *unit,
						  ivm_file_t *file);

ivm_serial_exec_unit_t *
ivm_serial_execUnitFromFile(ivm_file_t *file);

IVM_COM_END

#endif
