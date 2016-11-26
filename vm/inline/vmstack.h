#ifndef _IVM_VM_INLINE_VMSTACK_H_
#define _IVM_VM_INLINE_VMSTACK_H_

#include "pub/com.h"
#include "pub/err.h"
#include "pub/vm.h"

#include "std/mem.h"

#include "vm/call.h"
#include "vm/runtime.h"
#include "vm/vmstack.h"

IVM_COM_HEADER

IVM_INLINE
ivm_object_t *
ivm_vmstack_pop(ivm_coro_t *coro)
{
	ivm_runtime_t *runtime = IVM_CORO_GET(coro, RUNTIME);

	IVM_ASSERT(runtime, IVM_ERROR_MSG_PUSH_DEAD_CORO_STACK);

	return *IVM_RUNTIME_GET(runtime, DEC_SP);
}

IVM_INLINE
void
ivm_vmstack_popN(ivm_coro_t *coro,
				 ivm_int_t n)
{
	ivm_runtime_t *runtime = IVM_CORO_GET(coro, RUNTIME);

	IVM_ASSERT(runtime, IVM_ERROR_MSG_PUSH_DEAD_CORO_STACK);

	ivm_runtime_decSP(runtime, n);

	return;
}

IVM_INLINE
ivm_object_t *
ivm_vmstack_top(ivm_coro_t *coro)
{
	ivm_runtime_t *runtime = IVM_CORO_GET(coro, RUNTIME);

	IVM_ASSERT(runtime, IVM_ERROR_MSG_PUSH_DEAD_CORO_STACK);

	return *(IVM_RUNTIME_GET(runtime, SP) - 1);
}

// 1 prev is the top
IVM_INLINE
ivm_object_t *
ivm_vmstack_prev(ivm_coro_t *coro,
				 ivm_int_t prev)
{
	ivm_runtime_t *runtime = IVM_CORO_GET(coro, RUNTIME);

	// IVM_TRACE("%p %p\n", runtime->bp, runtime->sp);

	IVM_ASSERT(runtime, IVM_ERROR_MSG_PUSH_DEAD_CORO_STACK);

	return *(IVM_RUNTIME_GET(runtime, SP) - prev);
}

IVM_INLINE
void
ivm_vmstack_push(ivm_coro_t *coro,
				 ivm_object_t *obj)
{
	ivm_vmstack_t *stack = IVM_CORO_GET(coro, STACK);
	ivm_runtime_t *runtime = IVM_CORO_GET(coro, RUNTIME);
	ivm_object_t **sp;

	IVM_ASSERT(runtime, IVM_ERROR_MSG_PUSH_DEAD_CORO_STACK);
	
	sp = IVM_RUNTIME_GET(runtime, SP_INC);

	*sp++ = obj;

	if (sp == stack->edge) {
		ivm_vmstack_inc_c(stack, coro);
	}

	return;
}

IVM_COM_END

#endif
