#include "vm/native/native.h"

#include "priv.h"

ilang_gen_value_t
ilang_gen_id_expr_eval(ilang_gen_expr_t *expr,
					   ilang_gen_flag_t flag,
					   ilang_gen_env_t *env)
{
	ilang_gen_id_expr_t *id_expr = IVM_AS(expr, ilang_gen_id_expr_t);
	ivm_char_t *tmp_str;
	ilang_gen_value_t ret = NORET();
	const ivm_char_t *err;

	tmp_str = ivm_parser_parseStr_heap(
		env->heap,
		id_expr->val.val,
		id_expr->val.len,
		&err
	);

	if (!tmp_str) {
		GEN_ERR_FAILED_PARSE_STRING(expr, err);
	}

#define ID_GEN(name, extra, set_instr, get_instr) \
	if (sizeof(name) == sizeof("")                                     \
		|| !IVM_STRCMP(tmp_str, (name))) {                             \
		extra                                                          \
		if (flag.is_left_val) {                                        \
			set_instr;                                                 \
		} else if (!flag.is_top_level) {                               \
			get_instr;                                                 \
		}                                                              \
	}

	ID_GEN("loc",
		{ ret.is_id_loc = IVM_TRUE; } if (flag.is_slot_expr) { } else,
		// avoid generate code if is the operand of slot expr
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), SET_LOCAL_CONTEXT),
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), GET_LOCAL_CONTEXT))
	else
	ID_GEN("top",
		{ ret.is_id_top = IVM_TRUE; } if (flag.is_slot_expr) { } else,
		// avoid generate code if is the operand of slot expr
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), SET_GLOBAL_CONTEXT),
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), GET_GLOBAL_CONTEXT))
	else
	ID_GEN("", { },
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), SET_CONTEXT_SLOT, tmp_str),
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), GET_CONTEXT_SLOT, tmp_str))

#undef ID_GEN

	return ret;
}

ilang_gen_value_t
ilang_gen_import_expr_eval(ilang_gen_expr_t *expr,
						   ilang_gen_flag_t flag,
						   ilang_gen_env_t *env)
{
	ilang_gen_import_expr_t *import_expr = IVM_AS(expr, ilang_gen_import_expr_t);
	ilang_gen_token_value_list_iterator_t iter;
	ilang_gen_token_value_t *tmp_token;
	ivm_size_t buf_size = ilang_gen_token_value_list_size(import_expr->mod),
			   offset = 0, i, count = buf_size, tmp_ofs;
	const ivm_char_t *err;

	GEN_ASSERT_NOT_LEFT_VALUE(expr, "import expression", flag);

	{
		ILANG_GEN_TOKEN_VALUE_LIST_EACHPTR_R(import_expr->mod, iter) {
			tmp_token = ILANG_GEN_TOKEN_VALUE_LIST_ITER_GET_PTR(iter);
			buf_size += tmp_token->len;
		}
	}

	ivm_char_t buf[buf_size];
	
	{
		ILANG_GEN_TOKEN_VALUE_LIST_EACHPTR_R(import_expr->mod, iter) {
			tmp_token = ILANG_GEN_TOKEN_VALUE_LIST_ITER_GET_PTR(iter);
			tmp_ofs = ivm_parser_parseStr_c(
				buf + offset,
				tmp_token->val,
				tmp_token->len,
				&err
			);

			if (tmp_ofs == -1) {
				GEN_ERR_FAILED_PARSE_STRING(expr, err);
			}

			offset += tmp_ofs;

			buf[offset - 1] = IVM_FILE_SEPARATOR; /* replace '\0' with the file separator */
		}
	}

	if ((offset - 1) >= IVM_MAX_MOD_NAME_LEN) {
		GEN_ERR_TOO_LONG_MOD_NAME(expr, (offset - 1));
	}

	buf[offset - 1] = '\0';

	ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), NEW_STR, buf);
	ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), GET_CONTEXT_SLOT, IVM_NATIVE_IMPORT_FUNC);
	ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), INVOKE, 1);

	/*
		import a.b.c.d
		
		will generate:

			<invoke import>
			assert_context_slot "a"
			assert_slot "b"
			assert_slot "c"
			set_slot "d"
			pop
	 */

	if (flag.is_top_level) {
		{
			i = 0;

			ILANG_GEN_TOKEN_VALUE_LIST_EACHPTR_R(import_expr->mod, iter) {
				tmp_token = ILANG_GEN_TOKEN_VALUE_LIST_ITER_GET_PTR(iter);
				if (ivm_parser_parseStr_c(buf, tmp_token->val, tmp_token->len, &err) == -1) {
					GEN_ERR_FAILED_PARSE_STRING(expr, err);
				}
				
				if (!i) {
					// first
					if (count > 1) {
						ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), ASSERT_LOCAL_SLOT, buf);
					} else {
						ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), SET_LOCAL_SLOT, buf);
					}
				} else if (i + 1 != count) {
					ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), ASSERT_SLOT, buf);
				}

				i++;
			}

			if (count > 1) {
				ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), SET_SLOT, buf);
				ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), POP);
			}
		}
	}

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
	const ivm_char_t *err;

	GEN_ASSERT_NOT_LEFT_VALUE(expr, "table expression", flag);

	list = table_expr->list;
	size = ilang_gen_table_entry_list_size(list);

	/* not top level => no new object */
	if (size) {
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), NEW_OBJ_T, size);
	} else {
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), NEW_OBJ);
	}

	ILANG_GEN_TABLE_ENTRY_LIST_EACHPTR_R(list, eiter) {
		tmp_entry = ILANG_GEN_TABLE_ENTRY_LIST_ITER_GET(eiter);

		tmp_entry.expr->eval(
			tmp_entry.expr,
			FLAG(0),
			env
		);

		if (tmp_entry.oop != -1) {
			ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), SET_OOP_B, tmp_entry.oop);
		} else if (tmp_entry.index) {
			ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), DUP_N, 1);
			ilang_gen_index_expr_genArg(expr, tmp_entry.index, env);
			ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), IDXA);
			ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), POP);
		} else {
			tmp_str = ivm_parser_parseStr_heap(
				env->heap,
				tmp_entry.name.val,
				tmp_entry.name.len,
				&err
			);

			if (!tmp_str) {
				GEN_ERR_FAILED_PARSE_STRING(expr, err);
			}

			if (!IVM_STRCMP(tmp_str, "proto")) {
				ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), DUP_N, 1);
				ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), SET_PROTO);
				ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), POP);
			} else {
				ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), SET_SLOT_B, tmp_str);
			}
		}
	}

	if (flag.is_top_level) {
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), POP);
	}

	return NORET();
}

ilang_gen_value_t
ilang_gen_varg_expr_eval(ilang_gen_expr_t *expr,
						 ilang_gen_flag_t flag,
						 ilang_gen_env_t *env)
{
	ilang_gen_varg_expr_t *varg = IVM_AS(expr, ilang_gen_varg_expr_t);

	GEN_ASSERT_ONLY_LEFT_VAL(expr, flag, "varg expression");
	GEN_ASSERT_ONLY_LIST(expr, flag, "varg expression");

	ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), NEW_VARG, flag.varg_offset - 1);
	// IVM_TRACE("%d\n", flag.varg_offset);
	varg->bondee->eval(varg->bondee, FLAG(.is_left_val = IVM_TRUE), env);

	return NORET();
}

ilang_gen_value_t
ilang_gen_list_expr_eval(ilang_gen_expr_t *expr,
						 ilang_gen_flag_t flag,
						 ilang_gen_env_t *env)
{
	ilang_gen_list_expr_t *list_expr = IVM_AS(expr, ilang_gen_list_expr_t);
	ilang_gen_expr_list_t *elems = list_expr->elems;
	ilang_gen_expr_list_iterator_t eiter;
	ilang_gen_expr_t *tmp_elem;
	ivm_bool_t has_varg = IVM_FALSE;
	ivm_size_t size, vofs;

	// GEN_ASSERT_NOT_LEFT_VALUE(expr, "list expression", flag);

	if (flag.is_left_val) {
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), TO_LIST);

		size = ilang_gen_expr_list_size(elems);

		{
			ILANG_GEN_EXPR_LIST_EACHPTR_R(elems, eiter) {
				tmp_elem = ILANG_GEN_EXPR_LIST_ITER_GET(eiter);
				if (ilang_gen_expr_isExpr(tmp_elem, varg_expr)) {
					if (has_varg) {
						GEN_ERR_MULTIPLE_VARG(expr)
					}
					
					has_varg = IVM_TRUE;
				}
			}
		}

		if (has_varg) {
			ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), PUSH_BLOCK_S1);
			ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), UNPACK_LIST_ALL);
		} else {
			ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), UNPACK_LIST, size);
		}

		vofs = size;
		{
			ILANG_GEN_EXPR_LIST_EACHPTR_R(elems, eiter) {
				tmp_elem = ILANG_GEN_EXPR_LIST_ITER_GET(eiter);
				if (has_varg) {
					// because of the unpack_list_all instr, we have to ensure that the stack is not empty
					ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), ENSURE_NONE);
					tmp_elem->eval(tmp_elem, FLAG(.is_left_val = IVM_TRUE, .varg_offset = vofs), env);
				} else {
					tmp_elem->eval(tmp_elem, FLAG(.is_left_val = IVM_TRUE, .varg_offset = vofs), env);
				}
				vofs--;
			}
		}

		if (has_varg) {
			ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), POP_BLOCK);
		}
	} else {
		ILANG_GEN_EXPR_LIST_EACHPTR_R(elems, eiter) {
			tmp_elem = ILANG_GEN_EXPR_LIST_ITER_GET(eiter);
			tmp_elem->eval(tmp_elem, FLAG(0), env);
		}

		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), NEW_LIST, ilang_gen_expr_list_size(elems));
		
		if (flag.is_top_level) {
			ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), POP);
		}
	}

	return NORET();
}

ilang_gen_value_t
ilang_gen_list_comp_core_expr_eval(ilang_gen_expr_t *expr,
								   ilang_gen_flag_t flag,
								   ilang_gen_env_t *env)
{
	ilang_gen_list_comp_core_expr_t *core_expr = IVM_AS(expr, ilang_gen_list_comp_core_expr_t);

	// IVM_TRACE("block %d %p\n", core_expr->cblock, expr);

	core_expr->core->eval(core_expr->core, FLAG(0), env);
	ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), DUP_PREV_BLOCK, core_expr->cblock);
	ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), PUSH_LIST);

	return NORET();
}

ilang_gen_value_t
ilang_gen_list_comp_expr_eval(ilang_gen_expr_t *expr,
							  ilang_gen_flag_t flag,
							  ilang_gen_env_t *env)
{
	ilang_gen_list_comp_expr_t *comp_expr = IVM_AS(expr, ilang_gen_list_comp_expr_t);

	GEN_ASSERT_NOT_LEFT_VALUE(expr, "list comprehension expression", flag);

	ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), NEW_LIST, 0);
	comp_expr->expr->eval(comp_expr->expr, FLAG(.is_top_level = IVM_TRUE), env);

	if (flag.is_top_level) {
		ivm_exec_addInstr_l(env->cur_exec, GET_LINE(expr), POP);
	}

	return NORET();
}
