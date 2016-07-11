#ifndef _IVM_VM_INLINE_FUNC_H_
#define _IVM_VM_INLINE_FUNC_H_

#include "pub/com.h"
#include "pub/vm.h"

#include "std/string.h"

#include "../obj.h"
#include "../context.h"
#include "../func.h"
#include "../runtime.h"

IVM_COM_HEADER

IVM_INLINE
void
_ivm_function_invoke_c(const ivm_function_t *func,
					   ivm_vmstate_t *state,
					   ivm_ctchain_t *context,
					   ivm_runtime_t *runtime)
{
	if (func->is_native) {
		context = ivm_ctchain_clone(context, state);
		ivm_runtime_invoke(runtime, state, IVM_NULL, context);
	} else {
		context = ivm_ctchain_appendContext(context, state,
											ivm_context_new(state));
		ivm_runtime_invoke(runtime, state, &func->u.body, context);
	}

	return;
}

IVM_INLINE
void
_ivm_function_invoke_b(const ivm_function_t *func,
					   ivm_vmstate_t *state,
					   ivm_ctchain_t *context,
					   ivm_runtime_t *runtime,
					   ivm_object_t *base)
{
	ivm_context_t *ct;

	if (func->is_native) {
		context = ivm_ctchain_clone(context, state);
		ivm_runtime_invoke(runtime, state, IVM_NULL, context);
	} else {
		context = ivm_ctchain_appendContext(context, state,
											(ct = ivm_context_new(state)));

		ivm_runtime_invoke(runtime, state, &func->u.body, context);

		ivm_object_setSlot(
			ivm_context_toObject(ct),
			state,
			IVM_VMSTATE_CONST(state, C_BASE),
			base
		);
	}

	return;
}

IVM_INLINE
ivm_runtime_t *
ivm_function_createRuntime(const ivm_function_t *func,
						   ivm_vmstate_t *state,
						   ivm_ctchain_t *context,
						   ivm_coro_t *coro)
{
	ivm_runtime_t *ret = ivm_runtime_new(state);
	ivm_object_t **sp = ivm_vmstack_bottom(IVM_CORO_GET(coro, STACK));

	IVM_RUNTIME_SET(ret, SP, sp);
	IVM_RUNTIME_SET(ret, BP, sp);
	_ivm_function_invoke_c(func, state, context, ret);

	return ret;
}

IVM_INLINE
void
ivm_function_invoke(const ivm_function_t *func,
					ivm_vmstate_t *state,
					ivm_ctchain_t *context,
					ivm_coro_t *coro)
{
	ivm_runtime_t *runtime = IVM_CORO_GET(coro, RUNTIME);

	ivm_frame_stack_push(IVM_CORO_GET(coro, FRAME_STACK), runtime);
	_ivm_function_invoke_c(func, state, context, runtime);

	return;
}

IVM_INLINE
void
ivm_function_invokeBase(const ivm_function_t *func,
						ivm_vmstate_t *state,
						ivm_ctchain_t *context,
						ivm_coro_t *coro,
						ivm_object_t *base)
{
	ivm_runtime_t *runtime = IVM_CORO_GET(coro, RUNTIME);

	ivm_frame_stack_push(IVM_CORO_GET(coro, FRAME_STACK), runtime);
	_ivm_function_invoke_b(func, state, context, runtime, base);

	return;
}

IVM_INLINE
void
ivm_function_setExec(ivm_function_t *func,
					 ivm_vmstate_t *state,
					 ivm_exec_t *body)
{
	IVM_ASSERT(!func->is_native, "set native exec");

	ivm_exec_dump(&func->u.body);

	if (body) {
		ivm_exec_copy(body, &func->u.body);
		ivm_exec_preproc(&func->u.body, state);
	} else {
		MEM_INIT(&func->u.body, sizeof(func->u.body));
	}

	return;
}

IVM_COM_END

#endif
