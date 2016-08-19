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

#define RTM_FATAL(...) \
	IVM_CORO_NATIVE_ASSERT(_CORO, _STATE, 0, __VA_ARGS__)

#define RTM_ASSERT(cond, ...) \
	IVM_CORO_NATIVE_ASSERT(_CORO, _STATE, (cond), __VA_ARGS__)

#define GET_STRING_INDEX() \
	{                                                                                         \
		ivm_object_t *tmp_obj;                                                                \
                                                                                              \
		if (_ASSIGN) {                                                                        \
			ivm_object_setSlot(_OP1, _STATE, ivm_string_object_getValue(_OP2), _ASSIGN);      \
			return _ASSIGN;                                                                   \
		}                                                                                     \
                                                                                              \
		tmp_obj = ivm_object_getSlot(_OP1, _STATE, ivm_string_object_getValue(_OP2));         \
                                                                                              \
		return tmp_obj ? tmp_obj : IVM_NULL_OBJ(_STATE);                                      \
	}

#define LINK_STRING_NUM(op1, op2) \
	{                                                                              \
		const ivm_string_t *str1 = ivm_string_object_getValue(op1);                \
		ivm_size_t len1 = ivm_string_length(str1), len2;                           \
		ivm_size_t size;                                                           \
		ivm_string_t *ret;                                                         \
		ivm_char_t buf[25];                                                        \
		ivm_char_t *data;                                                          \
	                                                                               \
		size = len1 + (len2 = ivm_dtoa(ivm_numeric_getValue(op2), buf));           \
		ret = ivm_vmstate_alloc(_STATE, IVM_STRING_GET_SIZE(size));                \
		data = ivm_string_trimHead(ret);                                           \
	                                                                               \
		MEM_COPY(data, ivm_string_trimHead(str1), len1 * sizeof(ivm_char_t));      \
		MEM_COPY(data + len1, buf, len2 * sizeof(ivm_char_t));                     \
		data[size] = '\0';                                                         \
	                                                                               \
		ivm_string_initHead(ret, IVM_FALSE, size);                                 \
	                                                                               \
		return ivm_string_object_new_c(_STATE, ret);                               \
	}

#define LINK_STRING_NUM_R(op1, op2) \
	{                                                                                 \
		const ivm_string_t *str1 = ivm_string_object_getValue(op1);                   \
		ivm_size_t len1 = ivm_string_length(str1), len2;                              \
		ivm_size_t size;                                                              \
		ivm_string_t *ret;                                                            \
		ivm_char_t buf[25];                                                           \
		ivm_char_t *data;                                                             \
	                                                                                  \
		size = len1 + (len2 = ivm_dtoa(ivm_numeric_getValue(op2), buf));              \
		ret = ivm_vmstate_alloc(_STATE, IVM_STRING_GET_SIZE(size));                   \
		data = ivm_string_trimHead(ret);                                              \
	                                                                                  \
		MEM_COPY(data, buf, len2 * sizeof(ivm_char_t));                               \
		MEM_COPY(data + len2, ivm_string_trimHead(str1), len1 * sizeof(ivm_char_t));  \
		data[size] = '\0';                                                            \
	                                                                                  \
		ivm_string_initHead(ret, IVM_FALSE, size);                                    \
	                                                                                  \
		return ivm_string_object_new_c(_STATE, ret);                                  \
	}

#endif
