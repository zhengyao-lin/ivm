#include "pub/type.h"
#include "pub/com.h"
#include "pub/vm.h"
#include "pub/err.h"

#include "util/parser.h"

#include "gen.h"

#define FLAG ilang_gen_flag_build
#define RETVAL ilang_gen_value_build
#define NORET() ((ilang_gen_value_t) { 0 })

#define GEN_ERR(p, ...) \
	IVM_TRACE("ilang generator error: at line %zd pos %zd: ", (p).line, (p).pos); \
	IVM_TRACE(__VA_ARGS__); \
	IVM_TRACE("\n"); \
	IVM_EXIT(1);

#define GEN_WARN(p, ...) \
	IVM_TRACE("ilang generator warning: at line %zd pos %zd: ", (p).line, (p).pos); \
	IVM_TRACE(__VA_ARGS__); \
	IVM_TRACE("\n");

#define GEN_ERR_MSG_CANNOT_ASSIGN_TO(expr_name)						"cannot assign to %s", (expr_name)
#define GEN_ERR_MSG_NESTED_RET										"nested return expression"
#define GEN_ERR_MSG_FAILED_PARSE_NUM(val, len)						"failed parse num '%.*s'", (int)(len), (val)
#define GEN_ERR_MSG_UNSUPPORTED_UNARY_OP(type)						"unsupported unary operation type %d", (type)
#define GEN_ERR_MSG_UNSUPPORTED_BINARY_OP(type)						"unsupported binary operation type %d", (type)
#define GEN_ERR_MSG_UNSUPPORTED_CMP_TYPE(type)						"unsupported compare type type %d", (type)

#define GEN_ASSERT_NOT_LEFT_VALUE(expr, name, flag) \
	if ((flag).is_left_val) { \
		GEN_ERR((expr)->pos, GEN_ERR_MSG_CANNOT_ASSIGN_TO(name)); \
	}

#define GEN_WARN_NO_NESTED_RET(expr, flag) \
	if (!(flag).is_top_level) { \
		GEN_WARN((expr)->pos, GEN_ERR_MSG_NESTED_RET); \
	}

ilang_gen_value_t
ilang_gen_int_expr_eval(ilang_gen_expr_t *expr,
						ilang_gen_flag_t flag,
						ilang_gen_env_t *env)
{
	ilang_gen_int_expr_t *int_expr = IVM_AS(expr, ilang_gen_int_expr_t);
	ivm_double_t val;
	ivm_bool_t err = IVM_FALSE;

	GEN_ASSERT_NOT_LEFT_VALUE(expr, "integer expression", flag);

	if (!flag.is_top_level) {
		val = ivm_parser_parseNum(
			int_expr->val.val,
			int_expr->val.len,
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
		ivm_exec_addInstr(env->cur_exec, NEW_NUM_F, val);
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

	tmp_str = ivm_parser_parseStr(
		str_expr->val.val,
		str_expr->val.len
	);

	ivm_exec_addInstr(env->cur_exec, NEW_STR, tmp_str);

	MEM_FREE(tmp_str);

	return NORET();
}

ilang_gen_value_t
ilang_gen_id_expr_eval(ilang_gen_expr_t *expr,
					   ilang_gen_flag_t flag,
					   ilang_gen_env_t *env)
{
	ilang_gen_id_expr_t *id_expr = IVM_AS(expr, ilang_gen_id_expr_t);
	ivm_char_t *tmp_str;

	tmp_str = ivm_parser_parseStr(
		id_expr->val.val,
		id_expr->val.len
	);

	if (flag.is_left_val) {
		if (!flag.is_top_level) {
			ivm_exec_addInstr(env->cur_exec, DUP);
		}

		ivm_exec_addInstr(env->cur_exec, SET_CONTEXT_SLOT, tmp_str);
	} else if (!flag.is_top_level) {
		ivm_exec_addInstr(env->cur_exec, GET_CONTEXT_SLOT, tmp_str);
	}

	MEM_FREE(tmp_str);

	return NORET();
}

ilang_gen_value_t
ilang_gen_call_expr_eval(ilang_gen_expr_t *expr,
						 ilang_gen_flag_t flag,
						 ilang_gen_env_t *env)
{
	ilang_gen_call_expr_t *call_expr = IVM_AS(expr, ilang_gen_call_expr_t);
	ilang_gen_expr_list_t *args;
	ilang_gen_expr_list_iterator_t aiter;
	ilang_gen_expr_t *tmp_arg;


	GEN_ASSERT_NOT_LEFT_VALUE(expr, "call expression", flag);

	args = call_expr->args;
	// generate in a reverse order(original order in ast)
	ILANG_GEN_EXPR_LIST_EACHPTR(args, aiter) {
		tmp_arg = ILANG_GEN_EXPR_LIST_ITER_GET(aiter);
		tmp_arg->eval(tmp_arg, FLAG(0), env);
	}

	call_expr->callee->eval(
		call_expr->callee,
		FLAG(0), env
	);

	ivm_exec_addInstr(
		env->cur_exec, INVOKE,
		ilang_gen_expr_list_size(args)
	);

	if (flag.is_top_level) {
		ivm_exec_addInstr(env->cur_exec, POP);
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

	tmp_str = ivm_parser_parseStr(
		slot_expr->slot.val,
		slot_expr->slot.len
	);

	if (flag.is_left_val) {
		slot_expr->obj->eval(
			slot_expr->obj,
			FLAG(0),
			env
		);
		ivm_exec_addInstr(env->cur_exec, SET_SLOT, tmp_str);
		if (flag.is_top_level) {
			ivm_exec_addInstr(env->cur_exec, POP);
		}
	} else if (!flag.is_top_level) {
		slot_expr->obj->eval(
			slot_expr->obj,
			FLAG(0),
			env
		);
		ivm_exec_addInstr(env->cur_exec, GET_SLOT, tmp_str);
	} // else neither left value nor top level: don't generate

	MEM_FREE(tmp_str);

	return NORET();
}

ilang_gen_value_t
ilang_gen_unary_expr_eval(ilang_gen_expr_t *expr,
						  ilang_gen_flag_t flag,
						  ilang_gen_env_t *env)
{
	ilang_gen_unary_expr_t *unary_expr = IVM_AS(expr, ilang_gen_unary_expr_t);

	GEN_ASSERT_NOT_LEFT_VALUE(expr, "unary expression", flag);

	if (!flag.is_top_level) {
		switch (unary_expr->type) {
			case IVM_UNIOP_ID(NOT):
				unary_expr->opr->eval(
					unary_expr->opr,
					FLAG(0), env
				);
				ivm_exec_addInstr(env->cur_exec, NOT);
				break;
			default:
				IVM_FATAL(GEN_ERR_MSG_UNSUPPORTED_UNARY_OP(unary_expr->type));
		}
	}

	return NORET();
}

ilang_gen_value_t
ilang_gen_binary_expr_eval(ilang_gen_expr_t *expr,
						   ilang_gen_flag_t flag,
						   ilang_gen_env_t *env)
{
	ilang_gen_binary_expr_t *binary_expr = IVM_AS(expr, ilang_gen_binary_expr_t);

	GEN_ASSERT_NOT_LEFT_VALUE(expr, "binary expression", flag);

	if (!flag.is_top_level) {
		binary_expr->op1->eval(
			binary_expr->op1,
			FLAG(0), env
		);

		binary_expr->op2->eval(
			binary_expr->op2,
			FLAG(0), env
		);

		switch (binary_expr->type) {
			case IVM_BINOP_ID(ADD):
				ivm_exec_addInstr(env->cur_exec, ADD);
				break;
			case IVM_BINOP_ID(SUB):
				ivm_exec_addInstr(env->cur_exec, SUB);
				break;
			case IVM_BINOP_ID(MUL):
				ivm_exec_addInstr(env->cur_exec, MUL);
				break;
			case IVM_BINOP_ID(DIV):
				ivm_exec_addInstr(env->cur_exec, DIV);
				break;
			case IVM_BINOP_ID(MOD):
				ivm_exec_addInstr(env->cur_exec, MOD);
				break;
			default:
				IVM_FATAL(GEN_ERR_MSG_UNSUPPORTED_BINARY_OP(binary_expr->type));
		}
	}

	return NORET();
}

ilang_gen_value_t
ilang_gen_cmp_expr_eval(ilang_gen_expr_t *expr,
						ilang_gen_flag_t flag,
						ilang_gen_env_t *env)
{
	ilang_gen_cmp_expr_t *cmp_expr = IVM_AS(expr, ilang_gen_cmp_expr_t);

	GEN_ASSERT_NOT_LEFT_VALUE(expr, "compare expression", flag);

	if (!flag.is_top_level) {
		cmp_expr->op1->eval(
			cmp_expr->op1,
			FLAG(0), env
		);

		cmp_expr->op2->eval(
			cmp_expr->op2,
			FLAG(0), env
		);

		switch (cmp_expr->cmp_type) {
			case ILANG_GEN_CMP_LT:
				ivm_exec_addInstr(env->cur_exec, LT);
				break;
			case ILANG_GEN_CMP_LE:
				ivm_exec_addInstr(env->cur_exec, LE);
				break;
			case ILANG_GEN_CMP_EQ:
				ivm_exec_addInstr(env->cur_exec, EQ);
				break;
			case ILANG_GEN_CMP_GE:
				ivm_exec_addInstr(env->cur_exec, GE);
				break;
			case ILANG_GEN_CMP_GT:
				ivm_exec_addInstr(env->cur_exec, GT);
				break;
			case ILANG_GEN_CMP_NE:
				ivm_exec_addInstr(env->cur_exec, NE);
				break;
			default:
				IVM_FATAL(GEN_ERR_MSG_UNSUPPORTED_CMP_TYPE(cmp_expr->cmp_type));
		}
	}

	return NORET();
}

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

	GEN_ASSERT_NOT_LEFT_VALUE(expr, "function expression", flag);

	if (!flag.is_top_level) {
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
		ILANG_GEN_PARAM_LIST_EACHPTR_R(params, piter) {
			tmp_param = ILANG_GEN_PARAM_LIST_ITER_GET_PTR(piter);
			tmp_str = ivm_parser_parseStr(tmp_param->val, tmp_param->len);
			ivm_exec_addInstr(exec, SET_ARG, tmp_str);
			MEM_FREE(tmp_str);
		}

		func->body->eval(
			func->body,
			FLAG(0), // function body is not top-level(need last value)
			env
		);

		env->cur_exec = exec_backup;
		ivm_exec_addInstr(env->cur_exec, NEW_FUNC, exec_id);
	}

	return NORET();
}

ilang_gen_value_t
ilang_gen_if_expr_eval(ilang_gen_expr_t *expr,
					   ilang_gen_flag_t flag,
					   ilang_gen_env_t *env)
{
	ilang_gen_if_expr_t *if_expr = IVM_AS(expr, ilang_gen_if_expr_t);
	ivm_size_t main_jmp, prev_elif_jmp = 0;
	ilang_gen_branch_t main_br, last_br, tmp_br;
	ilang_gen_branch_list_t *elifs;
	ilang_gen_branch_list_iterator_t biter;

	GEN_ASSERT_NOT_LEFT_VALUE(expr, "if expression", flag);

	main_br = if_expr->main;
	last_br = if_expr->last;
	elifs = if_expr->elifs;

	main_br.cond->eval(main_br.cond, FLAG(0), env);
	main_jmp = ivm_exec_addInstr(
		env->cur_exec, JUMP_FALSE,
		0 /* replaced with else addr later */
	);
	main_br.body->eval(
		main_br.body,
		FLAG(.is_top_level = flag.is_top_level),
		env
	);

	ivm_exec_setArgAt( // main branch jump to next branch(elif or else)
		env->cur_exec,
		main_jmp,
		ivm_exec_cur(env->cur_exec) - main_jmp
	);

	ILANG_GEN_BRANCH_LIST_EACHPTR_R(elifs, biter) {
		if (prev_elif_jmp) { // has previous elif
			ivm_exec_setArgAt(
				env->cur_exec,
				prev_elif_jmp,
				ivm_exec_cur(env->cur_exec) - prev_elif_jmp // jump to here
			);
		}

		tmp_br = ILANG_GEN_BRANCH_LIST_ITER_GET(biter);
		tmp_br.cond->eval(tmp_br.cond, FLAG(0), env);
		prev_elif_jmp = ivm_exec_addInstr( // jump to next elif/else if false
			env->cur_exec, JUMP_FALSE, 0
		);
		tmp_br.body->eval(
			tmp_br.body,
			FLAG(.is_top_level = flag.is_top_level),
			env
		);
	}

	if (prev_elif_jmp) { // has previous elif
		ivm_exec_setArgAt(
			env->cur_exec,
			prev_elif_jmp,
			ivm_exec_cur(env->cur_exec) - prev_elif_jmp // jump to here
		);
	}

	if (last_br.body) { // has else branch
		last_br.body->eval(
			last_br.body,
			FLAG(.is_top_level = flag.is_top_level),
			env
		);
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
	// GEN_WARN_NO_NESTED_RET(expr, flag)

	// support return only currently
	IVM_ASSERT(intr->sig == ILANG_GEN_INTR_RET,
			   "unsupported interrupt signal");

	if (intr->val) {
		intr->val->eval(intr->val, FLAG(0), env);
	} else {
		ivm_exec_addInstr(env->cur_exec, NEW_NULL);
	}

	ivm_exec_addInstr(env->cur_exec, RETURN);

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
	assign->lhe->eval(
		assign->lhe,
		FLAG(
			.is_left_val = IVM_TRUE,
			.is_top_level = flag.is_top_level
		),
		env
	);

	return NORET();
}

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
		} else {
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
	ilang_gen_env_t env = { str_pool, ret, top_level };

	ivm_exec_unit_registerExec(ret, top_level);

	unit->top_level->eval(
		unit->top_level,
		FLAG(.is_top_level = IVM_TRUE), &env
	);

	return ret;
}

void
ilang_gen_expr_block_destruct(ilang_gen_expr_t *expr)
{
	ilang_gen_expr_list_free(IVM_AS(expr, ilang_gen_expr_block_t)->list);
	return;
}

void
ilang_gen_call_expr_destruct(ilang_gen_expr_t *expr)
{
	ilang_gen_call_expr_t *tmp = IVM_AS(expr, ilang_gen_call_expr_t);

	ilang_gen_expr_free(tmp->callee);
	ilang_gen_expr_list_free(tmp->args);
	
	return;
}

void
ilang_gen_slot_expr_destruct(ilang_gen_expr_t *expr)
{
	ilang_gen_expr_free(IVM_AS(expr, ilang_gen_slot_expr_t)->obj);
	return;
}

void
ilang_gen_unary_expr_destruct(ilang_gen_expr_t *expr)
{
	ilang_gen_expr_free(IVM_AS(expr, ilang_gen_unary_expr_t)->opr);
	return;
}

void
ilang_gen_binary_expr_destruct(ilang_gen_expr_t *expr)
{
	ilang_gen_binary_expr_t *tmp = IVM_AS(expr, ilang_gen_binary_expr_t);
	ilang_gen_expr_free(tmp->op1);
	ilang_gen_expr_free(tmp->op2);
	return;
}

void
ilang_gen_cmp_expr_destruct(ilang_gen_expr_t *expr)
{
	ilang_gen_cmp_expr_t *tmp = IVM_AS(expr, ilang_gen_cmp_expr_t);
	ilang_gen_expr_free(tmp->op1);
	ilang_gen_expr_free(tmp->op2);
	return;
}

void
ilang_gen_fn_expr_destruct(ilang_gen_expr_t *expr)
{
	ilang_gen_fn_expr_t *tmp = IVM_AS(expr, ilang_gen_fn_expr_t);

	ilang_gen_param_list_free(tmp->params);
	ilang_gen_expr_free(tmp->body);

	return;
}

void
ilang_gen_if_expr_destruct(ilang_gen_expr_t *expr)
{
	ilang_gen_if_expr_t *tmp = IVM_AS(expr, ilang_gen_if_expr_t);

	ilang_gen_branch_dump(&tmp->main);
	ilang_gen_branch_list_free(tmp->elifs);
	ilang_gen_branch_dump(&tmp->last);

	return;
}

void
ilang_gen_intr_expr_destruct(ilang_gen_expr_t *expr)
{
	ilang_gen_expr_free(IVM_AS(expr, ilang_gen_intr_expr_t)->val);
	return;
}

void
ilang_gen_assign_expr_destruct(ilang_gen_expr_t *expr)
{
	ilang_gen_assign_expr_t *tmp = IVM_AS(expr, ilang_gen_assign_expr_t);
	ilang_gen_expr_free(tmp->lhe);
	ilang_gen_expr_free(tmp->rhe);
	return;
}
