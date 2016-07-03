#include <math.h>

#include "pub/type.h"
#include "pub/com.h"
#include "pub/mem.h"
#include "pub/vm.h"
#include "pub/err.h"

#include "oprt.h"
#include "obj.h"

#define TYPE_TAG_OF(t) IVM_##t##_T
#define UNIOP_PROC_NAME(op, t) ivm_uniop_##op##_##t
#define BINOP_PROC_NAME(t1, op, t2) ivm_binop_##t1##_##op##_##t2

#define UNIOP_GEN(op, t, ...) \
	IVM_PRIVATE ivm_object_t * UNIOP_PROC_NAME(op, t)(ivm_vmstate_t *__state__, \
													  ivm_object_t *__op1__) \
	__VA_ARGS__

#define BINOP_GEN(t1, op, t2, ...) \
	IVM_PRIVATE ivm_object_t * BINOP_PROC_NAME(t1, op, t2)(ivm_vmstate_t *__state__, \
														   ivm_object_t *__op1__, \
														   ivm_object_t *__op2__) \
	__VA_ARGS__

#define _STATE (__state__)
#define _OP1 (__op1__)
#define _OP2 (__op2__)

	#include "oprt.req.h"
	#include "oprt.def.h"

#undef _STATE
#undef _OP1
#undef _OP2
#undef UNIOP_GEN
#undef BINOP_GEN

ivm_binop_table_t *
ivm_binop_table_new()
{
	ivm_binop_table_t *ret = MEM_ALLOC(sizeof(ivm_binop_table_t),
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
		MEM_FREE(table->lst);
		MEM_FREE(table);
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
		MEM_FREE(table->lst);
	}

	return;
}

void
ivm_oprt_initType(ivm_vmstate_t *state)
{
	ivm_type_list_t *type_list = IVM_VMSTATE_GET(state, TYPE_LIST);
	ivm_binop_table_t *tmp_table;
	ivm_type_t *tmp_type;

#define BINOP_GEN(t1, op, t2, ...) \
	(tmp_table = ivm_type_getBinopTable((tmp_type = ivm_type_list_at(type_list, TYPE_TAG_OF(t1))), op), \
	 /* tmp_table ? 0 : (tmp_table = ivm_type_setBinopTable(tmp_type, op, ivm_binop_table_new())), */ \
	 ivm_binop_table_set(tmp_table, TYPE_TAG_OF(t2), BINOP_PROC_NAME(t1, op, t2)));

#define UNIOP_GEN(op, t, ...) \
	(ivm_uniop_table_set(ivm_type_getUniopTable(ivm_type_list_at(type_list, TYPE_TAG_OF(t))), \
						 IVM_UNIOP_ID(op), UNIOP_PROC_NAME(op, t)));

	#include "oprt.def.h"

#undef BINOP_GEN

	return;
}
