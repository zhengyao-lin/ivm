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

ivm_serial_exec_list_t *
ivm_serial_serializeExecList(ivm_exec_list_t *list);

void
ivm_serial_exec_list_free(ivm_serial_exec_list_t *list);

ivm_size_t
ivm_serial_execListToFile(ivm_serial_exec_list_t *list,
						  ivm_file_t *file);

ivm_serial_exec_list_t *
ivm_serial_execListFromFile(ivm_file_t *file);

IVM_COM_END

#endif
