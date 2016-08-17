#include "priv.h"

ilang_gen_value_t
ilang_gen_unary_expr_eval(ilang_gen_expr_t *expr,
						  ilang_gen_flag_t flag,
						  ilang_gen_env_t *env)
{
	ilang_gen_unary_expr_t *unary_expr = IVM_AS(expr, ilang_gen_unary_expr_t);

	GEN_ASSERT_NOT_LEFT_VALUE(expr, "unary expression", flag);

	if (unary_expr->type == IVM_UNIOP_ID(DEL)) {
		ivm_exec_addInstr(env->cur_exec, NEW_NIL);
		
		unary_expr->opr->eval(
			unary_expr->opr,
			FLAG(.is_left_val = IVM_TRUE),
			env
		);

		if (!flag.is_top_level) {
			ivm_exec_addInstr(env->cur_exec, NEW_NULL);
		}
	} else {
		unary_expr->opr->eval(
			unary_expr->opr,
			FLAG(0),
			env
		);

		switch (unary_expr->type) {
			case IVM_UNIOP_ID(NOT):
				ivm_exec_addInstr(env->cur_exec, NOT);
				break;
			case IVM_UNIOP_ID(NEG):
				ivm_exec_addInstr(env->cur_exec, NEG);
				break;
			case IVM_UNIOP_ID(POS):
				ivm_exec_addInstr(env->cur_exec, POS);
				break;
			case IVM_UNIOP_ID(CLONE):
				ivm_exec_addInstr(env->cur_exec, CLONE);
				break;
			default:
				IVM_FATAL(GEN_ERR_MSG_UNSUPPORTED_UNARY_OP(unary_expr->type));
		}

		if (flag.is_top_level) {
			ivm_exec_addInstr(env->cur_exec, POP);
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

	if (binary_expr->type != IVM_BINOP_ID(IDX)) {
		GEN_ASSERT_NOT_LEFT_VALUE(expr, "binary expression(except index expression)", flag);
	}

	op1 = binary_expr->op1;
	op2 = binary_expr->op2;

	op1->eval(op1, FLAG(0), env);
	op2->eval(op2, FLAG(0), env);

#define BR(op) \
	case IVM_BINOP_ID(op):                      \
		ivm_exec_addInstr(env->cur_exec, op);   \
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
		case IVM_BINOP_ID(IDX):
			if (flag.is_left_val) {
				ivm_exec_addInstr(env->cur_exec, IDX_ASSIGN);
				ivm_exec_addInstr(env->cur_exec, POP);
			} else {
				ivm_exec_addInstr(env->cur_exec, IDX);
			}
			break;
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

	op1->eval(op1, FLAG(0), env);
	op2->eval(op2, FLAG(0), env);

	if (flag.is_top_level) {
		return NORET();
	}

#define BR(op) \
	case ILANG_GEN_CMP_##op:                                \
		if (flag.if_use_cond_reg && 0) {                    \
			/* ivm_exec_addInstr(env->cur_exec, op##_R); */ \
			return RETVAL(.use_cond_reg = IVM_TRUE);        \
		} else {                                            \
			ivm_exec_addInstr(env->cur_exec, op);           \
		}                                                   \
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

	if (flag.is_top_level) {
		ivm_exec_addInstr(env->cur_exec, POP);
	}

	return NORET();
}
