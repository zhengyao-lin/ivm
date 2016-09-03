#include "priv.h"

ilang_gen_value_t
ilang_gen_call_expr_eval(ilang_gen_expr_t *expr,
						 ilang_gen_flag_t flag,
						 ilang_gen_env_t *env)
{
	ilang_gen_call_expr_t *call_expr = IVM_AS(expr, ilang_gen_call_expr_t);
	ilang_gen_expr_list_t *args;
	ilang_gen_expr_list_iterator_t aiter;
	ilang_gen_expr_t *tmp_arg;
	ilang_gen_value_t tmp_ret;

	GEN_ASSERT_NOT_LEFT_VALUE(expr, "call expression", flag);

	args = call_expr->args;
	// generate in a reverse order(original order in ast)
	ILANG_GEN_EXPR_LIST_EACHPTR(args, aiter) {
		tmp_arg = ILANG_GEN_EXPR_LIST_ITER_GET(aiter);
		tmp_arg->eval(tmp_arg, FLAG(0), env);
	}

	tmp_ret = call_expr->callee->eval(
		call_expr->callee,
		FLAG(.is_callee = IVM_TRUE),
		env
	);

	if (tmp_ret.has_base) {
		ivm_exec_addInstr_l(
			env->cur_exec, GET_LINE(expr), INVOKE_BASE,
			ilang_gen_expr_list_size(args)
		);
	} else {
		ivm_exec_addInstr_l(
			env->cur_exec, GET_LINE(expr), INVOKE,
			ilang_gen_expr_list_size(args)
		);
	}

	if (flag.is_top_level) {
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), POP);
	}

	return NORET();
}

ilang_gen_value_t
ilang_gen_slot_expr_eval(ilang_gen_expr_t *expr,
						 ilang_gen_flag_t flag,
						 ilang_gen_env_t *env)
{
	ilang_gen_slot_expr_t *slot_expr = IVM_AS(expr, ilang_gen_slot_expr_t);
	ivm_char_t *tmp_str;
	ilang_gen_value_t ret = NORET();
	ivm_bool_t is_proto;
	ilang_gen_value_t tmp_ret;
	const ivm_char_t *err;

	tmp_str = ivm_parser_parseStr_heap(
		env->heap,
		slot_expr->slot.val,
		slot_expr->slot.len,
		&err
	);

	if (!tmp_str) {
		GEN_ERR_FAILED_PARSE_STRING(expr, err);
	}

	is_proto = !IVM_STRCMP(tmp_str, "proto");

	if (flag.is_left_val) {
		tmp_ret = slot_expr->obj->eval(
			slot_expr->obj,
			FLAG(.is_slot_expr = IVM_TRUE),
			env
		);
		
		if (tmp_ret.is_id_loc) {
			ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), SET_LOCAL_SLOT, tmp_str);
		} else if (tmp_ret.is_id_top) {
			ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), SET_GLOBAL_SLOT, tmp_str);
		} else if (is_proto) {
			ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), SET_PROTO);
			ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), POP);
		} else {
			ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), SET_SLOT, tmp_str);
			ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), POP);
		}
	} else {
		tmp_ret = slot_expr->obj->eval(
			slot_expr->obj,
			FLAG(.is_slot_expr = IVM_TRUE),
			// is_top_level == true => has side effect => no need to get slot
			env
		);

		if (flag.is_callee) {
			// leave base object on the stack
			if (tmp_ret.is_id_loc) {
				ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), GET_LOCAL_CONTEXT);
				ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), GET_SLOT_N, tmp_str);
			} else if (tmp_ret.is_id_top) {
				ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), GET_GLOBAL_CONTEXT);
				ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), GET_SLOT_N, tmp_str);
			} else if (is_proto) {
				ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), DUP);
				ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), GET_PROTO);
			} else {
				ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), GET_SLOT_N, tmp_str);
			}

			ret = RETVAL(.has_base = IVM_TRUE);
		} else {
			if (tmp_ret.is_id_loc) {
				ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), GET_LOCAL_SLOT, tmp_str);
			} else if (tmp_ret.is_id_top) {
				ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), GET_GLOBAL_SLOT, tmp_str);
			} else if (is_proto) {
				ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), GET_PROTO);
			} else {
				ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), GET_SLOT, tmp_str);
			}
		}

		if (flag.is_top_level) {
			ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), POP);
		}
	} // else neither left value nor top level: don't generate

	return ret;
}

ilang_gen_value_t
ilang_gen_oop_expr_eval(ilang_gen_expr_t *expr,
						ilang_gen_flag_t flag,
						ilang_gen_env_t *env)
{
	ilang_gen_oop_expr_t *oop_expr = IVM_AS(expr, ilang_gen_oop_expr_t);
	ilang_gen_value_t ret = NORET();

	oop_expr->obj->eval(
		oop_expr->obj,
		FLAG(0),
		env
	);

	if (flag.is_left_val) {
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), SET_OOP, oop_expr->oop);
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), POP);
	} else {
		if (flag.is_callee) {
			ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), DUP);
			ret = RETVAL(.has_base = IVM_TRUE);
		}
		
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), GET_OOP, oop_expr->oop);

		if (flag.is_top_level) {
			ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), POP);
		}
	}

	return ret;
}
