#include "pub/type.h"
#include "pub/com.h"
#include "pub/vm.h"
#include "pub/err.h"

#include "gen.h"

ilang_gen_value_t
ilang_gen_expr_block_eval(ilang_gen_expr_t *expr,
						  ilang_gen_flag_t flag,
						  ilang_gen_env_t *env)
{}

ilang_gen_value_t
ilang_gen_int_expr_eval(ilang_gen_expr_t *expr,
						ilang_gen_flag_t flag,
						ilang_gen_env_t *env)
{}

ilang_gen_value_t
ilang_gen_float_expr_eval(ilang_gen_expr_t *expr,
						  ilang_gen_flag_t flag,
						  ilang_gen_env_t *env)
{}

ilang_gen_value_t
ilang_gen_string_expr_eval(ilang_gen_expr_t *expr,
						   ilang_gen_flag_t flag,
						   ilang_gen_env_t *env)
{}

ilang_gen_value_t
ilang_gen_id_expr_eval(ilang_gen_expr_t *expr,
					   ilang_gen_flag_t flag,
					   ilang_gen_env_t *env)
{}

ilang_gen_value_t
ilang_gen_call_expr_eval(ilang_gen_expr_t *expr,
						 ilang_gen_flag_t flag,
						 ilang_gen_env_t *env)
{}

ilang_gen_value_t
ilang_gen_slot_expr_eval(ilang_gen_expr_t *expr,
						 ilang_gen_flag_t flag,
						 ilang_gen_env_t *env)
{}

ilang_gen_value_t
ilang_gen_unary_expr_eval(ilang_gen_expr_t *expr,
						  ilang_gen_flag_t flag,
						  ilang_gen_env_t *env)
{}

ilang_gen_value_t
ilang_gen_binary_expr_eval(ilang_gen_expr_t *expr,
						   ilang_gen_flag_t flag,
						   ilang_gen_env_t *env)
{}

ilang_gen_value_t
ilang_gen_cmp_expr_eval(ilang_gen_expr_t *expr,
						ilang_gen_flag_t flag,
						ilang_gen_env_t *env)
{}

ilang_gen_value_t
ilang_gen_fn_expr_eval(ilang_gen_expr_t *expr,
					   ilang_gen_flag_t flag,
					   ilang_gen_env_t *env)
{}

ilang_gen_value_t
ilang_gen_if_expr_eval(ilang_gen_expr_t *expr,
					   ilang_gen_flag_t flag,
					   ilang_gen_env_t *env)
{}

ilang_gen_value_t
ilang_gen_intr_expr_eval(ilang_gen_expr_t *expr,
						 ilang_gen_flag_t flag,
						 ilang_gen_env_t *env)
{}

ilang_gen_value_t
ilang_gen_assign_expr_eval(ilang_gen_expr_t *expr,
						   ilang_gen_flag_t flag,
						   ilang_gen_env_t *env)
{}

void
ilang_gen_expr_block_destruct(ilang_gen_expr_t *expr)
{
	ilang_gen_expr_list_free(IVM_AS(expr, ilang_gen_expr_block_t)->list);
	return;
}

void
ilang_gen_call_expr_destruct(ilang_gen_expr_t *expr)
{
	ilang_gen_call_expr_t *tmp = IVM_AS(expr, ilang_gen_call_expr_t);

	ilang_gen_expr_free(tmp->callee);
	ilang_gen_expr_list_free(tmp->args);
	
	return;
}

void
ilang_gen_slot_expr_destruct(ilang_gen_expr_t *expr)
{
	ilang_gen_expr_free(IVM_AS(expr, ilang_gen_slot_expr_t)->obj);
	return;
}

void
ilang_gen_unary_expr_destruct(ilang_gen_expr_t *expr)
{
	ilang_gen_expr_free(IVM_AS(expr, ilang_gen_unary_expr_t)->opr);
	return;
}

void
ilang_gen_binary_expr_destruct(ilang_gen_expr_t *expr)
{
	ilang_gen_binary_expr_t *tmp = IVM_AS(expr, ilang_gen_binary_expr_t);
	ilang_gen_expr_free(tmp->op1);
	ilang_gen_expr_free(tmp->op2);
	return;
}

void
ilang_gen_cmp_expr_destruct(ilang_gen_expr_t *expr)
{
	ilang_gen_cmp_expr_t *tmp = IVM_AS(expr, ilang_gen_cmp_expr_t);
	ilang_gen_expr_free(tmp->op1);
	ilang_gen_expr_free(tmp->op2);
	return;
}

void
ilang_gen_fn_expr_destruct(ilang_gen_expr_t *expr)
{
	ilang_gen_fn_expr_t *tmp = IVM_AS(expr, ilang_gen_fn_expr_t);

	ilang_gen_param_list_free(tmp->params);
	ilang_gen_expr_free(tmp->body);

	return;
}

void
ilang_gen_if_expr_destruct(ilang_gen_expr_t *expr)
{
	ilang_gen_if_expr_t *tmp = IVM_AS(expr, ilang_gen_if_expr_t);

	ilang_gen_branch_dump(&tmp->main);
	ilang_gen_branch_list_free(tmp->elifs);
	ilang_gen_branch_dump(&tmp->last);

	return;
}

void
ilang_gen_intr_expr_destruct(ilang_gen_expr_t *expr)
{
	ilang_gen_expr_free(IVM_AS(expr, ilang_gen_intr_expr_t)->val);
	return;
}

void
ilang_gen_assign_expr_destruct(ilang_gen_expr_t *expr)
{
	ilang_gen_assign_expr_t *tmp = IVM_AS(expr, ilang_gen_assign_expr_t);
	ilang_gen_expr_free(tmp->lhe);
	ilang_gen_expr_free(tmp->rhe);
	return;
}
