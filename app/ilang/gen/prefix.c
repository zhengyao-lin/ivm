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
	ilang_gen_param_t *tmp_param;
	ilang_gen_param_list_t *params;
	ilang_gen_param_list_iterator_t piter;
	ivm_char_t *tmp_str;
	ivm_bool_t has_varg = IVM_FALSE;
	ivm_int_t arg_count;

	GEN_ASSERT_NOT_LEFT_VALUE(expr, "function expression", flag);

	exec = ivm_exec_new(env->str_pool);
	exec_id = ivm_exec_unit_registerExec(env->unit, exec);
	exec_backup = env->cur_exec;
	env->cur_exec = exec;

	/*
		ink calling convention:
			1. first argument, top of the stack
			2. last argument, bottom of the stack
	 */
	params = func->params;
	if (params) {
		arg_count = 0;
		ILANG_GEN_PARAM_LIST_EACHPTR_R(params, piter) {
			arg_count++;

			tmp_param = ILANG_GEN_PARAM_LIST_ITER_GET_PTR(piter);
			tmp_str = ivm_parser_parseStr_heap(env->heap, tmp_param->name.val, tmp_param->name.len);
			
			if (tmp_param->is_varg) {
				if (has_varg) {
					GEN_ERR_MULTIPLE_VARG(expr);
				}
				has_varg = IVM_TRUE;
				ivm_exec_addInstr(exec, NEW_VARG, ilang_gen_param_list_size(params) - arg_count);
			}

			if (tmp_str) {
				ivm_exec_addInstr(exec, SET_ARG, tmp_str);
			} else {
				ivm_exec_addInstr(exec, SET_ARG, "$varg");
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

	if (!flag.is_top_level) {
		ivm_exec_addInstr(env->cur_exec, NEW_FUNC, exec_id);
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
				ivm_exec_addInstr(env->cur_exec, NEW_NULL);
			}
			ivm_exec_addInstr(env->cur_exec, RETURN);
			break;

		case ILANG_GEN_INTR_CONT:
			if (env->continue_addr != -1) {
				if (intr->val) {
					GEN_WARN_GENERAL(expr, GEN_ERR_MSG_BREAK_OR_CONT_IGNORE_ARG);
				}
				ivm_exec_addInstr(env->cur_exec, JUMP, env->continue_addr - cur);
			} else {
				GEN_ERR_GENERAL(expr, GEN_ERR_MSG_BREAK_OR_CONT_OUTSIDE_LOOP);
			}
			break;

		case ILANG_GEN_INTR_BREAK:
			if (env->break_ref) {
				if (intr->val) {
					GEN_WARN_GENERAL(expr, GEN_ERR_MSG_BREAK_OR_CONT_IGNORE_ARG);
				}
				ivm_exec_addInstr(env->cur_exec, JUMP, 0);
				ilang_gen_addr_list_push(env->break_ref, cur);
			} else {
				GEN_ERR_GENERAL(expr, GEN_ERR_MSG_BREAK_OR_CONT_OUTSIDE_LOOP);
			}
			break;
	
		case ILANG_GEN_INTR_RAISE:
			IVM_ASSERT(intr->val, "impossible");
			intr->val->eval(intr->val, FLAG(0), env);
			ivm_exec_addInstr(env->cur_exec, RAISE);
			break;

		case ILANG_GEN_INTR_YIELD:
			if (intr->val) {
				intr->val->eval(intr->val, FLAG(0), env);
			}
			
			if (intr->to) {
				intr->to->eval(intr->to, FLAG(0), env);
				ivm_exec_addInstr(env->cur_exec, YIELD_TO);
			} else {
				ivm_exec_addInstr(env->cur_exec, YIELD);
			}
			
			if (flag.is_top_level) {
				ivm_exec_addInstr(env->cur_exec, POP);
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
	if (!flag.is_top_level) {
		ivm_exec_addInstr(env->cur_exec, DUP);
	}
	assign->lhe->eval(
		assign->lhe,
		FLAG(
			.is_left_val = IVM_TRUE
		),
		env
	);

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
		ivm_exec_addInstr(env->cur_exec, GROUP);
		if (flag.is_top_level) {
			ivm_exec_addInstr(env->cur_exec, POP);
		}
	} else {
		fork_expr->forkee->eval(fork_expr->forkee, FLAG(0), env);
		ivm_exec_addInstr(env->cur_exec, FORK);
		if (!flag.is_top_level) {
			ivm_exec_addInstr(env->cur_exec, NEW_NULL);
		}
	}

	return NORET();
}
