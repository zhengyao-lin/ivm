#include "priv.h"

IVM_PRIVATE
ilang_gen_value_t
_ilang_gen_logic_and_expr_eval(ilang_gen_logic_expr_t *logic_expr,
							   ilang_gen_flag_t flag,
							   ilang_gen_env_t *env)
{
	/*
		and(value version):
				[lhe]
				jump_false end
				[rhe]
				jump_false end
				new_num_i 1
				jump very_end
			end:
				new_num_i 0
			very_end:

		and(branch version):
				[lhe]
				jump_false end
				[rhe]
				jump_false end
				jump begin
			begin:
				[while, if body or rhe of and expression]
			end: // else branch of if or end of while
	 */
	
	ilang_gen_expr_t *lhe, *rhe;
	ivm_size_t addr1, addr2, cur;
	ivm_list_t *end_ref, *begin_ref;
	ivm_list_t *end_ref_back, *begin_ref_back;
	ilang_gen_value_t tmp_ret;
	ilang_gen_value_t ret = NORET();
	IVM_LIST_ITER_TYPE(ivm_size_t) iter;

	lhe = logic_expr->lhe;
	rhe = logic_expr->rhe;

	end_ref_back = env->end_ref;
	begin_ref_back = env->begin_ref;

	if (flag.has_branch) { // branch version
		// the parent expr has branch struture
		// so end refs can just redirect to the parent expr
		
		ret.use_branch = IVM_TRUE;
		
		// rewrite begin ref
		env->begin_ref = begin_ref = ivm_list_new(sizeof(ivm_size_t));

		// lhe
		tmp_ret = lhe->eval(lhe, FLAG(.if_use_cond_reg = IVM_TRUE, .has_branch = IVM_TRUE), env);

		if (!tmp_ret.use_branch) {
			if (tmp_ret.use_cond_reg) {
				addr1 = ivm_exec_addInstr(env->cur_exec, JUMP_FALSE_R, 0);
			} else {
				addr1 = ivm_exec_addInstr(env->cur_exec, JUMP_FALSE, 0);
			}

			ivm_list_push(env->end_ref, &addr1); // add end ref
		}

		// all begin ref redirected to rhe gen
		cur = ivm_exec_cur(env->cur_exec);
		{
			IVM_LIST_EACHPTR(begin_ref, iter, ivm_size_t) {
				addr1 = IVM_LIST_ITER_GET(iter, ivm_size_t);
				ivm_exec_setArgAt(env->cur_exec, addr1, cur - addr1);
			}
		}

		ivm_list_free(begin_ref);

		// replace the current begin ref with the original one(parent expr)
		env->begin_ref = begin_ref_back;

		tmp_ret = rhe->eval(rhe, FLAG(.if_use_cond_reg = IVM_TRUE, .has_branch = IVM_TRUE), env);

		if (!tmp_ret.use_branch) {
			if (tmp_ret.use_cond_reg) {
				addr1 = ivm_exec_addInstr(env->cur_exec, JUMP_FALSE_R, 0);
			} else {
				addr1 = ivm_exec_addInstr(env->cur_exec, JUMP_FALSE, 0);
			}

			ivm_list_push(env->end_ref, &addr1); // add end ref
		}

		// the two expression all true
		// jump to branch body
		addr1 = ivm_exec_addInstr(env->cur_exec, JUMP, 0);
		ivm_list_push(env->begin_ref, &addr1); // add begin ref
	} else {
		// value version
		
		// create end ref list
		env->end_ref = end_ref = ivm_list_new(sizeof(ivm_size_t));
		env->begin_ref = begin_ref = ivm_list_new(sizeof(ivm_size_t));

		/******** lhe *******/

		tmp_ret = lhe->eval(lhe, FLAG(.if_use_cond_reg = IVM_TRUE, .has_branch = IVM_TRUE), env);

		if (!tmp_ret.use_branch) {
			if (tmp_ret.use_cond_reg) {
				addr1 = ivm_exec_addInstr(env->cur_exec, JUMP_FALSE_R, 0);
			} else {
				addr1 = ivm_exec_addInstr(env->cur_exec, JUMP_FALSE, 0);
			}

			ivm_list_push(end_ref, &addr1);
		}

		cur = ivm_exec_cur(env->cur_exec);

		// all ref to body begin redirected to rhe gen
		{
			IVM_LIST_EACHPTR(begin_ref, iter, ivm_size_t) {
				addr1 = IVM_LIST_ITER_GET(iter, ivm_size_t);
				ivm_exec_setArgAt(env->cur_exec, addr1, cur - addr1);
			}
		}
		ivm_list_empty(begin_ref);

		/******** rhe *******/

		tmp_ret = rhe->eval(rhe, FLAG(.if_use_cond_reg = IVM_TRUE, .has_branch = IVM_TRUE), env);

		if (!tmp_ret.use_branch) {
			if (tmp_ret.use_cond_reg) {
				addr1 = ivm_exec_addInstr(env->cur_exec, JUMP_FALSE_R, 0);
			} else {
				addr1 = ivm_exec_addInstr(env->cur_exec, JUMP_FALSE, 0);
			}

			ivm_list_push(end_ref, &addr1);
		}

		// true
		cur = ivm_exec_addInstr(env->cur_exec, NEW_NUM_I, 1);

		{
			IVM_LIST_EACHPTR(begin_ref, iter, ivm_size_t) {
				addr1 = IVM_LIST_ITER_GET(iter, ivm_size_t);
				ivm_exec_setArgAt(env->cur_exec, addr1, cur - addr1);
			}
		}

		// jump to end
		addr2 = ivm_exec_addInstr(env->cur_exec, JUMP, 0);

		// false
		cur = ivm_exec_addInstr(env->cur_exec, NEW_NUM_I, 0);

		{
			IVM_LIST_EACHPTR(end_ref, iter, ivm_size_t) {
				addr1 = IVM_LIST_ITER_GET(iter, ivm_size_t);
				ivm_exec_setArgAt(env->cur_exec, addr1, cur - addr1);
			}
		}

		// end of the whole expr
		ivm_exec_setArgAt(env->cur_exec, addr2, ivm_exec_cur(env->cur_exec) - addr2);

		ivm_list_free(end_ref);
		ivm_list_free(begin_ref);
	}

	env->end_ref = end_ref_back;
	env->begin_ref = begin_ref_back;

	return ret;
}

IVM_PRIVATE
ilang_gen_value_t
_ilang_gen_logic_or_expr_eval(ilang_gen_logic_expr_t *logic_expr,
							  ilang_gen_flag_t flag,
							  ilang_gen_env_t *env)
{
	/*
		or(value version):
				[lhe]
				jump_true begin
				[rhe]
				jump_true begin
				new_num_i 0
				jump very_end
			begin:
				new_num_i 1
			very_end:

		or(branch version):
				[lhe]
				jump_true begin
				[rhe]
				jump_true begin
				jump end
			begin:
				[while, if body or rhe of and expression]
			end: // else branch of if or end of while
	 */
	
	ilang_gen_expr_t *lhe, *rhe;
	ivm_size_t addr1, addr2, cur;
	ivm_list_t *end_ref, *begin_ref;
	ivm_list_t *end_ref_back, *begin_ref_back;
	ilang_gen_value_t tmp_ret;
	ilang_gen_value_t ret = NORET();
	IVM_LIST_ITER_TYPE(ivm_size_t) iter;

	lhe = logic_expr->lhe;
	rhe = logic_expr->rhe;

	end_ref_back = env->end_ref;
	begin_ref_back = env->begin_ref;

	if (flag.has_branch) { // branch version
		// the parent expr has branch struture
		// so end refs can just redirect to the parent expr
		
		ret.use_branch = IVM_TRUE;
		
		// rewrite end ref
		env->end_ref = end_ref = ivm_list_new(sizeof(ivm_size_t));

		// lhe
		tmp_ret = lhe->eval(lhe, FLAG(.if_use_cond_reg = IVM_TRUE, .has_branch = IVM_TRUE), env);

		if (!tmp_ret.use_branch) {
			if (tmp_ret.use_cond_reg) {
				addr1 = ivm_exec_addInstr(env->cur_exec, JUMP_TRUE_R, 0);
			} else {
				addr1 = ivm_exec_addInstr(env->cur_exec, JUMP_TRUE, 0);
			}

			ivm_list_push(env->begin_ref, &addr1); // add begin ref
		}

		// all end ref redirected to rhe gen
		cur = ivm_exec_cur(env->cur_exec);

		{
			IVM_LIST_EACHPTR(end_ref, iter, ivm_size_t) {
				addr1 = IVM_LIST_ITER_GET(iter, ivm_size_t);
				ivm_exec_setArgAt(env->cur_exec, addr1, cur - addr1);
			}
		}

		ivm_list_free(end_ref);

		// replace the current end ref with the original one(parent expr)
		env->end_ref = end_ref_back;

		tmp_ret = rhe->eval(rhe, FLAG(.if_use_cond_reg = IVM_TRUE, .has_branch = IVM_TRUE), env);

		if (!tmp_ret.use_branch) {
			if (tmp_ret.use_cond_reg) {
				addr1 = ivm_exec_addInstr(env->cur_exec, JUMP_TRUE_R, 0);
			} else {
				addr1 = ivm_exec_addInstr(env->cur_exec, JUMP_TRUE, 0);
			}

			ivm_list_push(env->begin_ref, &addr1); // add begin ref
		}

		// the two expression all false
		// jump to branch body
		addr1 = ivm_exec_addInstr(env->cur_exec, JUMP, 0);
		ivm_list_push(env->end_ref, &addr1); // add begin ref
	} else {
		// value version
		
		// create end ref list
		env->end_ref = end_ref = ivm_list_new(sizeof(ivm_size_t));
		env->begin_ref = begin_ref = ivm_list_new(sizeof(ivm_size_t));

		/******** lhe *******/

		tmp_ret = lhe->eval(lhe, FLAG(.if_use_cond_reg = IVM_TRUE, .has_branch = IVM_TRUE), env);

		if (!tmp_ret.use_branch) {
			if (tmp_ret.use_cond_reg) {
				addr1 = ivm_exec_addInstr(env->cur_exec, JUMP_TRUE_R, 0);
			} else {
				addr1 = ivm_exec_addInstr(env->cur_exec, JUMP_TRUE, 0);
			}

			ivm_list_push(begin_ref, &addr1);
		}

		cur = ivm_exec_cur(env->cur_exec);

		// all end ref redirected to rhe gen
		{
			IVM_LIST_EACHPTR(end_ref, iter, ivm_size_t) {
				addr1 = IVM_LIST_ITER_GET(iter, ivm_size_t);
				ivm_exec_setArgAt(env->cur_exec, addr1, cur - addr1);
			}
		}
		ivm_list_empty(end_ref);

		/******** rhe *******/

		tmp_ret = rhe->eval(rhe, FLAG(.if_use_cond_reg = IVM_TRUE, .has_branch = IVM_TRUE), env);

		if (!tmp_ret.use_branch) {
			if (tmp_ret.use_cond_reg) {
				addr1 = ivm_exec_addInstr(env->cur_exec, JUMP_TRUE_R, 0);
			} else {
				addr1 = ivm_exec_addInstr(env->cur_exec, JUMP_TRUE, 0);
			}

			ivm_list_push(begin_ref, &addr1);
		}

		cur = ivm_exec_cur(env->cur_exec);

		{
			IVM_LIST_EACHPTR(end_ref, iter, ivm_size_t) {
				addr1 = IVM_LIST_ITER_GET(iter, ivm_size_t);
				ivm_exec_setArgAt(env->cur_exec, addr1, cur - addr1);
			}
		}

		// false
		cur = ivm_exec_addInstr(env->cur_exec, NEW_NUM_I, 0);

		// jump to end
		addr2 = ivm_exec_addInstr(env->cur_exec, JUMP, 0);

		// true
		cur = ivm_exec_cur(env->cur_exec);

		{
			IVM_LIST_EACHPTR(begin_ref, iter, ivm_size_t) {
				addr1 = IVM_LIST_ITER_GET(iter, ivm_size_t);
				ivm_exec_setArgAt(env->cur_exec, addr1, cur - addr1);
			}
		}

		cur = ivm_exec_addInstr(env->cur_exec, NEW_NUM_I, 1);

		// end of the whole expr
		ivm_exec_setArgAt(env->cur_exec, addr2, ivm_exec_cur(env->cur_exec) - addr2);

		ivm_list_free(end_ref);
		ivm_list_free(begin_ref);
	}

	env->end_ref = end_ref_back;
	env->begin_ref = begin_ref_back;

	return ret;
}

ilang_gen_value_t
ilang_gen_logic_expr_eval(ilang_gen_expr_t *expr,
						  ilang_gen_flag_t flag,
						  ilang_gen_env_t *env)
{
	ilang_gen_logic_expr_t *logic_expr = IVM_AS(expr, ilang_gen_logic_expr_t);

	GEN_ASSERT_NOT_LEFT_VALUE(expr, "logic expression", flag);

	if (flag.is_top_level &&
		!expr->check(expr, CHECK_SE())) {
		/* is top level and has no side effect */
		return NORET();
	}

	if (logic_expr->type == ILANG_GEN_LOGIC_AND) {
		return _ilang_gen_logic_and_expr_eval(logic_expr, flag, env);
	} else {
		return _ilang_gen_logic_or_expr_eval(logic_expr, flag, env);
	}

	IVM_FATAL("unsupported logic expr type");

	return NORET();
}