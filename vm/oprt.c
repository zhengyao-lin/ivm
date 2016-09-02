#include <math.h>

#include "pub/type.h"
#include "pub/com.h"
#include "pub/vm.h"
#include "pub/err.h"

#include "std/mem.h"

#include "oprt.h"
#include "obj.h"

#define TYPE_TAG_OF(t) IVM_##t##_T
#define UNIOP_PROC_NAME(op, t) ivm_uniop_##op##_##t
#define BINOP_PROC_NAME(t1, op, t2) ivm_binop_##t1##_##op##_##t2

#define UNIOP_PROC_DEF(name) \
	IVM_PRIVATE ivm_object_t *name(ivm_vmstate_t *__state__,   \
								   ivm_coro_t *__coro__,       \
								   ivm_object_t *__op1__)

#define BINOP_PROC_DEF(name) \
	IVM_PRIVATE ivm_object_t *name(ivm_vmstate_t *__state__,   \
								   ivm_coro_t *__coro__,       \
								   ivm_object_t *__op1__,      \
								   ivm_object_t *__op2__,      \
								   ivm_object_t *__assign__)

#define UNIOP_GEN(op, t, ...) \
	UNIOP_PROC_DEF(UNIOP_PROC_NAME(op, t)) \
	__VA_ARGS__

#define BINOP_GEN(t1, op, t2, ...) \
	BINOP_PROC_DEF(BINOP_PROC_NAME(t1, op, t2)) \
	__VA_ARGS__

#define BINOP_GEN_C(t1, op, t2, func)

#define _STATE (__state__)
#define _CORO (__coro__)
#define _OP1 (__op1__)
#define _OP2 (__op2__)
#define _ASSIGN (__assign__)

	#include "oprt.req.h"
	#include "oprt.def.h"

	BINOP_PROC_DEF(ivm_binop_getStringIndex)
	{
		ivm_object_t *tmp_obj;

		if (_ASSIGN) {
			ivm_object_setSlot(_OP1, _STATE, ivm_string_object_getValue(_OP2), _ASSIGN);
			return _ASSIGN;
		}

		tmp_obj = ivm_object_getSlot(_OP1, _STATE, ivm_string_object_getValue(_OP2));

		return tmp_obj ? tmp_obj : IVM_NONE(_STATE);
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

	BINOP_PROC_DEF(ivm_binop_constFalse)
	{
		return (ivm_object_t *)(ivm_ptr_t)IVM_FALSE;
	}

	BINOP_PROC_DEF(ivm_binop_constTrue)
	{
		return (ivm_object_t *)(ivm_ptr_t)IVM_TRUE;
	}

#undef _STATE
#undef _OP1
#undef _OP2
#undef _ASSIGN

#undef UNIOP_GEN
#undef BINOP_GEN
#undef BINOP_GEN_C

ivm_binop_table_t *
ivm_binop_table_new()
{
	ivm_binop_table_t *ret = STD_ALLOC(sizeof(ivm_binop_table_t),
									   ivm_binop_table_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("binary operator table"));

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

void
ivm_oprt_initType(ivm_vmstate_t *state)
{
	ivm_type_t *type_list = IVM_VMSTATE_GET(state, TYPE_LIST);
	ivm_binop_table_t *tmp_table;
	ivm_type_t *tmp_type;

#define BINOP_GEN(t1, op, t2, ...) \
	(tmp_table = ivm_type_getBinopTable((tmp_type = type_list + t1), op), \
	 ivm_binop_table_set(tmp_table, t2, BINOP_PROC_NAME(t1, op, t2)));

#define BINOP_GEN_C(t1, op, t2, func) \
	(tmp_table = ivm_type_getBinopTable((tmp_type = type_list + t1), op), \
	 ivm_binop_table_set(tmp_table, t2, (func)));

#define UNIOP_GEN(op, t, ...) \
	(ivm_uniop_table_set(ivm_type_getUniopTable(type_list + t), \
						 IVM_UNIOP_ID(op), UNIOP_PROC_NAME(op, t)));

	#include "oprt.def.h"

#undef UNIOP_GEN
#undef BINOP_GEN
#undef BINOP_GEN_C

	return;
}
