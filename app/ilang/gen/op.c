#include "util/opt.h"

#include "priv.h"

IVM_INLINE
ilang_gen_value_t
_ilang_gen_ref_expr_eval(ilang_gen_unary_expr_t *unary_expr,
						 ilang_gen_flag_t flag,
						 ilang_gen_env_t *env)
{
	ivm_exec_t *exec, *exec_backup;
	ivm_size_t addr, exec_id;
	ivm_size_t sp_back;

	ilang_gen_addr_set_t addr_backup = env->addr;
	sp_back = env->sp;
	env->sp = 0;

	exec = ivm_exec_new(env->str_pool);
	exec_id = ivm_exec_unit_registerExec(env->unit, exec);
	exec_backup = env->cur_exec;
	env->cur_exec = exec;

	env->addr = ilang_gen_addr_set_init();

	ivm_exec_addInstr_l(env->cur_exec, GET_LINE(unary_expr), REMOVE_LOC);

	addr = ivm_exec_addInstr_l(env->cur_exec, GET_LINE(unary_expr), CHECK, 0);

	unary_expr->opr->eval(
		unary_expr->opr,
		FLAG(0),
		env
	);

	ivm_exec_setArgAt(env->cur_exec, addr, ivm_exec_cur(env->cur_exec) - addr);

	ivm_exec_addInstr_l(env->cur_exec, GET_LINE(unary_expr), DUP);
	unary_expr->opr->eval(
		unary_expr->opr,
		FLAG(.is_left_val = IVM_TRUE),
		env
	);

	ivm_exec_addInstr_l(env->cur_exec, GET_LINE(unary_expr), RETURN);

	env->cur_exec = exec_backup;
	ivm_opt_optExec(exec);

	env->addr = addr_backup;
	env->sp = sp_back;

	if (!flag.is_top_level) {
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(unary_expr), NEW_FUNC, exec_id);
	}

	return NORET();
}

ilang_gen_value_t
ilang_gen_unary_expr_eval(ilang_gen_expr_t *expr,
						  ilang_gen_flag_t flag,
						  ilang_gen_env_t *env)
{
	ilang_gen_unary_expr_t *unary_expr = IVM_AS(expr, ilang_gen_unary_expr_t);

	if (unary_expr->type == ILANG_GEN_UNIOP_DEREF) {
		/* deref: call the operand with or without argument for assign or eval */
		if (flag.is_left_val) {
			unary_expr->opr->eval(
				unary_expr->opr,
				FLAG(0),
				env
			);
			ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), INVOKE, 1);
			ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), POP);
		} else {
			unary_expr->opr->eval(
				unary_expr->opr,
				FLAG(0),
				env
			);
			ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), INVOKE, 0);
			if (flag.is_top_level) {
				ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), POP);
			}
		}
	} else {
		GEN_ASSERT_NOT_LEFT_VALUE(expr, "unary expression", flag);

		if (unary_expr->type == IVM_UNIOP_ID(DEL)) {
			ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), NEW_NIL);
			
			unary_expr->opr->eval(
				unary_expr->opr,
				FLAG(.is_left_val = IVM_TRUE),
				env
			);

			if (!flag.is_top_level) {
				ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), NEW_NONE);
			}
		} else if (unary_expr->type == ILANG_GEN_UNIOP_REF) {
			_ilang_gen_ref_expr_eval(unary_expr, flag, env);
		} else {
			unary_expr->opr->eval(
				unary_expr->opr,
				FLAG(0),
				env
			);

			switch (unary_expr->type) {
				case IVM_UNIOP_ID(NOT):
					ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), NOT);
					break;
				case IVM_UNIOP_ID(NEG):
					ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), NEG);
					break;
				case IVM_UNIOP_ID(POS):
					ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), POS);
					break;
				/* case IVM_UNIOP_ID(CLONE):
					ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), CLONE);
					break; */
				default:
					IVM_FATAL(GEN_ERR_MSG_UNSUPPORTED_UNARY_OP(unary_expr->type));
			}

			if (flag.is_top_level) {
				ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), POP);
			}
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
	ilang_gen_expr_t *op1, *op2;

	GEN_ASSERT_NOT_LEFT_VALUE(expr, "binary expression(except index expression)", flag);

	op1 = binary_expr->op1;
	op2 = binary_expr->op2;

	op1->eval(op1, FLAG(0), env);
	INC_SP();
	op2->eval(op2, FLAG(0), env);
	DEC_SP();

#define BR(op) \
	case IVM_BINOP_ID(op):                                        \
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), op);   \
		break;

	switch (binary_expr->type) {
		BR(ADD)
		BR(SUB)
		BR(MUL)
		BR(DIV)
		BR(MOD)
		BR(AND)
		BR(EOR)
		BR(IOR)
		BR(SHL)
		BR(SHAR)
		BR(SHLR)
		default:
			IVM_FATAL(GEN_ERR_MSG_UNSUPPORTED_BINARY_OP(binary_expr->type));
	}

#undef BR

	if (flag.is_top_level) {
		ivm_exec_addInstr(env->cur_exec, POP);
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

	if (cmp_expr->cmp_type != ILANG_GEN_CMP_IS) {
		op1->eval(op1, FLAG(0), env);
		INC_SP();
		op2->eval(op2, FLAG(0), env);
		DEC_SP();
	} else {
		op2->eval(op2, FLAG(0), env);
		INC_SP();
		op1->eval(op1, FLAG(0), env);
		DEC_SP();
	}

#if 0
#define BR(op) \
	case ILANG_GEN_CMP_##op:                                \
		if (flag.if_use_cond_reg && 0) {                    \
			/* ivm_exec_addInstr(env->cur_exec, op##_R); */ \
			return RETVAL(.use_cond_reg = IVM_TRUE);        \
		} else {                                            \
			ivm_exec_addInstr(env->cur_exec, op);           \
		}                                                   \
		break;
#endif

#define BR(op) \
	case ILANG_GEN_CMP_##op:                                      \
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), op);   \
		break;

	switch (cmp_expr->cmp_type) {
		BR(LT)
		BR(LE)
		BR(EQ)
		BR(GE)
		BR(GT)
		BR(NE)

		case ILANG_GEN_CMP_IS:
			ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), GET_CONTEXT_SLOT, "is");
			ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), INVOKE, 2);
			break;

		default:
			IVM_FATAL(GEN_ERR_MSG_UNSUPPORTED_CMP_TYPE(cmp_expr->cmp_type));
	}

#undef BR

	if (flag.is_top_level) {
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), POP);
	}

	return NORET();
}

ilang_gen_value_t
ilang_gen_index_expr_eval(ilang_gen_expr_t *expr,
						  ilang_gen_flag_t flag,
						  ilang_gen_env_t *env)
{
	ilang_gen_index_expr_t *index_expr = IVM_AS(expr, ilang_gen_index_expr_t);
	ilang_gen_expr_list_iterator_t eiter;
	ilang_gen_expr_t *tmp_expr;
	ivm_size_t count, sp_back;

	index_expr->op->eval(index_expr->op, FLAG(0), env);

	/*
		count		index obj
		0			none
		1			arg1
		>1			[arg1, arg2, ...]
	 */
	switch (count = ilang_gen_expr_list_size(index_expr->idx)) {
		case 0:
			ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), NEW_NONE);
			break;
		
		case 1:
			tmp_expr = ilang_gen_expr_list_at(index_expr->idx, 0);
			INC_SP();
			tmp_expr->eval(tmp_expr, FLAG(0), env);
			DEC_SP();
			break;

		default: {
			sp_back = env->sp;
			ILANG_GEN_EXPR_LIST_EACHPTR_R(index_expr->idx, eiter) {
				tmp_expr = ILANG_GEN_EXPR_LIST_ITER_GET(eiter);\
				INC_SP();
				tmp_expr->eval(tmp_expr, FLAG(0), env);
			}
			env->sp = sp_back;

			ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), NEW_LIST, count);
		}
	}

	if (flag.is_left_val) {
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), IDX_ASSIGN);
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), POP);
	} else {
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), IDX);
		if (flag.is_top_level) {
			ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), POP);
		}
	}

	return NORET();
}
