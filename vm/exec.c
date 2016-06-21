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
	exec->cached = IVM_FALSE;
	exec->pool = pool;
	ivm_ref_inc(pool);
	exec->alloc = IVM_DEFAULT_INSTR_BLOCK_BUFFER_SIZE;
	exec->next = 0;
	exec->instrs = MEM_ALLOC(sizeof(*exec->instrs)
							* IVM_DEFAULT_INSTR_BLOCK_BUFFER_SIZE,
							ivm_instr_t *);

	IVM_ASSERT(exec->instrs, IVM_ERROR_MSG_FAILED_ALLOC_NEW("instruction list in executable"));

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
	if (exec) {
		ivm_string_pool_free(exec->pool);
		MEM_FREE(exec->instrs);
		MEM_FREE(exec);
	}

	return;
}

IVM_PRIVATE
void
ivm_exec_expand(ivm_exec_t *exec)
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
	if (exec->next >= exec->alloc) {
		ivm_exec_expand(exec);
	}

	exec->instrs[exec->next] = instr;

	return exec->next++;
}

void
ivm_exec_preproc(ivm_exec_t *exec,
				 ivm_vmstate_t *state)
{
	ivm_instr_t *i, *end;

	if (!exec->cached) {
		exec->cached = IVM_TRUE;
		for (i = exec->instrs, end = i + exec->next;
			 i != end; i++) {
			switch (ivm_opcode_table_getParam(ivm_instr_opcode(i))[0]) {
				case 'S':
					/* string pool idx -> string pointer */
					ivm_instr_setArg(i,
						ivm_opcode_arg_fromPointer(
							ivm_exec_getString(exec, /**/
								ivm_opcode_arg_toInt(
									ivm_instr_arg(
										i
									)
								)
							)
						)
					);
					break;
			}
		}
	}

	return;
}
