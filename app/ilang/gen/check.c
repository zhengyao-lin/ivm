#include "priv.h"

ivm_bool_t
ilang_gen_int_expr_check(ilang_gen_expr_t *expr,
						 ilang_gen_check_flag_t flag)
{
	return IVM_FALSE;
}

ivm_bool_t
ilang_gen_float_expr_check(ilang_gen_expr_t *expr,
						   ilang_gen_check_flag_t flag)
{
	return IVM_FALSE;
}

ivm_bool_t
ilang_gen_string_expr_check(ilang_gen_expr_t *expr,
							ilang_gen_check_flag_t flag)
{
	return IVM_FALSE;
}

ivm_bool_t
ilang_gen_id_expr_check(ilang_gen_expr_t *expr,
						ilang_gen_check_flag_t flag)
{
	return IVM_FALSE;
}

ivm_bool_t
ilang_gen_table_expr_check(ilang_gen_expr_t *expr,
						   ilang_gen_check_flag_t flag)
{
	ilang_gen_table_expr_t *table_expr = IVM_AS(expr, ilang_gen_table_expr_t);
	ilang_gen_table_entry_t tmp_entry;
	ilang_gen_table_entry_list_t *list = table_expr->list;
	ilang_gen_table_entry_list_iterator_t eiter;

	ILANG_GEN_TABLE_ENTRY_LIST_EACHPTR_R(list, eiter) {
		tmp_entry = ILANG_GEN_TABLE_ENTRY_LIST_ITER_GET(eiter);
		if (tmp_entry.expr->check(tmp_entry.expr, flag)) {
			return IVM_TRUE;
		}
	}

	return IVM_FALSE;
}

ivm_bool_t
ilang_gen_list_expr_check(ilang_gen_expr_t *expr,
						  ilang_gen_check_flag_t flag)
{
	ilang_gen_list_expr_t *list_expr = IVM_AS(expr, ilang_gen_list_expr_t);
	ilang_gen_expr_list_t *elems = list_expr->elems;
	ilang_gen_expr_list_iterator_t eiter;
	ilang_gen_expr_t *tmp_elem;

	ILANG_GEN_EXPR_LIST_EACHPTR(elems, eiter) {
		tmp_elem = ILANG_GEN_EXPR_LIST_ITER_GET(eiter);
		if (tmp_elem->check(tmp_elem, flag)) {
			return IVM_TRUE;
		}
	}

	return IVM_FALSE;
}

ivm_bool_t
ilang_gen_call_expr_check(ilang_gen_expr_t *expr,
						  ilang_gen_check_flag_t flag)
{
	/*
	ilang_gen_call_expr_t *call_expr = IVM_AS(expr, ilang_gen_call_expr_t);
	ilang_gen_expr_list_t *args = call_expr->args;
	ilang_gen_expr_list_iterator_t aiter;
	ilang_gen_expr_t *tmp_arg;

	ILANG_GEN_EXPR_LIST_EACHPTR(args, aiter) {
		tmp_arg = ILANG_GEN_EXPR_LIST_ITER_GET(aiter);
		if (tmp_arg->check(tmp_arg, flag)) {
			return IVM_TRUE;
		}
	}
	*/

	return IVM_TRUE;
}

ivm_bool_t
ilang_gen_slot_expr_check(ilang_gen_expr_t *expr,
						  ilang_gen_check_flag_t flag)
{
	ilang_gen_slot_expr_t *slot_expr = IVM_AS(expr, ilang_gen_slot_expr_t);

	return slot_expr->obj->check(slot_expr->obj, flag);
}

ivm_bool_t
ilang_gen_unary_expr_check(ilang_gen_expr_t *expr,
						   ilang_gen_check_flag_t flag)
{
	ilang_gen_unary_expr_t *unary_expr = IVM_AS(expr, ilang_gen_unary_expr_t);

	return unary_expr->opr->check(unary_expr->opr, flag);
}

ivm_bool_t
ilang_gen_binary_expr_check(ilang_gen_expr_t *expr,
							ilang_gen_check_flag_t flag)
{
	ilang_gen_binary_expr_t *binary_expr = IVM_AS(expr, ilang_gen_binary_expr_t);

	return
	binary_expr->op1->check(binary_expr->op1, flag) ||
	binary_expr->op2->check(binary_expr->op2, flag);
}

ivm_bool_t
ilang_gen_cmp_expr_check(ilang_gen_expr_t *expr,
						 ilang_gen_check_flag_t flag)
{
	ilang_gen_cmp_expr_t *cmp_expr = IVM_AS(expr, ilang_gen_cmp_expr_t);

	return
	cmp_expr->op1->check(cmp_expr->op1, flag) ||
	cmp_expr->op2->check(cmp_expr->op2, flag);
}

ivm_bool_t
ilang_gen_logic_expr_check(ilang_gen_expr_t *expr,
						   ilang_gen_check_flag_t flag)
{
	ilang_gen_logic_expr_t *logic_expr = IVM_AS(expr, ilang_gen_logic_expr_t);

	return
	logic_expr->lhe->check(logic_expr->lhe, flag) ||
	logic_expr->rhe->check(logic_expr->rhe, flag);
}

ivm_bool_t
ilang_gen_fn_expr_check(ilang_gen_expr_t *expr, ilang_gen_check_flag_t flag)
{
	return IVM_FALSE;
}

ivm_bool_t
ilang_gen_if_expr_check(ilang_gen_expr_t *expr, ilang_gen_check_flag_t flag)
{
	ilang_gen_if_expr_t *if_expr = IVM_AS(expr, ilang_gen_if_expr_t);
	ilang_gen_branch_t main_br = if_expr->main,
					   last_br = if_expr->last,
					   tmp_br;
	ilang_gen_branch_list_t *elifs = if_expr->elifs;
	ilang_gen_branch_list_iterator_t eiter;

	if (main_br.cond->check(main_br.cond, flag)) {
		return IVM_TRUE;
	}

	if (main_br.body->check(main_br.body, flag)) {
		return IVM_TRUE;
	}

	ILANG_GEN_BRANCH_LIST_EACHPTR(elifs, eiter) {
		tmp_br = ILANG_GEN_BRANCH_LIST_ITER_GET(eiter);

		if (tmp_br.cond->check(tmp_br.cond, flag)) {
			return IVM_TRUE;
		}

		if (tmp_br.body->check(tmp_br.body, flag)) {
			return IVM_TRUE;
		}
	}

	if (last_br.body && last_br.body->check(last_br.body, flag)) {
		return IVM_TRUE;
	}

	return IVM_FALSE;
}

ivm_bool_t
ilang_gen_while_expr_check(ilang_gen_expr_t *expr, ilang_gen_check_flag_t flag)
{
	ilang_gen_while_expr_t *while_expr = IVM_AS(expr, ilang_gen_while_expr_t);

	if (while_expr->cond->check(while_expr->cond, flag)) {
		return IVM_TRUE;
	}

	if (while_expr->body->check(while_expr->body, flag)) {
		return IVM_TRUE;
	}

	return IVM_FALSE;
}

ivm_bool_t
ilang_gen_intr_expr_check(ilang_gen_expr_t *expr, ilang_gen_check_flag_t flag)
{
	return IVM_TRUE;
}

ivm_bool_t
ilang_gen_assign_expr_check(ilang_gen_expr_t *expr, ilang_gen_check_flag_t flag)
{
	return IVM_TRUE;
}
