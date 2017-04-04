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

ilang_gen_expr_t *
ilang_parser_parseExpr(ilang_gen_trans_unit_t *unit,
					   const ivm_char_t *src,
					   ivm_size_t lineno);

ilang_gen_value_t
ilang_gen_string_expr_eval(ilang_gen_expr_t *expr,
						   ilang_gen_flag_t flag,
						   ilang_gen_env_t *env)
{
	ilang_gen_string_expr_t *str_expr = IVM_AS(expr, ilang_gen_string_expr_t);
	ivm_char_t *tmp_str;
	const ivm_char_t *err;
	const ivm_char_t *i, *end, *beg, *lit;
	ivm_char_t last = 0;
	ivm_size_t brace_count, tmp_len, str_count = 0, j;
	ilang_gen_expr_t *tmp_expr;

	GEN_ASSERT_NOT_LEFT_VALUE(expr, "string expression", flag);

#if 1
	for (lit = i = str_expr->val.val, end = i + str_expr->val.len;
		 i != end; i++) {

#define MARK_NOESC(ch, dob) \
	case (ch): \
		if (last == '\\') last = 0; \
		else { \
			last = (ch); \
			dob; \
		} \
		break;

		switch (*i) {
			MARK_NOESC('#', 0)
			MARK_NOESC('\\', 0)

			case '{':
				if (last == '#') {
					// found interpolator
					beg = ++i;
					last = 0;
					brace_count = 1;

					for (; i != end; i++) {
						switch (*i) {
							MARK_NOESC('\\', 0)
							MARK_NOESC('{', {
								brace_count++;
							})

							MARK_NOESC('}', {
								brace_count--;
								if (!brace_count) {
									goto L_END;
								}
							})

							default: last = 0;
						}
					}

					L_END:;

					if (brace_count) {
						GEN_ERR_UNCLOSED_STRING_INT(expr);
					}

					tmp_len = IVM_PTR_DIFF(beg, lit, ivm_char_t) - 2 /* "#{" */;
					tmp_str = ivm_parser_parseStr_heap(env->heap, lit, tmp_len, &err);

					if (!tmp_str) {
						GEN_ERR_FAILED_PARSE_STRING(expr, err);
					}

					ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), NEW_STR, tmp_str);

					tmp_len = IVM_PTR_DIFF(i, beg, ivm_char_t) /* last "}" */;

					if (tmp_len) {
						str_count++;

						tmp_str = ivm_parser_parseStr_heap(env->heap, beg, tmp_len, &err);

						if (!tmp_str) {
							GEN_ERR_FAILED_PARSE_STRING(expr, err);
						}
					}

					// IVM_TRACE("src: %s\n", tmp_str);

					tmp_expr = ilang_parser_parseExpr(env->tunit, tmp_str, GET_LINE(expr));

					if (!tmp_expr) {
						GEN_ERR_FAILED_PARSE_STRING_INT(expr);
					}

					tmp_expr->eval(tmp_expr, FLAG(0), env);
					str_count++;

					lit = i + 1; // next literal
				}

				last = 0;
				
				break;

			default: last = 0;
		}
	}

	if (lit != end || !str_count /* there has to be at least 1 string */) {
		str_count++;
		tmp_len = IVM_PTR_DIFF(end, lit, ivm_char_t);
		tmp_str = ivm_parser_parseStr_heap(env->heap, lit, tmp_len, &err);

		if (!tmp_str) {
			GEN_ERR_FAILED_PARSE_STRING(expr, err);
		}

		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), NEW_STR, tmp_str);
	}

	for (j = 0; j < str_count - 1; j++) {
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), ADD);
	}

#endif

#if 0
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
#endif

	if (flag.is_top_level) {
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), POP);
	}

	return NORET();
}
