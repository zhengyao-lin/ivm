#include "pub/const.h"

#include "vm/native/native.h"

#include "util/opt.h"

#include "priv.h"

ilang_gen_value_t
ilang_gen_fn_expr_eval(ilang_gen_expr_t *expr,
					   ilang_gen_flag_t flag,
					   ilang_gen_env_t *env)
{
	ilang_gen_fn_expr_t *func = IVM_AS(expr, ilang_gen_fn_expr_t);
	ivm_exec_t *exec, *exec_backup;
	ivm_size_t exec_id;
	ilang_gen_param_t *tmp_param1, *tmp_param2;
	ilang_gen_param_list_t *params;
	ilang_gen_param_list_iterator_t piter, chk_iter;
	ivm_char_t *tmp_str;
	ivm_bool_t has_varg = IVM_FALSE;
	ivm_int_t cur_param, param_count;
	ivm_size_t sp_back;
	const ivm_char_t *err;

	ilang_gen_addr_set_t addr_backup = env->addr;
	sp_back = env->sp;
	env->sp = 0;

	GEN_ASSERT_NOT_LEFT_VALUE(expr, "function expression", flag);

	exec = ivm_exec_new(env->str_pool);
	exec_id = ivm_exec_unit_registerExec(env->unit, exec);
	exec_backup = env->cur_exec;
	env->cur_exec = exec;

	env->addr = ilang_gen_addr_set_init();

	// ivm_exec_setSourcePos(exec, env->file);

	/*
		ink calling convention:
			1. first argument, top of the stack
			2. last argument, bottom of the stack
	 */
	params = func->params;

	if (params) {
		param_count = ilang_gen_param_list_size(params);
		cur_param = 0;

		if (param_count >= IVM_DEFAULT_SLOT_TABLE_SIZE) {
			ivm_exec_addInstr_l(exec, GET_LINE(expr), EXPAND_LOC, param_count);
		}

		ILANG_GEN_PARAM_LIST_EACHPTR_R(params, piter) {
			cur_param++;

			tmp_param1 = ILANG_GEN_PARAM_LIST_ITER_GET_PTR(piter);

			ILANG_GEN_PARAM_LIST_EACHPTR_R(params, chk_iter) {
				tmp_param2 = ILANG_GEN_PARAM_LIST_ITER_GET_PTR(chk_iter);

				if (tmp_param1 != tmp_param2 &&
					!IVM_STRNCMP(tmp_param1->name.val, tmp_param1->name.len,
								 tmp_param2->name.val, tmp_param2->name.len)) {
					GEN_ERR_DUP_PARAM_NAME(expr, tmp_param2->name.val, tmp_param2->name.len);
				}
			}

			tmp_str = ivm_parser_parseStr_heap(env->heap, tmp_param1->name.val, tmp_param1->name.len, &err);
			
			if (!tmp_str) {
				GEN_ERR_FAILED_PARSE_STRING(expr, err);
			}

			if (tmp_param1->is_varg) {
				if (has_varg) {
					GEN_ERR_MULTIPLE_VARG(expr);
				}
				has_varg = IVM_TRUE;
				ivm_exec_addInstr_l(exec, GET_LINE(expr), NEW_VARG, param_count - cur_param);
			}

			if (tmp_param1->def) {
				tmp_param1->def->eval(tmp_param1->def, FLAG(0), env);
				ivm_exec_addInstr_l(exec, GET_LINE(expr), SET_DEF, tmp_str);
			} else {
				if (tmp_str) {
					ivm_exec_addInstr_l(exec, GET_LINE(expr), SET_ARG, tmp_str);
				} else {
					ivm_exec_addInstr_l(exec, GET_LINE(expr), SET_ARG, IVM_NATIVE_VARG_NAME);
				}
			}
		}
	}

	func->body->eval(
		func->body,
		FLAG(0), // function body is not top-level(need last value)
		env
	);

	env->cur_exec = exec_backup;
	ivm_opt_optExec(exec);

	env->addr = addr_backup;
	env->sp = sp_back;

	if (!flag.is_top_level) {
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), NEW_FUNC, exec_id);
	}

	return NORET();
}

ilang_gen_value_t
ilang_gen_intr_expr_eval(ilang_gen_expr_t *expr,
						 ilang_gen_flag_t flag,
						 ilang_gen_env_t *env)
{
	ilang_gen_intr_expr_t *intr = IVM_AS(expr, ilang_gen_intr_expr_t);

	GEN_ASSERT_NOT_LEFT_VALUE(expr, "interrupt expression", flag);
	// GEN_ASSERT_NO_NESTED_RET(expr, flag)

	ivm_size_t cur = ivm_exec_cur(env->cur_exec);

	switch (intr->sig) {
		case ILANG_GEN_INTR_RET:
			if (intr->val) {
				intr->val->eval(intr->val, FLAG(0), env);
			} else {
				ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), NEW_NONE);
			}
			ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), RETURN);

			break;

		case ILANG_GEN_INTR_CONT:
			if (env->addr.continue_addr != -1) {
				if (intr->val) {
					GEN_WARN_GENERAL(expr, GEN_ERR_MSG_BREAK_OR_CONT_IGNORE_ARG);
				}
				ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), INT_LOOP, env->addr.continue_addr - cur);
			} else {
				GEN_ERR_GENERAL(expr, GEN_ERR_MSG_BREAK_OR_CONT_OUTSIDE_LOOP);
			}

			break;

		case ILANG_GEN_INTR_BREAK:
			if (env->addr.break_ref) {
				if (intr->val) {
					GEN_WARN_GENERAL(expr, GEN_ERR_MSG_BREAK_OR_CONT_IGNORE_ARG);
				}
				ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), INT_LOOP, 0);
				ilang_gen_addr_list_push(env->addr.break_ref, cur);
			} else {
				GEN_ERR_GENERAL(expr, GEN_ERR_MSG_BREAK_OR_CONT_OUTSIDE_LOOP);
			}

			break;
	
		case ILANG_GEN_INTR_RAISE:
			intr->val->eval(intr->val, FLAG(0), env);
			ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), RAISE);

			break;

		case ILANG_GEN_INTR_RESUME:
			if (intr->with) {
				intr->with->eval(intr->with, FLAG(0), env);
			} else {
				ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), NEW_NONE);
			}

			intr->val->eval(intr->val, FLAG(0), env);
			ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), RESUME);

			if (flag.is_top_level) {
				ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), POP);
			}

			break;

		case ILANG_GEN_INTR_YIELD:
			if (intr->val) {
				intr->val->eval(intr->val, FLAG(0), env);
			} else {
				ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), NEW_NONE);
			}
			
			ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), YIELD);
			
			if (flag.is_top_level) {
				ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), POP);
			}

			break;

		default:
			IVM_FATAL("unsupported interrupt signal");
	}

	return NORET();
}

ilang_gen_value_t
ilang_gen_assign_expr_eval(ilang_gen_expr_t *expr,
						   ilang_gen_flag_t flag,
						   ilang_gen_env_t *env)
{
	ilang_gen_assign_expr_t *assign = IVM_AS(expr, ilang_gen_assign_expr_t);

	// assign expression itself should not be left value
	GEN_ASSERT_NOT_LEFT_VALUE(expr, "assign expression", flag);

	assign->rhe->eval(assign->rhe, FLAG(0), env);
	INC_SP();
	if (!flag.is_top_level) {
		INC_SP();
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), DUP);
	}
	// ilang_gen_leftval_eval(assign->lhe, expr, env);
	assign->lhe->eval(assign->lhe, FLAG(.is_left_val = IVM_TRUE), env);

	DEC_SP();
	if (!flag.is_top_level) {
		DEC_SP();
	}

	return NORET();
}

ilang_gen_value_t
ilang_gen_fork_expr_eval(ilang_gen_expr_t *expr,
						 ilang_gen_flag_t flag,
						 ilang_gen_env_t *env)
{
	ilang_gen_fork_expr_t *fork_expr = IVM_AS(expr, ilang_gen_fork_expr_t);
	
	GEN_ASSERT_NOT_LEFT_VALUE(expr, "assign expression", flag);

	if (fork_expr->is_group) {
		fork_expr->forkee->eval(fork_expr->forkee, FLAG(0), env);
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), GROUP);
		if (flag.is_top_level) {
			ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), POP);
		}
	} else {
		fork_expr->forkee->eval(fork_expr->forkee, FLAG(0), env);
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), FORK);
		if (!flag.is_top_level) {
			ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), NEW_NONE);
		}
	}

	return NORET();
}
