#include "priv.h"

ilang_gen_value_t
ilang_gen_if_expr_eval(ilang_gen_expr_t *expr,
					   ilang_gen_flag_t flag,
					   ilang_gen_env_t *env)
{
	ilang_gen_if_expr_t *if_expr = IVM_AS(expr, ilang_gen_if_expr_t);
	ivm_size_t main_jmp, prev_elif_jmp = 0, cur, tmp_addr;
	ivm_size_t main_end_jmp = -1;
	ilang_gen_branch_t main_br, last_br, tmp_br;
	ilang_gen_branch_list_t *elifs;
	ilang_gen_branch_list_iterator_t biter;
	ilang_gen_value_t cond_ret;
	ilang_gen_addr_list_iterator_t iter;

	ilang_gen_addr_list_t *begin_ref, *end_ref;

	ilang_gen_addr_set_t addr_backup;

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

	addr_backup = env->addr;

	/*************** main branch ***************/
	end_ref = env->addr.end_ref = ilang_gen_addr_list_new(env);
	begin_ref = env->addr.begin_ref = ilang_gen_addr_list_new(env);

	cond_ret = main_br.cond->eval(
		main_br.cond,
		FLAG(.if_use_cond_reg = IVM_TRUE,
			 .has_branch = IVM_TRUE),
		env
	);

	if (cond_ret.use_branch) {
		// redirect all begin ref to the begining of the body
		cur = ivm_exec_cur(env->cur_exec);
		ILANG_GEN_ADDR_LIST_EACHPTR(begin_ref, iter) {
			tmp_addr = ILANG_GEN_ADDR_LIST_ITER_GET(iter);
			ivm_exec_setArgAt(env->cur_exec, tmp_addr, cur - tmp_addr);
		}
		ilang_gen_addr_list_empty(begin_ref);
	} else {
		main_jmp = ivm_exec_addInstr_l(
			env->cur_exec, GET_LINE(expr), JUMP_FALSE,
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
		main_end_jmp = ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), JUMP, 0);
	}

	/*************** main branch ***************/

	cur = ivm_exec_cur(env->cur_exec);

	if (cond_ret.use_branch) {
		// redirect all end ref to next branch
		ILANG_GEN_ADDR_LIST_EACHPTR(end_ref, iter) {
			tmp_addr = ILANG_GEN_ADDR_LIST_ITER_GET(iter);
			ivm_exec_setArgAt(env->cur_exec, tmp_addr, cur - tmp_addr);
		}
		ilang_gen_addr_list_empty(end_ref);
	} else {
		// main branch jump to next branch(elif or else)
		ivm_exec_setArgAt(
			env->cur_exec,
			main_jmp,
			cur - main_jmp
		);
	}

	/*************** elifs ***************/
	ivm_size_t elif_end_jmps[ilang_gen_branch_list_size(elifs) + 1];
	ivm_size_t *cur_end_jmp = elif_end_jmps, *end;

	ILANG_GEN_BRANCH_LIST_EACHPTR_R(elifs, biter) {
		tmp_br = ILANG_GEN_BRANCH_LIST_ITER_GET(biter);
		cond_ret = tmp_br.cond->eval(
			tmp_br.cond,
			FLAG(.if_use_cond_reg = IVM_TRUE),
			env
		);

		if (cond_ret.use_branch) {
			// redirect all begin ref to the begining of the body
			cur = ivm_exec_cur(env->cur_exec);
			ILANG_GEN_ADDR_LIST_EACHPTR(begin_ref, iter) {
				tmp_addr = ILANG_GEN_ADDR_LIST_ITER_GET(iter);
				ivm_exec_setArgAt(env->cur_exec, tmp_addr, cur - tmp_addr);
			}
			ilang_gen_addr_list_empty(begin_ref);
		} else {
			prev_elif_jmp = ivm_exec_addInstr_l( // jump to next elif/else if false
				env->cur_exec, GET_LINE(expr), JUMP_FALSE, 0
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
			*cur_end_jmp++ = ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), JUMP, 0);
		}

		cur = ivm_exec_cur(env->cur_exec);

		if (cond_ret.use_branch) {
			ILANG_GEN_ADDR_LIST_EACHPTR(end_ref, iter) {
				tmp_addr = ILANG_GEN_ADDR_LIST_ITER_GET(iter);
				ivm_exec_setArgAt(env->cur_exec, tmp_addr, cur - tmp_addr);
			}
			ilang_gen_addr_list_empty(end_ref);
		} else {
			ivm_exec_setArgAt(
				env->cur_exec,
				prev_elif_jmp,
				cur - prev_elif_jmp // jump to here
			);
		}
	}

	/*************** elifs ***************/

	/*************** else ***************/
	if (last_br.body) { // has else branch
		last_br.body->eval(
			last_br.body,
			FLAG(.is_top_level = flag.is_top_level),
			env
		);
	} else {
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), NEW_NONE);
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

	env->addr = addr_backup;

	return NORET();
}

ilang_gen_value_t
ilang_gen_while_expr_eval(ilang_gen_expr_t *expr,
						  ilang_gen_flag_t flag,
						  ilang_gen_env_t *env)
{
	ilang_gen_while_expr_t *while_expr = IVM_AS(expr, ilang_gen_while_expr_t);
	ivm_size_t start_addr, main_jmp, cur, tmp_addr;
	ilang_gen_value_t cond_ret;

	ilang_gen_addr_list_iterator_t riter;
	ilang_gen_addr_list_iterator_t iter;
	ilang_gen_addr_list_t *break_ref, *begin_ref, *end_ref;

	ilang_gen_addr_set_t addr_backup;

	GEN_ASSERT_NOT_LEFT_VALUE(expr, "while expression", flag);

	/*
		push_block

		start:
			[cond]
			jump_false end
			[body]
			jump start
		end:

		pop_block
	 */
	
	// backup
	addr_backup = env->addr;

	ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), PUSH_BLOCK);

	// reset break/continue/end/begin ref list
	start_addr = env->addr.continue_addr = ivm_exec_cur(env->cur_exec);
	break_ref = env->addr.break_ref = ilang_gen_addr_list_new(env);

	end_ref = env->addr.end_ref = ilang_gen_addr_list_new(env);
	begin_ref = env->addr.begin_ref = ilang_gen_addr_list_new(env);

	// condition
	cond_ret = while_expr->cond->eval(
		while_expr->cond,
		FLAG(.if_use_cond_reg = IVM_TRUE,
			 .has_branch = IVM_TRUE),
		env
	);

	if (cond_ret.use_branch) {
		// redirect all begin ref to the begining of the body
		cur = ivm_exec_cur(env->cur_exec);
		ILANG_GEN_ADDR_LIST_EACHPTR(begin_ref, iter) {
			tmp_addr = ILANG_GEN_ADDR_LIST_ITER_GET(iter);
			ivm_exec_setArgAt(env->cur_exec, tmp_addr, cur - tmp_addr);
		}
		ilang_gen_addr_list_empty(begin_ref);
	} else {
		main_jmp = ivm_exec_addInstr_l(
			env->cur_exec, GET_LINE(expr), JUMP_FALSE,
			0 /* replaced with else addr later */
		);
	}

	while_expr->body->eval(
		while_expr->body,
		FLAG(.is_top_level = IVM_TRUE),
		env
	);

	ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), JUMP,
						start_addr - ivm_exec_cur(env->cur_exec));

	cur = ivm_exec_cur(env->cur_exec);

	if (cond_ret.use_branch) {
		ILANG_GEN_ADDR_LIST_EACHPTR(end_ref, iter) {
			tmp_addr = ILANG_GEN_ADDR_LIST_ITER_GET(iter);
			ivm_exec_setArgAt(env->cur_exec, tmp_addr, cur - tmp_addr);
		}
		ilang_gen_addr_list_empty(end_ref);
	} else {
		ivm_exec_setArgAt(env->cur_exec, main_jmp,
						  cur - main_jmp);
	}

	/* reset all break jump addresses */
	ILANG_GEN_ADDR_LIST_EACHPTR(break_ref, riter) {
		tmp_addr = ILANG_GEN_ADDR_LIST_ITER_GET(riter);
		ivm_exec_setArgAt(env->cur_exec, tmp_addr, cur - tmp_addr);
	}

	ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), POP_BLOCK);

	if (!flag.is_top_level) {
		// return null in default
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), NEW_NONE);
	}

	env->addr = addr_backup;

	return NORET();
}

ilang_gen_value_t
ilang_gen_for_expr_eval(ilang_gen_expr_t *expr,
						ilang_gen_flag_t flag,
						ilang_gen_env_t *env)
{
	ilang_gen_for_expr_t *for_expr = IVM_AS(expr, ilang_gen_for_expr_t);

	ilang_gen_addr_set_t addr_backup = env->addr;
	ilang_gen_addr_list_iterator_t iter;
	ilang_gen_addr_list_t *break_ref;

	ivm_size_t tmp_addr, cur;

	GEN_ASSERT_NOT_LEFT_VALUE(expr, "for expression", flag);

	/*
		push_block

		<gen iteree>
		get_slot_n "iter"
		invoke_base

cont >	addr1:
			iter_next addr2(end loop label)
			invoke_base 0
			rprot_cac
			<gen var as leftval>

			<body>

			jump addr1
break >	addr2:
		
		pop_block
	 */
	
	env->addr.break_ref = break_ref = ilang_gen_addr_list_new(env);

	ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), PUSH_BLOCK);

	for_expr->iteree->eval(for_expr->iteree, FLAG(0), env);
	ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), GET_SLOT_N, "iter");
	ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), INVOKE_BASE, 0);

	env->addr.continue_addr = ivm_exec_cur(env->cur_exec);

	tmp_addr = ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), ITER_NEXT, 0);
	ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), INVOKE_BASE, 0);
	ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), RPROT_CAC);

	// ilang_gen_leftval_eval(for_expr->var, expr, env);
	for_expr->var->eval(for_expr->var, FLAG(.is_left_val = IVM_TRUE), env);

	for_expr->body->eval(for_expr->body, FLAG(.is_top_level = IVM_TRUE), env);

	ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), JUMP, tmp_addr - ivm_exec_cur(env->cur_exec));

	cur = ivm_exec_cur(env->cur_exec);
	ivm_exec_setArgAt(env->cur_exec, tmp_addr, cur - tmp_addr);

	// solve all the breaks
	ILANG_GEN_ADDR_LIST_EACHPTR(break_ref, iter) {
		tmp_addr = ILANG_GEN_ADDR_LIST_ITER_GET(iter);
		ivm_exec_setArgAt(env->cur_exec, tmp_addr, cur - tmp_addr);
	}

	ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), POP_BLOCK);

	if (!flag.is_top_level) {
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), NEW_NONE);
	}

	env->addr = addr_backup;

	return NORET();
}

ilang_gen_value_t
ilang_gen_try_expr_eval(ilang_gen_expr_t *expr,
						ilang_gen_flag_t flag,
						ilang_gen_env_t *env)
{
	ilang_gen_try_expr_t *try_expr = IVM_AS(expr, ilang_gen_try_expr_t);
	ivm_size_t addr1, addr2;
	ivm_char_t *tmp_str;
	const ivm_char_t *err;

	GEN_ASSERT_NOT_LEFT_VALUE(expr, "try expression", flag);

	/*
		push_block

		try:
			rprot_set catch
			[try body]
			rprot_cac
			jump final
		catch:
			[catch body]
			jump final
		final:
			pop_block
			[final body]
	 */
	
	ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), PUSH_BLOCK);

	/* try body */
	addr1 = ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), RPROT_SET, 0);
	try_expr->try_body->eval(try_expr->try_body, FLAG(.is_top_level = IVM_TRUE), env);
	ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), RPROT_CAC);
	addr2 = ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), JUMP, 0);

	/* catch body */
	ivm_exec_setArgAt(env->cur_exec, addr1, ivm_exec_cur(env->cur_exec) - addr1);

	if (!ilang_gen_token_value_isEmpty(try_expr->catch_body.arg)) {
		tmp_str = ivm_parser_parseStr_heap(
			env->heap,
			try_expr->catch_body.arg.val,
			try_expr->catch_body.arg.len,
			&err
		);

		if (!tmp_str) {
			GEN_ERR_FAILED_PARSE_STRING(expr, err);
		}

		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), SET_ARG, tmp_str);
	} else {
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), POP);
	}
	if (try_expr->catch_body.body) {
		try_expr->catch_body.body->eval(try_expr->catch_body.body, FLAG(.is_top_level = IVM_TRUE), env);
	}
	// addr1 = ivm_exec_addInstr_l(env->cur_exec, JUMP, 0);
	/* fallthrough */

	/* final body */
	ivm_exec_setArgAt(env->cur_exec, addr2, ivm_exec_cur(env->cur_exec) - addr2);

	ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), POP_BLOCK);

	if (try_expr->final_body) {
		try_expr->final_body->eval(try_expr->final_body, FLAG(.is_top_level = flag.is_top_level), env);
	} else if (!flag.is_top_level) {
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), NEW_NONE);
	}

	return NORET();
}
