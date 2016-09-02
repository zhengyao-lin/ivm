#include "pub/type.h"

#include "std/mem.h"

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
	ivm_opt_il_t *ret = STD_ALLOC_INIT(sizeof(*ret),
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

	STD_FREE(il);

	return;
}

IVM_INLINE
ivm_bool_t
_is_jump(ivm_opt_instr_t *instr)
{
	return ivm_opcode_table_isJump(instr->opc);
}

IVM_INLINE
ivm_bool_t
_has_loop(ivm_opt_instr_t *instr)
{
	ivm_opt_instr_t *tmp = instr->jmpto;

	while (tmp) {
		if (tmp == instr) return IVM_TRUE;
		tmp = tmp->jmpto;
	}

	return IVM_FALSE;
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
			.lineno = ivm_instr_lineno(i),
			.opc = ivm_instr_opcode(i),
			.arg = ivm_instr_arg(i),
			.addr = -1
		);
		ivm_opt_instr_list_push(instrs, &tmp);

		if (ivm_opcode_table_isJump(ivm_instr_opcode(i)) &&
			ivm_opcode_arg_toInt(ivm_instr_arg(i)) + pos >= len) {
			out_ref = IVM_TRUE;
		}
	}

	if (out_ref || !len || ivm_instr_opcode(i - 1) != IVM_OPCODE(RETURN)) {
		tmp = ivm_opt_instr_build(
			.lineno = -1,
			.opc = IVM_OPCODE(RETURN),
			.addr = -1
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
ivm_size_t
_ivm_opt_getRealRefCount(ivm_ptlist_t *refs)
{
	ivm_size_t ret = 0;
	ivm_ptlist_iterator_t iter;

	if (refs) {
		IVM_PTLIST_EACHPTR(refs, iter, void *) {
			if (IVM_PTLIST_ITER_GET(iter))
				ret++;
		}
	}

	return ret;
}

void
ivm_opt_il_generateExec(ivm_opt_il_t *il,
						ivm_exec_t *dest)
{
	ivm_opt_instr_list_iterator_t iter;
	ivm_opt_instr_t *cur, *tmp;

	ivm_exec_setCached(dest, il->cached);

	{
		IVM_OPT_INSTR_LIST_EACHPTR(il->instrs, iter) {
			cur = IVM_OPT_INSTR_LIST_ITER_GET(iter);

			if (cur->opc == IVM_OPCODE(NOP) &&
				!_ivm_opt_getRealRefCount(cur->refs)) { // don't gen nop(if no ref toward it)
				continue;
			}

			cur->addr = ivm_exec_addInstr_c(dest, ivm_instr_build_l(cur->opc, cur->arg, cur->lineno));
		}
	}

	{
		// rewrite jump addr
		IVM_OPT_INSTR_LIST_EACHPTR(il->instrs, iter) {
			cur = IVM_OPT_INSTR_LIST_ITER_GET(iter);
			if (_is_jump(cur) && cur->addr != -1) {
				// is jump instr and is generated
				tmp = cur->jmpto;
				IVM_ASSERT(tmp->addr != -1, IVM_ERROR_MSG_OPT_NO_GEN_FOR_JMPTO);
				ivm_exec_setArgAt(dest, cur->addr, tmp->addr - cur->addr);
			}
		}
	}

	return;
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
	// from = to = (jump_false_n or jump_true_n)
	return to->opc == IVM_OPCODE(JUMP) ||
		   (from->opc == to->opc && (from->opc == IVM_OPCODE(JUMP_TRUE_N) ||
									 from->opc == IVM_OPCODE(JUMP_FALSE_N)));
}

IVM_INLINE
void
ivm_ptlist_findAndRemove(ivm_ptlist_t *list, void *p)
{
	ivm_size_t i = ivm_ptlist_find(list, p);
	
	if (i != -1) {
		ivm_ptlist_set(list, i, IVM_NULL);
	}

	return;
}

IVM_PRIVATE
void
_ivm_opt_jumpReduce(ivm_opt_il_t *il)
{
	ivm_opt_instr_list_t *instrs = il->instrs;
	ivm_opt_instr_list_iterator_t iter;
	ivm_opt_instr_t *cur, *tmp;
	ivm_bool_t suc_jump = IVM_FALSE;

	IVM_OPT_INSTR_LIST_EACHPTR(instrs, iter) {
		cur = IVM_OPT_INSTR_LIST_ITER_GET(iter);

		if (cur->jmpto) {
			// jump_xx addr1 => jump_xx addr2
			// ...
			// addr1: jump addr2
			for (tmp = cur->jmpto;
				 tmp->jmpto && _is_same_cond_jump(cur, tmp);
				 tmp = tmp->jmpto)
				if (tmp == cur) break; // loop

			if (tmp != cur->jmpto) {
				ivm_ptlist_findAndRemove(cur->jmpto->refs, cur);
				cur->jmpto = tmp;
			}

			if (_is_opposite_cond_jump(cur, tmp)) {
				cur->jmpto = tmp + 1;
			}
		}

		/*
			addr1: jump_false addr3 => jump_true addrx (vice versa)
			addr2: jump addrx       => nop
			addr3:
		 */

		if (cur->opc == IVM_OPCODE(JUMP_FALSE) &&
			cur->jmpto == cur + 2 &&
			(cur + 1)->opc == IVM_OPCODE(JUMP)) {
			cur->jmpto = (cur + 1)->jmpto;
			cur->opc = IVM_OPCODE(JUMP_TRUE);
			(cur + 1)->opc = IVM_OPCODE(NOP);
		} else if (cur->opc == IVM_OPCODE(JUMP_TRUE) &&
				   cur->jmpto == cur + 2 &&
				   (cur + 1)->opc == IVM_OPCODE(JUMP)) {
			cur->jmpto = (cur + 1)->jmpto;
			cur->opc = IVM_OPCODE(JUMP_FALSE);
			(cur + 1)->opc = IVM_OPCODE(NOP);
		}

		/*
			jump addr3 => jump addr3
			jump addr2 => nop
		 */
		if (cur->opc == IVM_OPCODE(JUMP)) {
			if (suc_jump || cur->jmpto == cur + 1) {
				// if there are successive jumps or jump argument is next instr
				// do not gen
				cur->opc = IVM_OPCODE(NOP);
			} else {
				suc_jump = IVM_TRUE;
			}
		} else {
			suc_jump = IVM_FALSE;
		}
	}

	return;
}

IVM_INLINE
ivm_bool_t
_is_raw_cmp(ivm_opcode_t opc)
{
	return opc == IVM_OPCODE(NE) ||
		   opc == IVM_OPCODE(EQ) ||
		   opc == IVM_OPCODE(GT) ||
		   opc == IVM_OPCODE(GE) ||
		   opc == IVM_OPCODE(LT) ||
		   opc == IVM_OPCODE(LE);
}

IVM_INLINE
ivm_bool_t
_is_raw_jump(ivm_opcode_t opc)
{
	return opc == IVM_OPCODE(JUMP_FALSE) ||
		   opc == IVM_OPCODE(JUMP_TRUE);
}

IVM_INLINE
ivm_opcode_t
_to_reg_op(ivm_opcode_t cmp)
{
#define CASE(op) \
	case IVM_OPCODE(op): \
		return IVM_OPCODE(op##_R);

	switch (cmp) {
		CASE(NE)
		CASE(EQ)
		CASE(GT)
		CASE(GE)
		CASE(LT)
		CASE(LE)
		CASE(JUMP_FALSE)
		CASE(JUMP_TRUE)
		default: break;
	}

#undef CASE

	IVM_FATAL("impossible");

	return IVM_OPCODE(NOP);
}

IVM_PRIVATE
void
_ivm_opt_cmpOpt(ivm_opt_il_t *il)
{
	ivm_opt_instr_list_t *instrs = il->instrs;
	ivm_opt_instr_list_iterator_t iter;
	ivm_opt_instr_t *cur;

	IVM_OPT_INSTR_LIST_EACHPTR(instrs, iter) {
		cur = IVM_OPT_INSTR_LIST_ITER_GET(iter);
		if (_is_raw_cmp(cur->opc) &&
			_is_raw_jump((cur + 1)->opc)) {
			cur->opc = _to_reg_op(cur->opc);
			(cur + 1)->opc = _to_reg_op((cur + 1)->opc);
		}
	}

	return;
}

void
ivm_opt_optExec(ivm_exec_t *exec)
{
	ivm_opt_il_t *il = ivm_opt_il_convertFromExec(exec);

	ivm_exec_empty(exec);

	_ivm_opt_jumpReduce(il);
	_ivm_opt_cmpOpt(il);

	ivm_opt_il_generateExec(il, exec);
	ivm_opt_il_free(il);

	return;
}
