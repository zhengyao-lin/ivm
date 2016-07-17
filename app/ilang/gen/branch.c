#include "priv.h"

ilang_gen_value_t
ilang_gen_if_expr_eval(ilang_gen_expr_t *expr,
					   ilang_gen_flag_t flag,
					   ilang_gen_env_t *env)
{
	ilang_gen_if_expr_t *if_expr = IVM_AS(expr, ilang_gen_if_expr_t);
	ivm_size_t main_jmp, prev_elif_jmp = 0;
	ivm_size_t main_end_jmp = -1;
	ilang_gen_branch_t main_br, last_br, tmp_br;
	ilang_gen_branch_list_t *elifs;
	ilang_gen_branch_list_iterator_t biter;
	ilang_gen_value_t cond_ret;

	GEN_ASSERT_NOT_LEFT_VALUE(expr, "if expression", flag);

	if (flag.is_top_level &&
		!expr->check(expr, CHECK_SE())) {
		return NORET();
	}

	/*
			[main.cond]
		jump_false next1
			[main.body]
			jump end
		next1:
			[elif1.cond]
			jump_false next2
			[elif1.body]
			jump end
			.
			.
			.
		else:
			[else.cond]
			jump_false end
			[else.body]
		end:
	 */

	main_br = if_expr->main;
	last_br = if_expr->last;
	elifs = if_expr->elifs;

	/*************** main branch ***************/
	cond_ret = main_br.cond->eval(
		main_br.cond,
		FLAG(.if_use_cond_reg = IVM_TRUE),
		env
	);

	// use vreg to opt
	if (cond_ret.use_cond_reg) {
		main_jmp = ivm_exec_addInstr(
			env->cur_exec, JUMP_FALSE_R,
			0 /* replaced with else addr later */
		);
	} else {
		main_jmp = ivm_exec_addInstr(
			env->cur_exec, JUMP_FALSE,
			0 /* replaced with else addr later */
		);
	}

	// body
	main_br.body->eval(
		main_br.body,
		FLAG(.is_top_level = flag.is_top_level),
		env
	);

	// end of if branch => jump to end of if expression
	if (last_br.body || ilang_gen_branch_list_size(elifs)) {
		// if there are other branches
		main_end_jmp = ivm_exec_addInstr(env->cur_exec, JUMP, 0);
	}

	/*************** main branch ***************/

	// main branch jump to next branch(elif or else)
	ivm_exec_setArgAt(
		env->cur_exec,
		main_jmp,
		ivm_exec_cur(env->cur_exec) - main_jmp
	);

	/*************** elifs ***************/
	ivm_size_t elif_end_jmps[ilang_gen_branch_list_size(elifs) + 1];
	ivm_size_t *cur_end_jmp = elif_end_jmps, *end;

	ILANG_GEN_BRANCH_LIST_EACHPTR_R(elifs, biter) {
		if (prev_elif_jmp) { // has previous elif
			ivm_exec_setArgAt(
				env->cur_exec,
				prev_elif_jmp,
				ivm_exec_cur(env->cur_exec) - prev_elif_jmp // jump to here
			);
		}

		tmp_br = ILANG_GEN_BRANCH_LIST_ITER_GET(biter);
		cond_ret = tmp_br.cond->eval(
			tmp_br.cond,
			FLAG(.if_use_cond_reg = IVM_TRUE),
			env
		);

		if (cond_ret.use_cond_reg) {
			prev_elif_jmp = ivm_exec_addInstr(
				env->cur_exec, JUMP_FALSE_R, 0
			);
		} else {
			prev_elif_jmp = ivm_exec_addInstr( // jump to next elif/else if false
				env->cur_exec, JUMP_FALSE, 0
			);
		}

		tmp_br.body->eval(
			tmp_br.body,
			FLAG(.is_top_level = flag.is_top_level),
			env
		);

		if (!ILANG_GEN_BRANCH_LIST_ITER_IS_LAST(elifs, biter) ||
			last_br.body) {
			// has branch(es) following
			*cur_end_jmp++ = ivm_exec_addInstr(env->cur_exec, JUMP, 0);
		}
	}

	/*************** elifs ***************/

	if (prev_elif_jmp) { // has previous elif
		ivm_exec_setArgAt(
			env->cur_exec,
			prev_elif_jmp,
			ivm_exec_cur(env->cur_exec) - prev_elif_jmp // jump to here
		);
	}

	/*************** else ***************/
	if (last_br.body) { // has else branch
		last_br.body->eval(
			last_br.body,
			FLAG(.is_top_level = flag.is_top_level),
			env
		);
	}
	/*************** else ***************/

	// set end jump in if branch
	if (main_end_jmp != -1) {
		ivm_exec_setArgAt(
			env->cur_exec,
			main_end_jmp,
			ivm_exec_cur(env->cur_exec) - main_end_jmp
		);
	}

	for (end = cur_end_jmp,
		 cur_end_jmp = elif_end_jmps;
		 cur_end_jmp != end; cur_end_jmp++) {
		ivm_exec_setArgAt(
			env->cur_exec,
			*cur_end_jmp,
			ivm_exec_cur(env->cur_exec) - *cur_end_jmp
		);
	}

	return NORET();
}

ilang_gen_value_t
ilang_gen_while_expr_eval(ilang_gen_expr_t *expr,
						  ilang_gen_flag_t flag,
						  ilang_gen_env_t *env)
{
	ilang_gen_while_expr_t *while_expr = IVM_AS(expr, ilang_gen_while_expr_t);
	ivm_size_t start_addr, main_jmp;
	ivm_size_t cont_addr_back;
	ivm_list_t *break_ref_back, *break_ref;
	ilang_gen_value_t cond_ret;
	IVM_LIST_ITER_TYPE(ivm_size_t) riter;

	if (flag.is_top_level &&
		!expr->check(expr, CHECK_SE())) {
		return NORET();
	}

	/*
		start:
			[cond]
			jump_false end
			[body]
			jump start
		end:
	 */
	
	cont_addr_back = env->continue_addr;
	break_ref_back = env->break_ref;

	env->continue_addr = start_addr = ivm_exec_cur(env->cur_exec);
	break_ref = env->break_ref = ivm_list_new(sizeof(ivm_size_t));

	cond_ret = while_expr->cond->eval(
		while_expr->cond,
		FLAG(.if_use_cond_reg = IVM_TRUE),
		env
	);

	if (cond_ret.use_cond_reg) {
		main_jmp = ivm_exec_addInstr(
			env->cur_exec, JUMP_FALSE_R,
			0 /* replaced with else addr later */
		);
	} else {
		main_jmp = ivm_exec_addInstr(
			env->cur_exec, JUMP_FALSE,
			0 /* replaced with else addr later */
		);
	}

	while_expr->body->eval(
		while_expr->body,
		FLAG(.is_top_level = IVM_TRUE),
		env
	);

	ivm_exec_addInstr(env->cur_exec, JUMP,
					  start_addr - ivm_exec_cur(env->cur_exec));

	ivm_exec_setArgAt(env->cur_exec, main_jmp,
					  ivm_exec_cur(env->cur_exec) - main_jmp);

	ivm_size_t cur = ivm_exec_cur(env->cur_exec);
	ivm_size_t jmp_addr;

	/* reset all break jump addresses */
	IVM_LIST_EACHPTR(break_ref, riter, ivm_size_t) {
		jmp_addr = IVM_LIST_ITER_GET(riter, ivm_size_t);
		ivm_exec_setArgAt(env->cur_exec, jmp_addr, cur - jmp_addr);
	}

	if (!flag.is_top_level) {
		// return null in default
		ivm_exec_addInstr(env->cur_exec, NEW_NULL);
	}

	ivm_list_free(break_ref);
	env->continue_addr = cont_addr_back;
	env->break_ref = break_ref_back;

	return NORET();
}
