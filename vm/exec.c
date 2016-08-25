#include <stdarg.h>

#include "pub/mem.h"
#include "pub/com.h"
#include "pub/err.h"
#include "pub/vm.h"

#include "std/string.h"
#include "std/ref.h"

#include "exec.h"
#include "byte.h"

IVM_PRIVATE
IVM_INLINE
void
_ivm_exec_init(ivm_exec_t *exec,
			   ivm_string_pool_t *pool)
{
	ivm_ref_init(exec);
	exec->offset = 0;
	exec->pool = pool;
	ivm_ref_inc(pool);

	exec->pos = ivm_source_pos_build(IVM_NULL);

	// exec->max_stack = 0;
	// exec->fin_stack = 0;
	exec->alloc = IVM_DEFAULT_EXEC_BUFFER_SIZE;
	exec->next = 0;
	exec->instrs = MEM_ALLOC(sizeof(*exec->instrs)
							* IVM_DEFAULT_EXEC_BUFFER_SIZE,
							ivm_instr_t *);

	IVM_ASSERT(exec->instrs, IVM_ERROR_MSG_FAILED_ALLOC_NEW("instruction list in executable"));

	exec->cached = IVM_FALSE;

	return;
}

ivm_exec_t *
ivm_exec_new(ivm_string_pool_t *pool)
{
	ivm_exec_t *ret = MEM_ALLOC(sizeof(*ret), ivm_exec_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("executable"));

	_ivm_exec_init(ret, pool);

	return ret;
}

void
ivm_exec_free(ivm_exec_t *exec)
{
	if (exec && !ivm_ref_dec(exec)) {
		ivm_string_pool_free(exec->pool);
		MEM_FREE(exec->instrs);
		MEM_FREE(exec);
	}

	return;
}

void
ivm_exec_dump(ivm_exec_t *exec)
{
	if (exec) {
		ivm_string_pool_free(exec->pool);
		MEM_FREE(exec->instrs);
	}

	return;
}

void
ivm_exec_copy(ivm_exec_t *exec,
			  ivm_exec_t *dest)
{
	ivm_size_t size;

	*dest = *exec;
	ivm_ref_init(dest);
	ivm_ref_inc(dest->pool);
	size = sizeof(*exec->instrs) * exec->alloc;
	dest->instrs = MEM_ALLOC(size, ivm_instr_t *);
	MEM_COPY(dest->instrs, exec->instrs, size);

	return;
}

IVM_PRIVATE
void
_ivm_exec_expand(ivm_exec_t *exec)
{
	exec->alloc <<= 1;
	exec->instrs = MEM_REALLOC(exec->instrs,
								sizeof(*exec->instrs)
								* exec->alloc,
								ivm_instr_t *);

	IVM_ASSERT(exec->instrs,
			   IVM_ERROR_MSG_FAILED_ALLOC_NEW("expanded instruction list in executable"));

	return;
}

ivm_size_t
ivm_exec_addInstr_c(ivm_exec_t *exec,
					ivm_instr_t instr)
{
	// ivm_int_t inc;

	if (exec->next >= exec->alloc) {
		_ivm_exec_expand(exec);
	}

	exec->instrs[exec->next] = instr;

	return exec->next++;
}

void
ivm_exec_preproc(ivm_exec_t *exec,
				 ivm_vmstate_t *state)
{
	ivm_size_t addr, end;
	ivm_instr_t *i, *j;
	ivm_source_pos_t *pos = ivm_exec_getSourcePos(exec);

	if (!exec->cached) {
		exec->cached = IVM_TRUE;
		for (addr = 0, end = exec->next;
			 addr != end; addr++) {
			i = exec->instrs + addr;
			ivm_instr_setPos(i, pos);
			
			switch (ivm_opcode_table_getParam(ivm_instr_opcode(i))[0]) {
				case 'S':
					/* string pool idx -> string pointer */
					ivm_instr_setArg(i,
						ivm_opcode_arg_fromPointer(
							ivm_exec_getString(exec,
								ivm_opcode_arg_toInt(
									ivm_instr_arg(i)
								)
							)
						)
					);
					break;
				case 'A':
					if (ivm_opcode_arg_toInt(ivm_instr_arg(i)) + addr >= end) {
						ivm_exec_addInstr(exec, RETURN);
						end = exec->next; // update end
					}
					break;
			}
		}

		if (!exec->next
			|| (ivm_instr_opcode(exec->instrs + (exec->next - 1))
				!= IVM_OPCODE(RETURN))) {
			// avoid empty address
			ivm_exec_addInstr(exec, RETURN);
		}

		for (i = exec->instrs, j = i + exec->next;
			 i != j; i++) {
			// cache final jump addr of jump instr
			
			if (ivm_opcode_table_getParam(ivm_instr_opcode(i))[0]
				== 'A') {
				ivm_instr_setArg(i,
					ivm_opcode_arg_fromPointer(
						i + ivm_opcode_arg_toInt(
							ivm_instr_arg(i)
						)
					)
				);
			}
		}
	}

	return;
}

ivm_instr_t
ivm_exec_decache(ivm_exec_t *exec,
				 ivm_vmstate_t *state,
				 ivm_instr_t *instr)
{
	ivm_size_t tmp;
	ivm_char_t arg = ivm_opcode_table_getParam(ivm_instr_opcode(instr))[0];
	ivm_ptr_t tmp_instr;

	if (arg == 'S') {
		tmp = ivm_string_pool_find(
			exec->pool,
			(const ivm_string_t *)ivm_opcode_arg_toPointer(ivm_instr_arg(instr))
		);

		IVM_ASSERT(tmp != -1, IVM_ERROR_MSG_UNEXPECTED_INSTR_ARG_CACHE);

		return ivm_instr_build(
			ivm_instr_opcode(instr),
			ivm_opcode_arg_fromInt(tmp)
		);
	} else if (ivm_opcode_table_isJump(ivm_instr_opcode(instr))) {
		tmp_instr = ivm_opcode_arg_toPointer(ivm_instr_arg(instr));
		return ivm_instr_build(
			ivm_instr_opcode(instr),
			ivm_opcode_arg_fromInt((tmp_instr - (ivm_ptr_t)instr) / sizeof(instr))
		);
	}

	return ivm_instr_build(
		ivm_instr_opcode(instr),
		ivm_instr_arg(instr)
	);
}

ivm_exec_unit_t *
ivm_exec_unit_new(ivm_size_t root,
				  ivm_exec_list_t *execs)
{
	ivm_exec_unit_t *ret = MEM_ALLOC(sizeof(*ret), ivm_exec_unit_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("executable unit"));

	ret->root = root;
	ret->execs = execs;
	ret->offset = 0;

	return ret;
}

void
ivm_exec_unit_free(ivm_exec_unit_t *unit)
{
	if (unit) {
		ivm_exec_list_free(unit->execs);
		MEM_FREE(unit);
	}

	return;
}

ivm_function_t * /* root function */
ivm_exec_unit_mergeToVM(ivm_exec_unit_t *unit,
						ivm_vmstate_t *state)
{
	ivm_function_t *func, *root = IVM_NULL;
	ivm_exec_t *exec;
	ivm_exec_list_iterator_t eiter;
	ivm_size_t i = 0;
	ivm_uint_t offset = unit->offset;

	IVM_ASSERT(
		unit->offset == ivm_vmstate_getLinkOffset(state),
		IVM_ERROR_MSG_LINK_OFFSET_MISMATCH(
			ivm_vmstate_getLinkOffset(state), unit->offset
		)
	);

	IVM_EXEC_LIST_EACHPTR(unit->execs, eiter) {
		exec = IVM_EXEC_LIST_ITER_GET(eiter);
		ivm_exec_setOffset(exec, offset);

		func = ivm_function_new(state, exec);
		ivm_vmstate_registerFunc(state, func);

		if (i++ == unit->root) {
			root = func;
		}

		// ivm_exec_preproc(exec, state);
	}

	return root;
}

ivm_vmstate_t *
ivm_exec_unit_generateVM(ivm_exec_unit_t *unit)
{
	ivm_vmstate_t *state = ivm_vmstate_new();
	ivm_function_t *root = ivm_exec_unit_mergeToVM(unit, state);

	if (root) {
		ivm_vmstate_addCoroToCurCGroup(
			state, IVM_AS(
				ivm_function_object_new(
					state, IVM_NULL, root
				),
				ivm_function_object_t
			)
		);
	}

	return state;
}
