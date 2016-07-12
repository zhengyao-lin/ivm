#include "pub/type.h"
#include "pub/com.h"
#include "pub/vm.h"
#include "pub/err.h"

#include "util/parser.h"

#include "gen.h"

#define FLAG ilang_gen_flag_build
#define CHECK_SE() ((ilang_gen_check_flag_t) { .has_side_effect = IVM_TRUE })
#define RETVAL ilang_gen_value_build
#define NORET() ((ilang_gen_value_t) { 0, 0 })

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
#define GEN_ERR_MSG_BREAK_OR_CONT_OUTSIDE_LOOP						"using break/cont outside a loop"
#define GEN_ERR_MSG_BREAK_OR_CONT_IGNORE_ARG						"ignore break/cont argument"

#define GEN_ERR_GENERAL(expr, ...) \
	GEN_ERR((expr)->pos, __VA_ARGS__)

#define GEN_WARN_GENERAL(expr, ...) \
	GEN_WARN((expr)->pos, __VA_ARGS__)

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

#define ID_GEN(name, set_instr, get_instr) \
	if (sizeof(name) == sizeof("")                                     \
		|| !IVM_STRCMP(tmp_str, (name))) {                             \
		if (flag.is_left_val) {                                        \
			if (!flag.is_top_level) {                                  \
				ivm_exec_addInstr(env->cur_exec, DUP);                 \
			}                                                          \
			set_instr;                                                 \
		} else if (!flag.is_top_level) {                               \
			get_instr;                                                 \
		}                                                              \
	}

	ID_GEN("let",
		ivm_exec_addInstr(env->cur_exec, SET_LOCAL_CONTEXT),
		ivm_exec_addInstr(env->cur_exec, GET_LOCAL_CONTEXT))
	else
	ID_GEN("top",
		ivm_exec_addInstr(env->cur_exec, SET_GLOBAL_CONTEXT),
		ivm_exec_addInstr(env->cur_exec, GET_GLOBAL_CONTEXT))
	else
	ID_GEN("",
		ivm_exec_addInstr(env->cur_exec, SET_CONTEXT_SLOT, tmp_str),
		ivm_exec_addInstr(env->cur_exec, GET_CONTEXT_SLOT, tmp_str))

#undef ID_GEN

	MEM_FREE(tmp_str);

	return NORET();
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

	if (flag.is_top_level &&
		!expr->check(expr, CHECK_SE())) {
		return NORET();
	}

	list = table_expr->list;
	size = ilang_gen_table_entry_list_size(list);

	/* not top level => no new object */
	if (!flag.is_top_level) {
		if (size) {
			ivm_exec_addInstr(env->cur_exec, NEW_OBJ_T, size);
		} else {
			ivm_exec_addInstr(env->cur_exec, NEW_OBJ);
		}
	}

	ILANG_GEN_TABLE_ENTRY_LIST_EACHPTR_R(list, eiter) {
		tmp_entry = ILANG_GEN_TABLE_ENTRY_LIST_ITER_GET(eiter);

		if (!flag.is_top_level ||
			tmp_entry.expr->check(tmp_entry.expr, CHECK_SE())) {
			// not top level and no side effect => skip
			tmp_entry.expr->eval(
				tmp_entry.expr,
				FLAG(.is_top_level = flag.is_top_level),
				env
			);

			if (!flag.is_top_level) {
				tmp_str = ivm_parser_parseStr(
					tmp_entry.name.val,
					tmp_entry.name.len
				);

				ivm_exec_addInstr(env->cur_exec, SET_SLOT_B, tmp_str);

				MEM_FREE(tmp_str);
			}
		}
	}

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
		ivm_exec_addInstr(
			env->cur_exec, INVOKE_BASE,
			ilang_gen_expr_list_size(args)
		);
	} else {
		ivm_exec_addInstr(
			env->cur_exec, INVOKE,
			ilang_gen_expr_list_size(args)
		);
	}

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
	ilang_gen_value_t ret = NORET();
	ivm_bool_t is_proto;

	tmp_str = ivm_parser_parseStr(
		slot_expr->slot.val,
		slot_expr->slot.len
	);

	is_proto = !IVM_STRCMP(tmp_str, "proto");

	if (flag.is_left_val) {
		slot_expr->obj->eval(
			slot_expr->obj,
			FLAG(0),
			env
		);
		
		if (is_proto) {
			ivm_exec_addInstr(env->cur_exec, SET_PROTO);
		} else {
			ivm_exec_addInstr(env->cur_exec, SET_SLOT, tmp_str);
		}

		ivm_exec_addInstr(env->cur_exec, POP);
	} else {
		if (flag.is_top_level &&
			!expr->check(expr, CHECK_SE())) {
			goto END;
		}

		slot_expr->obj->eval(
			slot_expr->obj,
			FLAG(.is_top_level = flag.is_top_level),
			// is_top_level == true => has side effect => no need to get slot
			env
		);

		if (!flag.is_top_level) {
			if (flag.is_callee) {
				// leave base object on the stack
				if (is_proto) {
					ivm_exec_addInstr(env->cur_exec, DUP);
					ivm_exec_addInstr(env->cur_exec, GET_PROTO);
				} else {
					ivm_exec_addInstr(env->cur_exec, GET_SLOT_N, tmp_str);
				}

				ret = RETVAL(.has_base = IVM_TRUE);
			} else {
				if (is_proto) {
					ivm_exec_addInstr(env->cur_exec, GET_PROTO);
				} else {
					ivm_exec_addInstr(env->cur_exec, GET_SLOT, tmp_str);
				}
			}
		}
	} // else neither left value nor top level: don't generate

END:
	MEM_FREE(tmp_str);

	return ret;
}

ilang_gen_value_t
ilang_gen_unary_expr_eval(ilang_gen_expr_t *expr,
						  ilang_gen_flag_t flag,
						  ilang_gen_env_t *env)
{
	ilang_gen_unary_expr_t *unary_expr = IVM_AS(expr, ilang_gen_unary_expr_t);

	GEN_ASSERT_NOT_LEFT_VALUE(expr, "unary expression", flag);

	if (flag.is_top_level &&
		!expr->check(expr, CHECK_SE())) {
		return NORET();
	}

	unary_expr->opr->eval(
		unary_expr->opr,
		FLAG(.is_top_level = flag.is_top_level),
		env
	);

	if (flag.is_top_level) {
		return NORET();
	}

	switch (unary_expr->type) {
		case IVM_UNIOP_ID(NOT):
			ivm_exec_addInstr(env->cur_exec, NOT);
			break;
		case IVM_UNIOP_ID(CLONE):
			ivm_exec_addInstr(env->cur_exec, CLONE);
			break;
		default:
			IVM_FATAL(GEN_ERR_MSG_UNSUPPORTED_UNARY_OP(unary_expr->type));
	}

	return NORET();
}

ilang_gen_value_t
ilang_gen_binary_expr_eval(ilang_gen_expr_t *expr,
						   ilang_gen_flag_t flag,
						   ilang_gen_env_t *env)
{
	ilang_gen_binary_expr_t *binary_expr = IVM_AS(expr, ilang_gen_binary_expr_t);
	ilang_gen_expr_t *op1, *op2;

	GEN_ASSERT_NOT_LEFT_VALUE(expr, "binary expression", flag);

	op1 = binary_expr->op1;
	op2 = binary_expr->op2;

	if (flag.is_top_level &&
		!expr->check(expr, CHECK_SE())) {
		/* is top level and has no side effect */
		return NORET();
	}

	op1->eval(op1, FLAG(.is_top_level = flag.is_top_level), env);
	op2->eval(op2, FLAG(.is_top_level = flag.is_top_level), env);

	if (flag.is_top_level) {
		return NORET();
	}

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

	return NORET();
}

ilang_gen_value_t
ilang_gen_cmp_expr_eval(ilang_gen_expr_t *expr,
						ilang_gen_flag_t flag,
						ilang_gen_env_t *env)
{
	ilang_gen_cmp_expr_t *cmp_expr = IVM_AS(expr, ilang_gen_cmp_expr_t);
	ilang_gen_expr_t *op1, *op2;

	GEN_ASSERT_NOT_LEFT_VALUE(expr, "compare expression", flag);

	op1 = cmp_expr->op1;
	op2 = cmp_expr->op2;

	if (flag.is_top_level &&
		!expr->check(expr, CHECK_SE())) {
		/* is top level and has no side effect */
		return NORET();
	}

	op1->eval(op1, FLAG(.is_top_level = flag.is_top_level), env);
	op2->eval(op2, FLAG(.is_top_level = flag.is_top_level), env);

	if (flag.is_top_level) {
		return NORET();
	}

#define BR(op) \
	case ILANG_GEN_CMP_##op:                             \
		if (flag.if_use_cond_reg) {                      \
			ivm_exec_addInstr(env->cur_exec, op##_R);    \
			return RETVAL(.use_cond_reg = IVM_TRUE);     \
		} else {                                         \
			ivm_exec_addInstr(env->cur_exec, op);        \
		}                                                \
		break;

	switch (cmp_expr->cmp_type) {
		BR(LT)
		BR(LE)
		BR(EQ)
		BR(GE)
		BR(GT)
		BR(NE)
		default:
			IVM_FATAL(GEN_ERR_MSG_UNSUPPORTED_CMP_TYPE(cmp_expr->cmp_type));
	}

#undef BR

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
	ivm_size_t main_end_jmp;
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
	main_end_jmp = ivm_exec_addInstr(env->cur_exec, JUMP, 0);

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

		*cur_end_jmp++ = ivm_exec_addInstr(env->cur_exec, JUMP, 0);
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
	ivm_exec_setArgAt(
		env->cur_exec,
		main_end_jmp,
		ivm_exec_cur(env->cur_exec) - main_end_jmp
	);

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

ilang_gen_value_t
ilang_gen_intr_expr_eval(ilang_gen_expr_t *expr,
						 ilang_gen_flag_t flag,
						 ilang_gen_env_t *env)
{
	ilang_gen_intr_expr_t *intr = IVM_AS(expr, ilang_gen_intr_expr_t);

	GEN_ASSERT_NOT_LEFT_VALUE(expr, "interrupt expression", flag);
	// GEN_WARN_NO_NESTED_RET(expr, flag)

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
				ivm_list_push(env->break_ref, &cur);
			} else {
				GEN_ERR_GENERAL(expr, GEN_ERR_MSG_BREAK_OR_CONT_OUTSIDE_LOOP);
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

	if (flag.is_top_level &&
		!expr->check(expr, CHECK_SE())) {
		return NORET();
	}

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
		} else if (!flag.is_top_level ||
				   tmp_expr->check(tmp_expr, CHECK_SE())) {
			// is top level or has side effect => generate
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
	ilang_gen_env_t env = { str_pool, ret, top_level, -1, IVM_NULL };

	ivm_exec_unit_registerExec(ret, top_level);

	unit->top_level->eval(
		unit->top_level,
		FLAG(.is_top_level = IVM_TRUE), &env
	);

	return ret;
}

ivm_bool_t
ilang_gen_expr_block_check(ilang_gen_expr_t *expr,
						   ilang_gen_check_flag_t flag)
{
	ilang_gen_expr_block_t *block = IVM_AS(expr, ilang_gen_expr_block_t);
	ilang_gen_expr_list_t *list = block->list;
	ilang_gen_expr_list_iterator_t eiter;
	ilang_gen_expr_t *tmp_expr;

	ILANG_GEN_EXPR_LIST_EACHPTR_R(list, eiter) {
		tmp_expr = ILANG_GEN_EXPR_LIST_ITER_GET(eiter);
		if (tmp_expr->check(tmp_expr, flag)) {
			return IVM_TRUE;
		}
	}

	return IVM_FALSE;
}

ivm_bool_t
ilang_gen_int_expr_check(ilang_gen_expr_t *expr,
						 ilang_gen_check_flag_t flag)
{
	return IVM_FALSE;
}

ivm_bool_t
ilang_gen_float_expr_check(ilang_gen_expr_t *expr,
						   ilang_gen_check_flag_t flag)
{
	return IVM_FALSE;
}

ivm_bool_t
ilang_gen_string_expr_check(ilang_gen_expr_t *expr,
							ilang_gen_check_flag_t flag)
{
	return IVM_FALSE;
}

ivm_bool_t
ilang_gen_id_expr_check(ilang_gen_expr_t *expr,
						ilang_gen_check_flag_t flag)
{
	return IVM_FALSE;
}

ivm_bool_t
ilang_gen_table_expr_check(ilang_gen_expr_t *expr,
						   ilang_gen_check_flag_t flag)
{
	ilang_gen_table_expr_t *table_expr = IVM_AS(expr, ilang_gen_table_expr_t);
	ilang_gen_table_entry_t tmp_entry;
	ilang_gen_table_entry_list_t *list = table_expr->list;
	ilang_gen_table_entry_list_iterator_t eiter;

	ILANG_GEN_TABLE_ENTRY_LIST_EACHPTR_R(list, eiter) {
		tmp_entry = ILANG_GEN_TABLE_ENTRY_LIST_ITER_GET(eiter);
		if (tmp_entry.expr->check(tmp_entry.expr, flag)) {
			return IVM_TRUE;
		}
	}

	return IVM_FALSE;
}

ivm_bool_t
ilang_gen_call_expr_check(ilang_gen_expr_t *expr,
						  ilang_gen_check_flag_t flag)
{
	/*
	ilang_gen_call_expr_t *call_expr = IVM_AS(expr, ilang_gen_call_expr_t);
	ilang_gen_expr_list_t *args = call_expr->args;
	ilang_gen_expr_list_iterator_t aiter;
	ilang_gen_expr_t *tmp_arg;

	ILANG_GEN_EXPR_LIST_EACHPTR(args, aiter) {
		tmp_arg = ILANG_GEN_EXPR_LIST_ITER_GET(aiter);
		if (tmp_arg->check(tmp_arg, flag)) {
			return IVM_TRUE;
		}
	}
	*/

	return IVM_TRUE;
}

ivm_bool_t
ilang_gen_slot_expr_check(ilang_gen_expr_t *expr,
						  ilang_gen_check_flag_t flag)
{
	ilang_gen_slot_expr_t *slot_expr = IVM_AS(expr, ilang_gen_slot_expr_t);

	return slot_expr->obj->check(slot_expr->obj, flag);
}

ivm_bool_t
ilang_gen_unary_expr_check(ilang_gen_expr_t *expr,
						   ilang_gen_check_flag_t flag)
{
	ilang_gen_unary_expr_t *unary_expr = IVM_AS(expr, ilang_gen_unary_expr_t);

	return unary_expr->opr->check(unary_expr->opr, flag);
}

ivm_bool_t
ilang_gen_binary_expr_check(ilang_gen_expr_t *expr,
							ilang_gen_check_flag_t flag)
{
	ilang_gen_binary_expr_t *binary_expr = IVM_AS(expr, ilang_gen_binary_expr_t);

	return
	binary_expr->op1->check(binary_expr->op1, flag) ||
	binary_expr->op2->check(binary_expr->op2, flag);
}

ivm_bool_t
ilang_gen_cmp_expr_check(ilang_gen_expr_t *expr,
						 ilang_gen_check_flag_t flag)
{
	ilang_gen_cmp_expr_t *cmp_expr = IVM_AS(expr, ilang_gen_cmp_expr_t);

	return
	cmp_expr->op1->check(cmp_expr->op1, flag) ||
	cmp_expr->op2->check(cmp_expr->op2, flag);
}

ivm_bool_t
ilang_gen_fn_expr_check(ilang_gen_expr_t *expr, ilang_gen_check_flag_t flag)
{
	return IVM_FALSE;
}

ivm_bool_t
ilang_gen_if_expr_check(ilang_gen_expr_t *expr, ilang_gen_check_flag_t flag)
{
	ilang_gen_if_expr_t *if_expr = IVM_AS(expr, ilang_gen_if_expr_t);
	ilang_gen_branch_t main_br = if_expr->main,
					   last_br = if_expr->last,
					   tmp_br;
	ilang_gen_branch_list_t *elifs = if_expr->elifs;
	ilang_gen_branch_list_iterator_t eiter;

	if (main_br.cond->check(main_br.cond, flag)) {
		return IVM_TRUE;
	}

	if (main_br.body->check(main_br.body, flag)) {
		return IVM_TRUE;
	}

	ILANG_GEN_BRANCH_LIST_EACHPTR(elifs, eiter) {
		tmp_br = ILANG_GEN_BRANCH_LIST_ITER_GET(eiter);

		if (tmp_br.cond->check(tmp_br.cond, flag)) {
			return IVM_TRUE;
		}

		if (tmp_br.body->check(tmp_br.body, flag)) {
			return IVM_TRUE;
		}
	}

	if (last_br.body && last_br.body->check(last_br.body, flag)) {
		return IVM_TRUE;
	}

	return IVM_FALSE;
}

ivm_bool_t
ilang_gen_while_expr_check(ilang_gen_expr_t *expr, ilang_gen_check_flag_t flag)
{
	ilang_gen_while_expr_t *while_expr = IVM_AS(expr, ilang_gen_while_expr_t);

	if (while_expr->cond->check(while_expr->cond, flag)) {
		return IVM_TRUE;
	}

	if (while_expr->body->check(while_expr->body, flag)) {
		return IVM_TRUE;
	}

	return IVM_FALSE;
}

ivm_bool_t
ilang_gen_intr_expr_check(ilang_gen_expr_t *expr, ilang_gen_check_flag_t flag)
{
	return IVM_TRUE;
}

ivm_bool_t
ilang_gen_assign_expr_check(ilang_gen_expr_t *expr, ilang_gen_check_flag_t flag)
{
	return IVM_TRUE;
}
