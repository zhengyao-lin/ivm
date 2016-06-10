#include <math.h>

#include "pub/type.h"
#include "pub/mem.h"
#include "pub/vm.h"

#include "oprt.h"
#include "obj.h"


ivm_oprt_binary_table_t *
ivm_oprt_binary_table_new()
{
	ivm_oprt_binary_table_t *ret = MEM_ALLOC(sizeof(ivm_oprt_binary_table_t),
											 ivm_oprt_binary_table_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("binary operator table"));

	ret->alloc = IVM_DEFAULT_OPRT_BINARY_TABLE_BUFFER_SIZE;
	ret->size = 0;
	ret->lst = MEM_ALLOC(sizeof(ivm_oprt_binary_t) * IVM_DEFAULT_OPRT_BINARY_TABLE_BUFFER_SIZE,
						 ivm_oprt_binary_t *);

	return ret;
}

void
ivm_oprt_binary_table_free(ivm_oprt_binary_table_t *table)
{
	if (table) {
		MEM_FREE(table->lst);
		MEM_FREE(table);
	}

	return;
}

IVM_PRIVATE
ivm_object_t *
num_add_num(ivm_vmstate_t *state,
			ivm_object_t *op1,
			ivm_object_t *op2)
{
	return ivm_numeric_new(state, ivm_numeric_getValue(op1) + ivm_numeric_getValue(op2));
}

IVM_PRIVATE
ivm_object_t *
num_sub_num(ivm_vmstate_t *state,
			ivm_object_t *op1,
			ivm_object_t *op2)
{
	return ivm_numeric_new(state, ivm_numeric_getValue(op1) - ivm_numeric_getValue(op2));
}

IVM_PRIVATE
ivm_object_t *
num_mul_num(ivm_vmstate_t *state,
			ivm_object_t *op1,
			ivm_object_t *op2)
{
	return ivm_numeric_new(state, ivm_numeric_getValue(op1) * ivm_numeric_getValue(op2));
}

IVM_PRIVATE
ivm_object_t *
num_div_num(ivm_vmstate_t *state,
			ivm_object_t *op1,
			ivm_object_t *op2)
{
	return ivm_numeric_new(state, ivm_numeric_getValue(op1) / ivm_numeric_getValue(op2));
}

IVM_PRIVATE
ivm_object_t *
num_mod_num(ivm_vmstate_t *state,
			ivm_object_t *op1,
			ivm_object_t *op2)
{
	return ivm_numeric_new(state, fmod(ivm_numeric_getValue(op1), ivm_numeric_getValue(op2)));
}

IVM_PRIVATE
ivm_object_t *
num_cmp_num(ivm_vmstate_t *state,
			ivm_object_t *op1,
			ivm_object_t *op2)
{
	return ivm_numeric_new(state, ivm_numeric_getValue(op1) - ivm_numeric_getValue(op2));
}

void
init_numeric(ivm_type_t *type,
			 ivm_vmstate_t *state)
{
	ivm_oprt_binary_table_t *add_table = ivm_oprt_binary_table_new();
	ivm_oprt_binary_table_t *sub_table = ivm_oprt_binary_table_new();
	ivm_oprt_binary_table_t *mul_table = ivm_oprt_binary_table_new();
	ivm_oprt_binary_table_t *div_table = ivm_oprt_binary_table_new();
	ivm_oprt_binary_table_t *mod_table = ivm_oprt_binary_table_new();
	ivm_oprt_binary_table_t *cmp_table = ivm_oprt_binary_table_new();

	ivm_oprt_binary_table_set(add_table, IVM_NUMERIC_T, num_add_num); /* num + num */
	ivm_oprt_binary_table_set(sub_table, IVM_NUMERIC_T, num_sub_num); /* num - num */
	ivm_oprt_binary_table_set(mul_table, IVM_NUMERIC_T, num_mul_num); /* num * num */
	ivm_oprt_binary_table_set(div_table, IVM_NUMERIC_T, num_div_num); /* num / num */
	ivm_oprt_binary_table_set(mod_table, IVM_NUMERIC_T, num_mod_num); /* num % num */
	ivm_oprt_binary_table_set(cmp_table, IVM_NUMERIC_T, num_cmp_num); /* num <=> num */

	ivm_type_setOpTable(type, add, add_table);
	ivm_type_setOpTable(type, sub, sub_table);
	ivm_type_setOpTable(type, mul, mul_table);
	ivm_type_setOpTable(type, div, div_table);
	ivm_type_setOpTable(type, mod, mod_table);
	ivm_type_setOpTable(type, cmp, cmp_table);

	return;
}

IVM_PRIVATE
ivm_type_init_proc_t
init_proc[] = {
	IVM_NULL,
	IVM_NULL,
	IVM_NULL,
	init_numeric,
	IVM_NULL,
	IVM_NULL
};

void
ivm_oprt_initType(ivm_type_t *type,
				  ivm_vmstate_t *state)
{
	ivm_type_init_proc_t tmp = init_proc[type->tag];

	if (tmp) {
		tmp(type, state);
	}

	return;
}
