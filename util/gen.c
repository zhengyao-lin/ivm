#include "pub/const.h"
#include "pub/mem.h"
#include "pub/vm.h"
#include "pub/inlines.h"

#include "std/string.h"
#include "std/list.h"
#include "std/ref.h"

#include "parser.h"
#include "gen.h"

#include "vm/dbg.h"

#define GEN_ERR(p, ...) \
	IVM_TRACE("generator: at line %zd pos %zd: ", (p).line, (p).pos); \
	IVM_TRACE(__VA_ARGS__); \
	IVM_TRACE("\n");

#define GEN_ERR_MSG_UNKNOWN_OPCODE(instr, len)							"unknown opcode '%.*s'", (int)(len), (instr)
#define GEN_ERR_MSG_UNMATCHED_ARGUMENT(instr, len, arg, exp)			"unmatched argument type for instruction '%.*s'(expecting %c, %c given)", (int)(len), (instr), (exp), (arg)
#define GEN_ERR_MSG_UNKNOWN_ARGUMENT_TYPE(t)							"unknown argument type '%c'", (t)
#define GEN_ERR_MSG_UNDEFINED_BLOCK(label, len)							"undefined block '%.*s'", (int)(len), (label)
#define GEN_ERR_MSG_REDEF_LABEL(label, len)								"redefinition of label '%.*s'", (int)(len), (label)

ivm_gen_env_t *
ivm_gen_env_new(ivm_gen_block_list_t *block_list)
{
	ivm_gen_env_t *ret = MEM_ALLOC(sizeof(*ret),
								   ivm_gen_env_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("generator environment"));

	ret->str_pool = ivm_string_pool_new(IVM_TRUE);
	ivm_ref_inc(ret->str_pool);
	ret->block_list = block_list;
	ret->jmp_table = ivm_gen_label_list_new();

	return ret;
}

IVM_PRIVATE
IVM_INLINE
void
_ivm_gen_env_cleanJumpTable(ivm_gen_env_t *env)
{
	ivm_gen_label_list_iterator_t liter;

	IVM_GEN_LABEL_LIST_EACHPTR(env->jmp_table, liter) {
		ivm_gen_label_free(IVM_GEN_LABEL_LIST_ITER_GET(liter));
	}
	ivm_gen_label_list_empty(env->jmp_table);

	return;
}

void
ivm_gen_env_free(ivm_gen_env_t *env)
{
	if (env) {
		ivm_string_pool_free(env->str_pool);
		ivm_gen_block_list_free(env->block_list);
		
		_ivm_gen_env_cleanJumpTable(env);
		ivm_gen_label_list_free(env->jmp_table);

		MEM_FREE(env);
	}

	return;
}

ivm_gen_label_t *
ivm_gen_label_new(const ivm_char_t *name,
				  ivm_size_t len,
				  ivm_bool_t is_def,
				  ivm_gen_pos_t pos,
				  ivm_size_t addr)
{
	ivm_gen_label_t *ret = MEM_ALLOC(sizeof(*ret),
									 ivm_gen_label_t *);
	ivm_gen_label_ref_t tmp;

	ret->has_def = is_def;
	ret->name = name;
	ret->len = len;

	ret->refs = ivm_gen_label_ref_list_new();

	if (is_def) {
		ret->def_pos = pos;
		ret->addr = addr;
	} else {
		tmp = (ivm_gen_label_ref_t) { pos, addr };
		ivm_gen_label_ref_list_add(ret->refs, &tmp);
	}

	return ret;
}

void
ivm_gen_label_free(ivm_gen_label_t *label)
{
	if (label) {
		ivm_gen_label_ref_list_free(label->refs);
		MEM_FREE(label);
	}

	return;
}

void
ivm_gen_block_list_free(ivm_gen_block_list_t *list)
{
	ivm_gen_block_list_iterator_t iter;

	if (list) {
		IVM_GEN_BLOCK_LIST_EACHPTR(list, iter) {
			ivm_gen_instr_list_free(IVM_GEN_BLOCK_LIST_ITER_GET(iter).instrs);
		}

		ivm_list_free(list);
	}

	return;
}

IVM_PRIVATE
void
_ivm_gen_label_addRef(ivm_gen_label_t *label,
					  ivm_gen_pos_t pos,
					  ivm_size_t addr)
{
	ivm_gen_label_ref_t tmp = (ivm_gen_label_ref_t) { pos, addr };
	ivm_gen_label_ref_list_add(label->refs, &tmp);
	return;
}

IVM_PRIVATE
ivm_long_t
_ivm_gen_env_refJumpAddr(ivm_gen_env_t *env,
						 const ivm_char_t *name,
						 ivm_size_t len,
						 ivm_gen_pos_t pos,
						 ivm_size_t addr)
{
	ivm_gen_label_list_iterator_t iter;
	ivm_gen_label_t *tmp;

	IVM_GEN_LABEL_LIST_EACHPTR(env->jmp_table, iter) {
		tmp = IVM_GEN_LABEL_LIST_ITER_GET(iter);
		if (!IVM_STRNCMP(tmp->name,
						 tmp->len,
						 name, len)) {
			if (tmp->has_def) {
				/* found definition */
				return (ivm_long_t)tmp->addr - (ivm_long_t)addr; /* to - from */
			}
			_ivm_gen_label_addRef(tmp, pos, addr);
			return 0;
		}
	}

	/* add new reference */
	ivm_gen_label_list_add(env->jmp_table,
						   ivm_gen_label_new(name, len, IVM_FALSE, pos, addr));

	return 0;
}

IVM_PRIVATE
void
_ivm_gen_env_addLabel(ivm_gen_env_t *env,
					  ivm_exec_t *exec,
					  const ivm_char_t *name,
					  ivm_size_t len,
					  ivm_gen_pos_t pos,
					  ivm_size_t addr)
{
	ivm_gen_label_ref_list_iterator_t riter;
	ivm_gen_label_list_iterator_t iter;
	ivm_gen_label_t *tmp;
	ivm_long_t from, to;

	IVM_GEN_LABEL_LIST_EACHPTR(env->jmp_table, iter) {
		tmp = IVM_GEN_LABEL_LIST_ITER_GET(iter);
		if (!IVM_STRNCMP(tmp->name,
						 tmp->len,
						 name, len)) {
			if (tmp->has_def) {
				/* found definition */
				GEN_ERR(pos, GEN_ERR_MSG_REDEF_LABEL(name, len));
				return;
			}

			tmp->addr = addr;
			tmp->def_pos = pos;

			/* apply address to all referers */
			to = addr;
			IVM_GEN_LABEL_REF_LIST_EACHPTR(tmp->refs, riter) {
				from = IVM_GEN_LABEL_REF_LIST_ITER_GET(riter).ref_addr;
				ivm_exec_setArgAt(exec, from, to - from);
			}

			tmp->has_def = IVM_TRUE;

			return;
		}
	}

	/* add new definition */
	ivm_gen_label_list_add(env->jmp_table,
						   ivm_gen_label_new(name, len, IVM_TRUE, pos, addr));

	return;
}

IVM_PRIVATE
ivm_function_id_t
_ivm_gen_env_getBlockID(ivm_gen_env_t *env,
						ivm_gen_pos_t pos,
						const ivm_char_t *label,
						ivm_size_t len)
{
	ivm_function_id_t ret = 0;
	ivm_gen_block_list_iterator_t iter;
	ivm_gen_block_t *tmp;

	IVM_GEN_BLOCK_LIST_EACHPTR(env->block_list, iter) {
		tmp = IVM_GEN_BLOCK_LIST_ITER_GET_PTR(iter);
		if (!IVM_STRNCMP(tmp->label,
						 tmp->len,
						 label, len)) {
			return ret;
		}
		ret++;
	}

	GEN_ERR(pos, GEN_ERR_MSG_UNDEFINED_BLOCK(label, len));

	return (ivm_function_id_t)-1;
}

IVM_PRIVATE
ivm_opcode_arg_t
_ivm_gen_opcode_arg_generateOpcodeArg(ivm_gen_opcode_arg_t arg,
									  ivm_gen_instr_t instr,
									  ivm_exec_t *exec,
									  ivm_gen_env_t *env,
									  const ivm_char_t param,
									  ivm_bool_t *failed)
{
	ivm_function_id_t tmp_id;
	ivm_char_t *tmp_str;
	ivm_opcode_arg_t tmp_ret;

#define SET_FAILED() \
	if (failed) *failed = IVM_TRUE;

#define UNMATCHED() \
	GEN_ERR(arg.pos, \
			GEN_ERR_MSG_UNMATCHED_ARGUMENT(instr.opcode, instr.olen, arg.type, param)); \
	SET_FAILED(); \
	return ivm_opcode_arg_fromInt(0); \

	switch (arg.type) {
		case 'N':
			if (param != 'N') {
				UNMATCHED();
			}
			
			return ivm_opcode_arg_fromInt(0);
		case 'I':
		case 'F':
			switch (param) {
				case 'I':
					return ivm_opcode_arg_fromInt(ivm_parser_parseNum(arg.val, arg.len, IVM_NULL));
				case 'F':
					return ivm_opcode_arg_fromFloat(ivm_parser_parseNum(arg.val, arg.len, IVM_NULL));
				case 'X':
					return ivm_opcode_arg_fromFunc(ivm_parser_parseNum(arg.val, arg.len, IVM_NULL));
				default: UNMATCHED();
			}
		case 'S':
			switch (param) {
				case 'S':
					tmp_str = ivm_parser_parseStr(arg.val, arg.len);
					tmp_ret = ivm_opcode_arg_fromInt(ivm_string_pool_registerRaw(env->str_pool, tmp_str));
					MEM_FREE(tmp_str);
					return tmp_ret;
				default: UNMATCHED();
			}
		case 'D':
			switch (param) {
				case 'I':
					return
						ivm_opcode_arg_fromInt(
							_ivm_gen_env_refJumpAddr(
								env, arg.val, arg.len,
								arg.pos, ivm_exec_cur(exec)
							)
						);
				case 'X':
					tmp_id = _ivm_gen_env_getBlockID(env, arg.pos, arg.val, arg.len);
					if (tmp_id == (ivm_function_id_t)-1) {
						SET_FAILED();
					}
					return ivm_opcode_arg_fromFunc(tmp_id);
				default: UNMATCHED();
			}
	}

#undef UNMATCHED

	IVM_FATAL(GEN_ERR_MSG_UNKNOWN_ARGUMENT_TYPE(arg.type));

	return ivm_opcode_arg_fromInt(0);
}

IVM_PRIVATE
ivm_exec_t *
_ivm_gen_block_generateExec(ivm_gen_block_t *block,
							ivm_gen_env_t *env)
{
	ivm_opcode_t opc;
	ivm_opcode_arg_t arg;
	ivm_gen_instr_t instr;
	const ivm_char_t *param;
	ivm_gen_instr_list_iterator_t iter;
	ivm_exec_t *ret = ivm_exec_new(env->str_pool);
	ivm_bool_t failed;

	if (block->instrs) {
		IVM_GEN_INSTR_LIST_EACHPTR(block->instrs, iter) {
			instr = IVM_GEN_INSTR_LIST_ITER_GET(iter);

			if (instr.label) {
				_ivm_gen_env_addLabel(
					env, ret, instr.label, instr.llen,
					instr.pos, ivm_exec_cur(ret)
				);
			}

			if (instr.opcode) {
				opc = ivm_opcode_searchOp_len(instr.opcode, instr.olen);
				if (opc == IVM_OPCODE(LAST)) {
					GEN_ERR(instr.pos,
							GEN_ERR_MSG_UNKNOWN_OPCODE(instr.opcode, instr.olen));
					opc = IVM_OPCODE(NOP);
				}

				param = ivm_opcode_table_getParam(opc);
				failed = IVM_FALSE;
				arg =
				_ivm_gen_opcode_arg_generateOpcodeArg(
					instr.arg, instr, ret,
					env, param[0], &failed
				);

				if (failed) {
					opc = IVM_OPCODE(NOP);
				}
			} else {
				opc = IVM_OPCODE(NOP);
			}

			ivm_exec_addInstr_c(ret, ivm_instr_build(opc, arg));
		}
	}

	_ivm_gen_env_cleanJumpTable(env);

	// ivm_dbg_printExec(ret, "  ", stderr);

	return ret;
}

ivm_exec_unit_t *
ivm_gen_env_generateExecUnit(ivm_gen_env_t *env)
{
	ivm_exec_unit_t *ret;
	ivm_exec_list_t *execs = ivm_exec_list_new();
	ivm_gen_block_t *block;
	ivm_exec_t *exec;
	ivm_size_t i = 0, root = -1;
	ivm_gen_block_list_iterator_t iter;

	if (env->block_list) {
		IVM_GEN_BLOCK_LIST_EACHPTR(env->block_list, iter) {
			block = IVM_GEN_BLOCK_LIST_ITER_GET_PTR(iter);
			exec = _ivm_gen_block_generateExec(block, env);

			if (!IVM_STRNCMP("root", IVM_STRLEN("root"),
							 block->label, block->len)) {
				root = i;
			}

			ivm_exec_list_push(execs, exec);
			i++;
		}
	}

	ret = ivm_exec_unit_new(root, execs);

	return ret;
}

ivm_vmstate_t *
ivm_gen_env_generateVM(ivm_gen_env_t *env)
{
	return ivm_exec_unit_generateVM(
		ivm_gen_env_generateExecUnit(env)
	);
}
