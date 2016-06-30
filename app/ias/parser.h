#ifndef _IVM_APP_IAS_PARSER_H_
#define _IVM_APP_IAS_PARSER_H_

#include "pub/com.h"
#include "pub/type.h"
#include "pub/vm.h"

#include "std/list.h"

#include "gen.h"

IVM_COM_HEADER

ias_gen_env_t *
ias_parser_parseSource(const ivm_char_t *src);

IVM_COM_END

#endif
