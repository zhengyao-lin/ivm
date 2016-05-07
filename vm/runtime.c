#include "pub/mem.h"
#include "runtime.h"
#include "context.h"
#include "call.h"
#include "coro.h"
#include "err.h"

ivm_runtime_t *
ivm_runtime_new(ivm_exec_t *exec, ivm_ctchain_t *context)
{
	ivm_runtime_t *ret = MEM_ALLOC(sizeof(*ret));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("runtime"));

	ret->pc = 0;
	ret->exec = exec;
	ret->context = context
				   ? ivm_ctchain_clone(context)
				   : ivm_ctchain_new();

	return ret;
}

void
ivm_runtime_free(ivm_runtime_t *runtime)
{
	if (runtime) {
		ivm_ctchain_free(runtime->context);
		MEM_FREE(runtime);
	}
	
	return;
}

ivm_caller_info_t *
ivm_runtime_invoke(ivm_runtime_t *runtime, ivm_coro_t *coro,
				   ivm_exec_t *exec, ivm_ctchain_t *context)
{
	ivm_pc_t cur_pc = runtime->pc;
	ivm_exec_t *cur_exec = runtime->exec;
	ivm_ctchain_t *cur_ct = runtime->context;

	runtime->pc = 0;
	runtime->exec = exec;
	runtime->context = ivm_ctchain_clone(context);

	return ivm_caller_info_new(cur_exec,
							   ivm_coro_stackTop(coro),
							   cur_pc, cur_ct);
}

void
ivm_runtime_restore(ivm_runtime_t *runtime, ivm_coro_t *coro,
					struct ivm_caller_info_t_tag *info)
{
	ivm_ctchain_free(runtime->context);

	runtime->pc = IVM_CALLER_INFO_GET(info, PC);
	runtime->exec = IVM_CALLER_INFO_GET(info, EXEC);
	runtime->context = IVM_CALLER_INFO_GET(info, CONTEXT);

	return;
}
