#include "priv.h"

ilang_gen_value_t
ilang_gen_expr_block_eval(ilang_gen_expr_t *expr,
						  ilang_gen_flag_t flag,
						  ilang_gen_env_t *env)
{
	ilang_gen_expr_block_t *block = IVM_AS(expr, ilang_gen_expr_block_t);
	ilang_gen_expr_list_t *list = block->list;
	ilang_gen_expr_list_iterator_t eiter;
	ilang_gen_expr_t *tmp_expr;

	GEN_ASSERT_NOT_LEFT_VALUE(expr, "expression block", flag);

	if (flag.is_top_level &&
		!expr->check(expr, CHECK_SE())) {
		return NORET();
	}

	ILANG_GEN_EXPR_LIST_EACHPTR_R(list, eiter) {
		tmp_expr = ILANG_GEN_EXPR_LIST_ITER_GET(eiter);
		if (ILANG_GEN_EXPR_LIST_ITER_IS_FIRST(list, eiter)
			&& !flag.is_top_level) {
			// the last expression in a block should leave an object on the stack
			// if and only if the block is nested(it's value will be used)
			tmp_expr->eval(
				tmp_expr,
				FLAG(0), env
			);
		} else if (!flag.is_top_level ||
				   tmp_expr->check(tmp_expr, CHECK_SE())) {
			// is top level or has side effect => generate
			// other expression should leave no object on the stack
			tmp_expr->eval(
				tmp_expr,
				FLAG(.is_top_level = IVM_TRUE),
				env
			);
		}
	}

	return NORET();
}

ivm_exec_unit_t *
ilang_gen_generateExecUnit(ilang_gen_trans_unit_t *unit)
{
	ivm_string_pool_t *str_pool = ivm_string_pool_new(IVM_TRUE);
	ivm_exec_unit_t *ret = ivm_exec_unit_new(0, ivm_exec_list_new());
	ivm_exec_t *top_level = ivm_exec_new(str_pool);
	ilang_gen_env_t env = { str_pool, ret, top_level, -1, IVM_NULL, IVM_NULL, IVM_NULL };

	ivm_exec_unit_registerExec(ret, top_level);

	unit->top_level->eval(
		unit->top_level,
		FLAG(.is_top_level = IVM_TRUE), &env
	);

	return ret;
}

ivm_bool_t
ilang_gen_expr_block_check(ilang_gen_expr_t *expr,
						   ilang_gen_check_flag_t flag)
{
	ilang_gen_expr_block_t *block = IVM_AS(expr, ilang_gen_expr_block_t);
	ilang_gen_expr_list_t *list = block->list;
	ilang_gen_expr_list_iterator_t eiter;
	ilang_gen_expr_t *tmp_expr;

	ILANG_GEN_EXPR_LIST_EACHPTR_R(list, eiter) {
		tmp_expr = ILANG_GEN_EXPR_LIST_ITER_GET(eiter);
		if (tmp_expr->check(tmp_expr, flag)) {
			return IVM_TRUE;
		}
	}

	return IVM_FALSE;
}
