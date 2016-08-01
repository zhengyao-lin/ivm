#include "priv.h"

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
		val = ivm_parser_parseNum(
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
			ivm_exec_addInstr(env->cur_exec, NEW_NUM_F, val);
		} else {
			ivm_exec_addInstr(env->cur_exec, NEW_NUM_I, val);
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
		val = ivm_parser_parseNum(
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
		ivm_exec_addInstr(env->cur_exec, NEW_NUM_F, val);
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

	GEN_ASSERT_NOT_LEFT_VALUE(expr, "string expression", flag);

	if (!flag.is_top_level) {
		tmp_str = ivm_parser_parseStr(
			str_expr->val.val,
			str_expr->val.len
		);

		ivm_exec_addInstr(env->cur_exec, NEW_STR, tmp_str);

		MEM_FREE(tmp_str);
	}

	return NORET();
}

ilang_gen_value_t
ilang_gen_id_expr_eval(ilang_gen_expr_t *expr,
					   ilang_gen_flag_t flag,
					   ilang_gen_env_t *env)
{
	ilang_gen_id_expr_t *id_expr = IVM_AS(expr, ilang_gen_id_expr_t);
	ivm_char_t *tmp_str;
	ilang_gen_value_t ret = NORET();

	tmp_str = ivm_parser_parseStr(
		id_expr->val.val,
		id_expr->val.len
	);

#define ID_GEN(name, extra, set_instr, get_instr) \
	if (sizeof(name) == sizeof("")                                     \
		|| !IVM_STRCMP(tmp_str, (name))) {                             \
		extra                                                          \
		if (flag.is_left_val) {                                        \
			set_instr;                                                 \
		} else if (!flag.is_top_level) {                               \
			get_instr;                                                 \
		}                                                              \
	}

	ID_GEN("loc",
		{ ret.is_id_loc = IVM_TRUE; } if (flag.is_slot_expr) { } else,
		// avoid generate code if is the operand of slot expr
		ivm_exec_addInstr(env->cur_exec, SET_LOCAL_CONTEXT),
		ivm_exec_addInstr(env->cur_exec, GET_LOCAL_CONTEXT))
	else
	ID_GEN("top",
		{ ret.is_id_top = IVM_TRUE; } if (flag.is_slot_expr) { } else,
		// avoid generate code if is the operand of slot expr
		ivm_exec_addInstr(env->cur_exec, SET_GLOBAL_CONTEXT),
		ivm_exec_addInstr(env->cur_exec, GET_GLOBAL_CONTEXT))
	else
	ID_GEN("", { },
		ivm_exec_addInstr(env->cur_exec, SET_CONTEXT_SLOT, tmp_str),
		ivm_exec_addInstr(env->cur_exec, GET_CONTEXT_SLOT, tmp_str))

#undef ID_GEN

	MEM_FREE(tmp_str);

	return ret;
}

ilang_gen_value_t
ilang_gen_table_expr_eval(ilang_gen_expr_t *expr,
						  ilang_gen_flag_t flag,
						  ilang_gen_env_t *env)
{
	ilang_gen_table_expr_t *table_expr = IVM_AS(expr, ilang_gen_table_expr_t);
	ivm_size_t size;
	ilang_gen_table_entry_t tmp_entry;
	ilang_gen_table_entry_list_t *list;
	ilang_gen_table_entry_list_iterator_t eiter;
	ivm_char_t *tmp_str;

	GEN_ASSERT_NOT_LEFT_VALUE(expr, "table expression", flag);

	list = table_expr->list;
	size = ilang_gen_table_entry_list_size(list);

	/* not top level => no new object */
	if (size) {
		ivm_exec_addInstr(env->cur_exec, NEW_OBJ_T, size);
	} else {
		ivm_exec_addInstr(env->cur_exec, NEW_OBJ);
	}

	ILANG_GEN_TABLE_ENTRY_LIST_EACHPTR_R(list, eiter) {
		tmp_entry = ILANG_GEN_TABLE_ENTRY_LIST_ITER_GET(eiter);

		if (!flag.is_top_level ||
			tmp_entry.expr->check(tmp_entry.expr, CHECK_SE())) {
			// not top level and no side effect => skip
			tmp_entry.expr->eval(
				tmp_entry.expr,
				FLAG(0),
				env
			);

			if (!flag.is_top_level) {
				tmp_str = ivm_parser_parseStr(
					tmp_entry.name.val,
					tmp_entry.name.len
				);

				if (!IVM_STRCMP(tmp_str, "proto")) {
					ivm_exec_addInstr(env->cur_exec, DUP_N, 1);
					ivm_exec_addInstr(env->cur_exec, SET_PROTO);
				} else {
					ivm_exec_addInstr(env->cur_exec, SET_SLOT_B, tmp_str);
				}

				MEM_FREE(tmp_str);
			}
		}
	}

	if (flag.is_top_level) {
		ivm_exec_addInstr(env->cur_exec, POP);
	}

	return NORET();
}

ilang_gen_value_t
ilang_gen_list_expr_eval(ilang_gen_expr_t *expr,
						 ilang_gen_flag_t flag,
						 ilang_gen_env_t *env)
{
	ilang_gen_list_expr_t *list_expr = IVM_AS(expr, ilang_gen_list_expr_t);
	ilang_gen_expr_list_t *elems = list_expr->elems;
	ilang_gen_expr_list_iterator_t eiter;
	ilang_gen_expr_t *tmp_elem;

	ILANG_GEN_EXPR_LIST_EACHPTR_R(elems, eiter) {
		tmp_elem = ILANG_GEN_EXPR_LIST_ITER_GET(eiter);
		tmp_elem->eval(tmp_elem, FLAG(0), env);
	}

	ivm_exec_addInstr(env->cur_exec, NEW_LIST, ilang_gen_expr_list_size(elems));
	
	if (flag.is_top_level) {
		ivm_exec_addInstr(env->cur_exec, POP);
	}

	return NORET();
}
