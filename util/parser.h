#ifndef _IVM_UTIL_PARSER_H_
#define _IVM_UTIL_PARSER_H_

#include "pub/com.h"
#include "pub/type.h"
#include "pub/vm.h"

#include "std/list.h"

IVM_COM_HEADER

ivm_list_t *
_ivm_parser_getTokens(const ivm_char_t *src);

IVM_COM_END

#endif

#if 0
opcode -> t_identifier

instr -> opcode arg1[, arg2[, ...]] '\n'
#endif
