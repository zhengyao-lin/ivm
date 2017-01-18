#ifndef _IVM_VM_OPRT_REQ_H_
#define _IVM_VM_OPRT_REQ_H_

#include "pub/type.h"
#include "pub/vm.h"
#include "pub/err.h"
#include "pub/inlines.h"

#include "std/string.h"
#include "std/conv.h"
#include "std/list.h"

#include "obj.h"
#include "num.h"
#include "strobj.h"
#include "listobj.h"

#define OPRT_FATAL(...) \
	IVM_CORO_NATIVE_ASSERT(_CORO, _STATE, 0, __VA_ARGS__)

#define OPRT_ASSERT(cond, ...) \
	IVM_CORO_NATIVE_ASSERT(_CORO, _STATE, (cond), __VA_ARGS__)

#define OPRT_CHECK_OP2(type) \
	OPRT_ASSERT(                                                                           \
		IVM_IS_BTTYPE(_OP2, _STATE, (type)),                                               \
		IVM_ERROR_MSG_WRONG_OPERAND(                                                       \
			2, ivm_vmstate_getTypeName(_STATE, (type)),                                    \
			IVM_OBJECT_GET(_OP2, TYPE_NAME)                                                \
		)                                                                                  \
	)

#define LINK_STRING_NUM(op1, op2, e) \
	const ivm_string_t *str1 = ivm_string_object_getValue(op1);                \
	ivm_size_t len1 = ivm_string_length(str1), len2;                           \
	ivm_size_t size;                                                           \
	ivm_string_t *ret;                                                         \
	ivm_char_t buf[25];                                                        \
	ivm_char_t *data;                                                          \
                                                                               \
	size = len1 + (len2 = ivm_conv_dtoa(ivm_numeric_getValue(op2), buf));      \
	ret = ivm_vmstate_alloc(_STATE, IVM_STRING_GET_SIZE(size));                \
	data = ivm_string_trimHead(ret);                                           \
                                                                               \
	e;                                                                         \
                                                                               \
	ivm_string_initHead(ret, IVM_FALSE, size);                                 \
                                                                               \
	return ivm_string_object_new_c(_STATE, ret);                               \

#endif
