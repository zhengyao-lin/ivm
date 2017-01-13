#ifndef _IVM_UTIL_SERIAL_H_
#define _IVM_UTIL_SERIAL_H_

#include "pub/com.h"
#include "pub/const.h"
#include "pub/type.h"
#include "pub/vm.h"

#include "std/string.h"
#include "std/io.h"

IVM_COM_HEADER

ivm_exec_unit_t *
ivm_serial_decodeCache(ivm_file_t *file);

void
ivm_serial_encodeCache(ivm_exec_unit_t *unit,
					   ivm_file_t *file);

ivm_object_t *
ivm_serial_mod_loadCache(const ivm_char_t *path,
						 ivm_char_t **err,
						 ivm_bool_t *is_const,
						 ivm_vmstate_t *state,
						 ivm_coro_t *coro,
						 ivm_context_t *context);

IVM_COM_END

#endif
