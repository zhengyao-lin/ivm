#ifndef _IVM_APP_ILANG_PARSER_H_
#define _IVM_APP_ILANG_PARSER_H_

#include "pub/com.h"
#include "pub/type.h"
#include "pub/vm.h"

#include "std/list.h"

#include "gen/gen.h"

IVM_COM_HEADER

ilang_gen_trans_unit_t *
ilang_parser_parseSource(const ivm_char_t *file,
						 const ivm_char_t *str,
						 ivm_bool_t debug);

ilang_gen_expr_t *
ilang_parser_parseExpr(ilang_gen_trans_unit_t *unit,
					   const ivm_char_t *src,
					   ivm_size_t lineno);

IVM_COM_END

#endif
