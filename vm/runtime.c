#include "pub/mem.h"
#include "pub/err.h"
#include "pub/vm.h"
#include "pub/inlines.h"

#include "runtime.h"
#include "context.h"
#include "call.h"
#include "coro.h"

ivm_runtime_t *
ivm_runtime_new(ivm_vmstate_t *state)
{
	ivm_runtime_t *ret = MEM_ALLOC_INIT(sizeof(*ret),
										ivm_runtime_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("runtime"));

	return ret;
}

void
ivm_runtime_invoke(ivm_runtime_t *runtime,
				   ivm_vmstate_t *state,
				   const ivm_exec_t *exec,
				   ivm_ctchain_t *context)
{
	runtime->context = ivm_ctchain_addRef(context);
	runtime->ip = exec ? ivm_exec_instrPtrStart(exec) : IVM_NULL;
	runtime->bp = runtime->sp;

	return;
}
