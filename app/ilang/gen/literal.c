#include "priv.h"

ilang_gen_value_t
ilang_gen_none_expr_eval(ilang_gen_expr_t *expr,
						 ilang_gen_flag_t flag,
						 ilang_gen_env_t *env)
{
	// GEN_ASSERT_NOT_LEFT_VALUE(expr, "none expression", flag);

	if (flag.is_left_val) {
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), POP);
	} else if (!flag.is_top_level) {
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), NEW_NONE);
	}

	return NORET();
}

ilang_gen_value_t
ilang_gen_int_expr_eval(ilang_gen_expr_t *expr,
						ilang_gen_flag_t flag,
						ilang_gen_env_t *env)
{
	ilang_gen_int_expr_t *int_expr = IVM_AS(expr, ilang_gen_int_expr_t);
	ivm_double_t val;
	ivm_bool_t overfl = IVM_FALSE;
	ivm_bool_t err = IVM_FALSE;

	GEN_ASSERT_NOT_LEFT_VALUE(expr, "integer expression", flag);

	if (!flag.is_top_level) {
		val = ivm_conv_parseDouble(
			int_expr->val.val,
			int_expr->val.len,
			&overfl,
			&err
		);

		if (err) {
			GEN_ERR(
				expr->pos,
				GEN_ERR_MSG_FAILED_PARSE_NUM(
					int_expr->val.val,
					int_expr->val.len
				)
			);
		}

		if (overfl) {
			ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), NEW_NUM_F, val);
		} else {
			ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), NEW_NUM_I, val);
		}
	}

	return NORET();
}

ilang_gen_value_t
ilang_gen_float_expr_eval(ilang_gen_expr_t *expr,
						  ilang_gen_flag_t flag,
						  ilang_gen_env_t *env)
{
	ilang_gen_float_expr_t *float_expr = IVM_AS(expr, ilang_gen_float_expr_t);
	ivm_double_t val;
	ivm_bool_t err = IVM_FALSE;

	GEN_ASSERT_NOT_LEFT_VALUE(expr, "float expression", flag);

	if (!flag.is_top_level) {
		val = ivm_conv_parseDouble(
			float_expr->val.val,
			float_expr->val.len,
			IVM_NULL,
			&err
		);

		if (err) {
			GEN_ERR(
				expr->pos,
				GEN_ERR_MSG_FAILED_PARSE_NUM(
					float_expr->val.val,
					float_expr->val.len
				)
			);
		}
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), NEW_NUM_F, val);
	}

	return NORET();
}

ilang_gen_value_t
ilang_gen_string_expr_eval(ilang_gen_expr_t *expr,
						   ilang_gen_flag_t flag,
						   ilang_gen_env_t *env)
{
	ilang_gen_string_expr_t *str_expr = IVM_AS(expr, ilang_gen_string_expr_t);
	ivm_char_t *tmp_str;
	const ivm_char_t *err;

	GEN_ASSERT_NOT_LEFT_VALUE(expr, "string expression", flag);

	if (!flag.is_top_level) {
		tmp_str = ivm_parser_parseStr_heap(
			env->heap,
			str_expr->val.val,
			str_expr->val.len,
			&err
		);

		if (!tmp_str) {
			GEN_ERR_FAILED_PARSE_STRING(expr, err);
		}

		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), NEW_STR, tmp_str);
	}

	return NORET();
}
