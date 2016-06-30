#ifndef _IVM_UTIL_PARSER_H_
#define _IVM_UTIL_PARSER_H_

#include "pub/com.h"
#include "pub/type.h"
#include "pub/vm.h"

#include "std/list.h"

IVM_COM_HEADER

ivm_double_t
ivm_parser_parseNum(const ivm_char_t *src,
					ivm_size_t len,
					ivm_bool_t *err);

ivm_char_t *
ivm_parser_parseStr(const ivm_char_t *str,
					ivm_size_t len);

IVM_COM_END

#endif
