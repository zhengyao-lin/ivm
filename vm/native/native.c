#include <stdarg.h>

#include "pub/type.h"
#include "pub/const.h"
#include "pub/vm.h"
#include "pub/inlines.h"

#include "vm/obj.h"
#include "vm/num.h"
#include "vm/strobj.h"
#include "vm/func.h"
#include "vm/listobj.h"

#include "native.h"

/*
	rules:                                                                      pass in type
		1. 'n': check num type and get the value                             -- ivm_number_t *
		2. 's': check string type and get the value                          -- const ivm_string_t **
		3. 'l': check list type and convert to list object                   -- ivm_list_object_t **
		4. 'f': check function type and convert to function object           -- ivm_function_object_t **
		5. 'b': check buffer type and convert to buffer object               -- ivm_buffer_object_t **
		6. '.': no type check but accept the object                          -- ivm_object_t **
		7. '*' prefix: following argument are all optional,
			   but if they exist, the type will be checked[1][2]             -- \

		NOTE:
			[1]. return address passed to an optional argument need to be initialized
			[2]. optional mark could only appear once
 */

ivm_int_t
ivm_native_matchArgument(ivm_function_arg_t arg,
						 ivm_vmstate_t *state,
						 const ivm_char_t *rule, ...)
{
	ivm_bool_t next_opt = IVM_FALSE;
	ivm_int_t ret = 0;
	ivm_argc_t i;
	ivm_object_t *tmp;
	va_list args;

	va_start(args, rule);

#define SUB1(r, type, cvt, val) \
	case r:                                           \
		if (ivm_function_arg_has(arg, i)) {           \
			tmp = ivm_function_arg_at(arg, i);        \
			if (IVM_IS_BTTYPE(tmp, state, (type))) {  \
				*((cvt *)va_arg(args, cvt *))         \
				= (val);                              \
			} else {                                  \
				ret = i;                              \
				goto END;                             \
			}                                         \
			i++;                                      \
		} else {                                      \
			if (!next_opt) ret = i;                   \
			goto END;                                 \
		}                                             \
		break;

	for (i = 1; *rule; rule++) {
		switch (*rule) {
			SUB1('n', IVM_NUMERIC_T, ivm_number_t, ivm_numeric_getValue(tmp))
			SUB1('s', IVM_STRING_OBJECT_T, const ivm_string_t *, ivm_string_object_getValue(tmp))
			SUB1('l', IVM_LIST_OBJECT_T, ivm_list_object_t *, IVM_AS(tmp, ivm_list_object_t))
			SUB1('f', IVM_FUNCTION_OBJECT_T, ivm_function_object_t *, IVM_AS(tmp, ivm_function_object_t))
			SUB1('b', IVM_BUFFER_OBJECT_T, ivm_buffer_object_t *, IVM_AS(tmp, ivm_buffer_object_t))

			case '.':
				if (ivm_function_arg_has(arg, i)) {
					*(va_arg(args, ivm_object_t **)) = ivm_function_arg_at(arg, i);
					i++;
				} else {
					if (!next_opt) ret = i;
					goto END;
				}
				break;

			case '*':
				IVM_ASSERT(!next_opt, IVM_ERROR_MSG_REPEAT_OPTIONAL_MARK);
				next_opt = IVM_TRUE;
				break;

			default:
				IVM_FATAL(IVM_ERROR_MSG_WRONG_NATIVE_ARG_RULE(*rule));
		}
	}

#undef SUB1

END:
	va_end(args);

	return ret;
}

ivm_object_t *
IVM_NATIVE_WRAP_CONS_c(ivm_vmstate_t *state,
					   ivm_object_t *proto,
					   ivm_native_function_t func)
{
	ivm_object_t *ret = ivm_function_object_newNative(state, func);
	ivm_object_setProto(ret, state, proto);

	return ret;
}
