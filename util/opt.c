#include "pub/type.h"
#include "pub/mem.h"

#include "opt.h"

IVM_PRIVATE
void
_ivm_opt_instr_dump(ivm_opt_instr_t *instr)
{
	if (instr) {
		ivm_ptlist_free(instr->refs);
	}

	return;
}

IVM_PRIVATE
ivm_opt_il_t *
_ivm_opt_il_new(ivm_string_pool_t *pool,
				ivm_size_t size,
				ivm_bool_t cached)
{
	ivm_opt_il_t *ret = MEM_ALLOC_INIT(sizeof(*ret),
									   ivm_opt_il_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("opt il"));

	ret->pool = pool;
	ivm_ref_inc(pool);
	ret->instrs = ivm_opt_instr_list_new(size);
	ret->cached = cached;

	return ret;
}

void
ivm_opt_il_free(ivm_opt_il_t *il)
{
	ivm_opt_instr_list_iterator_t iter;

	ivm_string_pool_free(il->pool);

	IVM_OPT_INSTR_LIST_EACHPTR(il->instrs, iter) {
		_ivm_opt_instr_dump(IVM_OPT_INSTR_LIST_ITER_GET(iter));
	}
	ivm_opt_instr_list_free(il->instrs);

	return;
}

IVM_INLINE
ivm_bool_t
_is_jump(ivm_opt_instr_t *instr)
{
	return ivm_opcode_table_isJump(instr->opc);
}

ivm_opt_il_t *
ivm_opt_il_convertFromExec(ivm_exec_t *exec)
{
	ivm_size_t len = ivm_exec_length(exec), pos;
	ivm_opt_il_t *ret = _ivm_opt_il_new(ivm_exec_pool(exec), len, ivm_exec_cached(exec));
	ivm_instr_t *i = ivm_exec_instrs(exec), *end;
	ivm_opt_instr_list_t *instrs = ret->instrs;
	ivm_opt_instr_t tmp, *cur, *jmpto;
	ivm_opt_instr_list_iterator_t iter;
	ivm_size_t addr;
	ivm_bool_t out_ref = IVM_FALSE;

	for (pos = 0, end = i + len;
		 i != end; i++, pos++) {
		tmp = ivm_opt_instr_build(
			.opc = ivm_instr_opcode(i),
			.arg = ivm_instr_arg(i)
		);
		ivm_opt_instr_list_push(instrs, &tmp);

		if (ivm_opcode_table_isJump(ivm_instr_opcode(i)) &&
			ivm_opcode_arg_toInt(ivm_instr_arg(i)) + pos >= len) {
			out_ref = IVM_TRUE;
		}
	}

	if (out_ref) {
		tmp = ivm_opt_instr_build(
			.opc = IVM_OPCODE(RETURN)
		);
		ivm_opt_instr_list_push(instrs, &tmp);
	}

	addr = 0;
	IVM_OPT_INSTR_LIST_EACHPTR(instrs, iter) {
		cur = IVM_OPT_INSTR_LIST_ITER_GET(iter);
		if (_is_jump(cur)) {
			jmpto
			= cur->jmpto
			= ivm_opt_instr_list_at(instrs, addr + ivm_opcode_arg_toInt(cur->arg));

			if (!jmpto->refs) {
				jmpto->refs = ivm_ptlist_new();
			}
			ivm_ptlist_push(jmpto->refs, cur);
		}
		addr++;
	}

	return ret;
}

IVM_INLINE
ivm_bool_t
_is_opposite_cond_jump(ivm_opt_instr_t *a,
					   ivm_opt_instr_t *b)
{
	ivm_opcode_t i = a->opc, j = b->opc;
	return
		(i == IVM_OPCODE(JUMP_TRUE_N) && j == IVM_OPCODE(JUMP_FALSE_N)) ||
		(i == IVM_OPCODE(JUMP_FALSE_N) && j == IVM_OPCODE(JUMP_TRUE_N));
}

IVM_INLINE
ivm_bool_t
_is_same_cond_jump(ivm_opt_instr_t *from,
				   ivm_opt_instr_t *to)
{
	// to is no-cond jump
	// or
	// from = to = (jump_false_n or jump_true_b)
	return to->opc == IVM_OPCODE(JUMP) ||
		   (from->opc == to->opc && (from->opc == IVM_OPCODE(JUMP_TRUE_N) ||
									 from->opc == IVM_OPCODE(JUMP_FALSE_N)));
}

void
ivm_opt_jumpReduce(ivm_opt_il_t *il)
{
	ivm_opt_instr_list_t *instrs = il->instrs;
	ivm_opt_instr_list_iterator_t iter;
	ivm_opt_instr_t *cur, *tmp;

	IVM_OPT_INSTR_LIST_EACHPTR(instrs, iter) {
		cur = IVM_OPT_INSTR_LIST_ITER_GET(iter);
		if (cur->jmpto) {
			// jump_xx addr1 => jump_xx addr2
			// ...
			// addr1: jump addr2
			for (tmp = cur->jmpto;
				 tmp->jmpto && _is_same_cond_jump(cur, tmp);
				 tmp = tmp->jmpto) ;
			cur->jmpto = tmp;

			// _is_opposite_cond_jump(
		}
	}

	return;
}
