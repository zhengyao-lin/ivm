#include <stdarg.h>
#include "pub/mem.h"
#include "pub/com.h"
#include "pub/err.h"

#include "std/string.h"
#include "exec.h"
#include "byte.h"

ivm_exec_t *
ivm_exec_new(ivm_string_pool_t *pool)
{
	ivm_exec_t *ret = MEM_ALLOC(sizeof(*ret), ivm_exec_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("executable"));

	ret->pool = pool;
	ret->alloc = IVM_DEFAULT_INSTR_BLOCK_BUFFER_SIZE;
	ret->next = 0;
	ret->instrs = MEM_ALLOC(sizeof(*ret->instrs)
							* IVM_DEFAULT_INSTR_BLOCK_BUFFER_SIZE,
							ivm_instr_t *);

	IVM_ASSERT(ret->instrs, IVM_ERROR_MSG_FAILED_ALLOC_NEW("instruction list in executable"));

	return ret;
}

void
ivm_exec_free(ivm_exec_t *exec)
{
	if (exec) {
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
