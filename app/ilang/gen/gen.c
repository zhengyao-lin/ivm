#include <setjmp.h>

#include "util/opt.h"

#include "priv.h"

void
ilang_gen_env_init(ilang_gen_env_t *env,
				   const ivm_char_t *file,
				   ivm_string_pool_t *str_pool,
				   ivm_exec_unit_t *unit,
				   ivm_exec_t *cur_exec)
{
	*env = (ilang_gen_env_t) {
		.file = file,

		.str_pool = str_pool,
		.unit = unit,
		.cur_exec = cur_exec,

		.addr = ilang_gen_addr_set_init(),

		.list_log = ivm_ptlist_new(),
		.heap = ivm_heap_new(IVM_DEFAULT_INIT_HEAP_SIZE)
	};

	if (setjmp(env->err_handle)) {
		IVM_FATAL("impossible");
	}

	return;
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
	ivm_bool_t has_ret = IVM_FALSE;

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
			has_ret = IVM_TRUE;
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

	if (!has_ret && !flag.is_top_level) {
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), NEW_NONE);
	}

	return NORET();
}

ivm_exec_unit_t *
ilang_gen_generateExecUnit_c(ilang_gen_trans_unit_t *unit,
							 ivm_size_t offset)
{
	ivm_string_pool_t *str_pool = ivm_string_pool_new(IVM_TRUE);
	ivm_exec_unit_t *ret = ivm_exec_unit_new(0, ivm_exec_list_new());
	ivm_exec_t *top_level = ivm_exec_new(str_pool);

	ivm_exec_unit_setOffset(ret, offset);
	ivm_exec_unit_setSourcePos(ret, ivm_source_pos_new(unit->file));

	ilang_gen_env_t env;

	ivm_exec_unit_registerExec(ret, top_level);

	ilang_gen_env_init(&env, unit->file, str_pool, ret, top_level);

	if (setjmp(env.err_handle)) {
		ilang_gen_env_dump(&env);
		ivm_exec_unit_free(ret);
		return IVM_NULL;
	}

	unit->top_level->eval(
		unit->top_level,
		FLAG(0), &env
	);

	ivm_opt_optExec(top_level);

	ilang_gen_env_dump(&env);

	return ret;
}
