#include <math.h>

#include "pub/type.h"
#include "pub/com.h"
#include "pub/vm.h"
#include "pub/err.h"

#include "std/mem.h"

#include "native/priv.h"

#include "oprt.h"
#include "obj.h"

#define TYPE_TAG_OF(t) IVM_##t##_T
#define UNIOP_PROC_NAME(op, t) ivm_uniop_##op##_##t
#define BINOP_PROC_NAME(t, op) ivm_binop_##t##_##op

#define DEF_OOP_UNIOP_NAME(op, t) ivm_def_oop_uniop_##op##_##t
#define DEF_OOP_BINOP_NAME(t, op) ivm_def_oop_binop_##t##_##op
#define DEF_OOP_TRIOP_NAME(t, op) ivm_def_oop_triop_##t##_##op

#define DEF_OOP_UNIOP_PROC(type, op) \
	IVM_NATIVE_FUNC_C(DEF_OOP_UNIOP_NAME(op, type)) {                                                         \
		CHECK_BASE(type);                                                                                     \
		return UNIOP_PROC_NAME(op, type)(NAT_STATE(), NAT_CORO(), NAT_BASE());                                \
	}

#define DEF_OOP_BINOP_PROC(t, op, is_cmp, p) \
	IVM_NATIVE_FUNC_C(DEF_OOP_BINOP_NAME(t, op)) {                                                            \
		ivm_binop_proc_t proc = (p) ? (p) : BINOP_PROC_NAME(t, op);                                           \
		CHECK_BASE(t);                                                                                        \
		CHECK_ARG_COUNT(1);                                                                                   \
		return is_cmp                                                                                         \
			   ? ivm_numeric_new(NAT_STATE(),                                                                 \
			   					 (ivm_ptr_t)proc(NAT_STATE(), NAT_CORO(), NAT_BASE(), NAT_ARG_AT(1)))         \
			   : proc(NAT_STATE(), NAT_CORO(), NAT_BASE(), NAT_ARG_AT(1));                                    \
	}

#define DEF_OOP_TRIOP_PROC(t, op, p) \
	IVM_NATIVE_FUNC_C(DEF_OOP_TRIOP_NAME(t, op)) {                                                            \
		CHECK_BASE(t);                                                                                        \
		CHECK_ARG_COUNT(2);                                                                                   \
		return ((p) ? (p) : BINOP_PROC_NAME(t, op))                                                           \
			   (NAT_STATE(), NAT_CORO(), NAT_BASE(), NAT_ARG_AT(1), NAT_ARG_AT(2));                           \
	}

#define UNIOP_PROC_DEF(name) IVM_UNIOP_PROC_DEF(name)
#define BINOP_PROC_DEF(name) IVM_BINOP_PROC_DEF(name)
#define TRIOP_PROC_DEF(name) IVM_TRIOP_PROC_DEF(name)

#define UNIOP_GEN(op, t, ...) \
	UNIOP_PROC_DEF(UNIOP_PROC_NAME(op, t))   \
	__VA_ARGS__                              \
	DEF_OOP_UNIOP_PROC(t, op)

#define BINOP_GEN(t, op, is_cmp, ...) \
	BINOP_PROC_DEF(BINOP_PROC_NAME(t, op))   \
	__VA_ARGS__                              \
	DEF_OOP_BINOP_PROC(t, op, is_cmp, IVM_NULL)

#define TRIOP_GEN(t, op, ...) \
	TRIOP_PROC_DEF(BINOP_PROC_NAME(t, op))   \
	__VA_ARGS__                              \
	DEF_OOP_TRIOP_PROC(t, op, IVM_NULL)

#define BINOP_GEN_C(t, op, is_cmp, func) \
	IVM_PRIVATE ivm_binop_proc_t BINOP_PROC_NAME(t, op) = (func); \
	DEF_OOP_BINOP_PROC(t, op, is_cmp, func)

#define TRIOP_GEN_C(t, op, func) \
	IVM_PRIVATE ivm_triop_proc_t BINOP_PROC_NAME(t, op) = (func); \
	DEF_OOP_TRIOP_PROC(t, op, func)

#define _STATE (__state__)
#define _CORO (__coro__)
#define _OP1 (__op1__)
#define _OP2 (__op2__)
#define _OP3 (__op3__)

#define _TYPE_SWITCH(...) \
	switch (IVM_OBJECT_GET(_OP2, TYPE_TAG)) { \
		__VA_ARGS__;                          \
	}

#define _IS_TYPE(tag, ...) \
	case (tag): __VA_ARGS__; break;

#define _NO_OTHER(op) \
	default:                                                                          \
		OPRT_FATAL(IVM_ERROR_MSG_NO_BINOP_FOR(IVM_OBJECT_GET(_OP1, TYPE_NAME), op,    \
											  IVM_OBJECT_GET(_OP2, TYPE_NAME)));

#define _ONLY_TYPE(tag, op, dob) \
	if (IVM_IS_BTTYPE(_OP2, _STATE, tag)) {                                           \
		dob;                                                                          \
	} else {                                                                          \
		OPRT_FATAL(IVM_ERROR_MSG_NO_BINOP_FOR(IVM_OBJECT_GET(_OP1, TYPE_NAME), op,    \
											  IVM_OBJECT_GET(_OP2, TYPE_NAME)));      \
	}

#define _ONLY_TYPE_C(tag, op, dob, els) \
	if (IVM_IS_BTTYPE(_OP2, _STATE, tag)) {                                           \
		dob;                                                                          \
	} else {                                                                          \
		els;                                                                          \
	}

#define _FALL_TO_BINOP(proc) \
	return (proc)(_STATE, _CORO, _OP1, _OP2);

#define _FALL_TO_TRIOP(proc) \
	return (proc)(_STATE, _CORO, _OP1, _OP2, _OP3);

#define _STD_EQ(tag, dob) _ONLY_TYPE_C(tag, "==", dob, { return (ivm_object_t *)(ivm_ptr_t)IVM_FALSE; })
#define _STD_NE(tag, dob) _ONLY_TYPE_C(tag, "!=", dob, { return (ivm_object_t *)(ivm_ptr_t)IVM_TRUE; })

	#include "oprt.req.h"

	UNIOP_PROC_DEF(ivm_uniop_not)
	{
		return ivm_numeric_new(_STATE, !ivm_object_toBool(_OP1, _STATE));
	}

	TRIOP_PROC_DEF(ivm_binop_setStringIndex)
	{
		_ONLY_TYPE(IVM_STRING_OBJECT_T, "[=]", {
			ivm_object_setSlot(_OP1, _STATE, ivm_string_object_getValue(_OP2), _OP3);
			return _OP3 ? _OP3 : IVM_NONE(_STATE);
		});
	}

	BINOP_PROC_DEF(ivm_binop_getStringIndex)
	{
		_ONLY_TYPE(IVM_STRING_OBJECT_T, "[]", {
			ivm_object_t *tmp_obj;
			tmp_obj = ivm_object_getSlot(_OP1, _STATE, ivm_string_object_getValue(_OP2));	
			return tmp_obj ? tmp_obj : IVM_NONE(_STATE);
		});
	}

	BINOP_PROC_DEF(ivm_binop_linkStringString)
	{
		const ivm_string_t *str1 = ivm_string_object_getValue(_OP1);
		const ivm_string_t *str2 = ivm_string_object_getValue(_OP2);
		ivm_size_t len1 = ivm_string_length(str1);
		ivm_size_t len2 = ivm_string_length(str2);
		ivm_size_t size = len1 + len2;
		ivm_string_t *ret = ivm_vmstate_alloc(_STATE, IVM_STRING_GET_SIZE(size));

		ivm_char_t *data = ivm_string_trimHead(ret);

		STD_MEMCPY(data, ivm_string_trimHead(str1), len1 * sizeof(ivm_char_t));
		STD_MEMCPY(data + len1, ivm_string_trimHead(str2), len2 * sizeof(ivm_char_t));
		data[size] = '\0';

		ivm_string_initHead(ret, IVM_FALSE, size);

		return ivm_string_object_new_c(_STATE, ret);
	}

	BINOP_PROC_DEF(ivm_binop_linkStringNum)
	{
		LINK_STRING_NUM(_OP1, _OP2, {
			STD_MEMCPY(data, ivm_string_trimHead(str1), len1 * sizeof(ivm_char_t));
			STD_MEMCPY(data + len1, buf, len2 * sizeof(ivm_char_t));
			data[size] = '\0';
		});
	}

	BINOP_PROC_DEF(ivm_binop_linkStringNum_r)
	{
		LINK_STRING_NUM(_OP2, _OP1, {
			STD_MEMCPY(data, buf, len2 * sizeof(ivm_char_t));
			STD_MEMCPY(data + len2, ivm_string_trimHead(str1), len1 * sizeof(ivm_char_t));
			data[size] = '\0';
		});
	}

	BINOP_PROC_DEF(ivm_binop_mulList)
	{
		_ONLY_TYPE(IVM_NUMERIC_T, "*", {
			ivm_number_t times = ivm_numeric_getValue(_OP2);
			ivm_list_object_t *ret;

			CHECK_OVERFLOW(times);

			if (times <= 0) {
				return ivm_list_object_new(_STATE);
			}

			ret = IVM_AS(ivm_object_clone(_OP1, _STATE), ivm_list_object_t);
			if (!ivm_list_object_multiply(ret, _STATE, times)) {
				return IVM_NULL;
			}

			return IVM_AS_OBJ(ret);
		});
	}

	BINOP_PROC_DEF(ivm_binop_mulString)
	{
		_ONLY_TYPE(IVM_NUMERIC_T, "*", {
			ivm_number_t times = ivm_numeric_getValue(_OP2);
			ivm_string_object_t *ret;

			CHECK_OVERFLOW(times);

			if (times <= 0) {
				return ivm_string_object_new(_STATE, IVM_VMSTATE_CONST(_STATE, C_EMPTY));
			}

			ret = IVM_AS(ivm_object_clone(_OP1, _STATE), ivm_string_object_t);
			if (!ivm_string_object_multiply(ret, _STATE, times)) {
				return IVM_NULL;
			}

			return IVM_AS_OBJ(ret);
		});
	}

	BINOP_PROC_DEF(ivm_binop_eq)
	{
		return (ivm_object_t *)(ivm_ptr_t)(_OP1 == _OP2);
	}

	BINOP_PROC_DEF(ivm_binop_ne)
	{
		return (ivm_object_t *)(ivm_ptr_t)(_OP1 != _OP2);
	}

	#include "oprt.def.h"

#undef _STATE
#undef _OP1
#undef _OP2
#undef _OP3

#undef UNIOP_GEN
#undef BINOP_GEN
#undef BINOP_GEN_C
#undef TRIOP_GEN
#undef TRIOP_GEN_C

#if 0

ivm_binop_table_t *
ivm_binop_table_new()
{
	ivm_binop_table_t *ret = STD_ALLOC(sizeof(ivm_binop_table_t));

	IVM_MEMCHECK(ret);

	ret->size = 0;
	ret->lst = IVM_NULL;

	return ret;
}

void
ivm_binop_table_free(ivm_binop_table_t *table)
{
	if (table) {
		STD_FREE(table->lst);
		STD_FREE(table);
	}

	return;
}

void
ivm_binop_table_init(ivm_binop_table_t *table)
{
	table->size = 0;
	table->lst = IVM_NULL;
	return;
}

void
ivm_binop_table_dump(ivm_binop_table_t *table)
{
	if (table) {
		STD_FREE(table->lst);
	}

	return;
}

#endif

void
ivm_oprt_initType(ivm_vmstate_t *state)
{
	ivm_type_t *type_list = IVM_VMSTATE_GET(state, TYPE_LIST);
	ivm_type_t *tmp_type;

#define TRIOP_GEN(t, op, is_cmp, ...) \
	(ivm_type_setDefaultOop(type_list + t, IVM_OOP_ID(op), ivm_function_newNative(state, DEF_OOP_TRIOP_NAME(t, op))), \
	 ivm_binop_table_set(ivm_type_getBinopTable(tmp_type = type_list + t), \
	 					 IVM_BINOP_ID(op), (ivm_binop_proc_t)BINOP_PROC_NAME(t, op)));

#define TRIOP_GEN_C(t, op, func) \
	(ivm_type_setDefaultOop(type_list + t, IVM_OOP_ID(op), ivm_function_newNative(state, DEF_OOP_TRIOP_NAME(t, op))), \
	 ivm_binop_table_set(ivm_type_getBinopTable(tmp_type = type_list + t), \
	 					 IVM_BINOP_ID(op), (ivm_binop_proc_t)(func)));

#define BINOP_GEN(t, op, is_cmp, ...) \
	(ivm_type_setDefaultOop(type_list + t, IVM_OOP_ID(op), ivm_function_newNative(state, DEF_OOP_BINOP_NAME(t, op))), \
	 ivm_binop_table_set(ivm_type_getBinopTable(tmp_type = type_list + t), \
	 					 IVM_BINOP_ID(op), BINOP_PROC_NAME(t, op)));

#define BINOP_GEN_C(t, op, is_cmp, func) \
	(ivm_type_setDefaultOop(type_list + t, IVM_OOP_ID(op), ivm_function_newNative(state, DEF_OOP_BINOP_NAME(t, op))), \
	 ivm_binop_table_set(ivm_type_getBinopTable(tmp_type = type_list + t), \
	 					 IVM_BINOP_ID(op), (func)));

#define UNIOP_GEN(op, t, ...) \
	(ivm_type_setDefaultOop(type_list + t, IVM_OOP_ID(op), ivm_function_newNative(state, DEF_OOP_UNIOP_NAME(op, t))), \
	 ivm_uniop_table_set(ivm_type_getUniopTable(type_list + t), \
						 IVM_UNIOP_ID(op), UNIOP_PROC_NAME(op, t)));

	#include "oprt.def.h"

#undef UNIOP_GEN
#undef BINOP_GEN
#undef BINOP_GEN_C
#undef TRIOP_GEN
#undef TRIOP_GEN_C

	return;
}
