#include "type.h"
#include "expr.h"
#include "obj.h"
#include "vm.h"
#include "err.h"

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
	ivm_binary_op_proc_list_t *add_table = ivm_binary_op_proc_list_new();
	ivm_binary_op_proc_list_t *sub_table = ivm_binary_op_proc_list_new();
	ivm_binary_op_proc_list_t *cmp_table = ivm_binary_op_proc_list_new();

	ivm_binary_op_proc_list_set(add_table, IVM_NUMERIC_T, num_add_num); /* num + num */
	ivm_binary_op_proc_list_set(sub_table, IVM_NUMERIC_T, num_sub_num); /* num - num */
	ivm_binary_op_proc_list_set(cmp_table, IVM_NUMERIC_T, num_cmp_num); /* num <=> num */

	ivm_type_setOpTable(type, add, add_table);
	ivm_type_setOpTable(type, sub, sub_table);
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
ivm_binary_op_initType(ivm_type_t *type, ivm_vmstate_t *state)
{
	ivm_type_init_proc_t tmp = init_proc[type->tag];

	if (tmp) {
		tmp(type, state);
	}

	return;
}
