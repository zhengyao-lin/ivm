#ifndef _IVM_VM_INLINE_FUNC_H_
#define _IVM_VM_INLINE_FUNC_H_

#include "pub/com.h"
#include "pub/vm.h"

#include "std/string.h"

#include "vm/obj.h"
#include "vm/context.h"
#include "vm/func.h"
#include "vm/runtime.h"

IVM_COM_HEADER

#define INVOKE_HANDLER(e1) \
	if (func->is_native) {                                          \
		ivm_runtime_invokeNative(runtime, state, ctx);              \
		return IVM_NULL;                                            \
	}                                                               \
                                                                    \
	e1;                                                             \
                                                                    \
	return ivm_runtime_invoke(runtime, state, &func->u.body, ctx);

IVM_INLINE
ivm_instr_t *
_ivm_function_invoke_c(const ivm_function_t *func,
					   ivm_vmstate_t *state,
					   ivm_context_t *ctx,
					   ivm_runtime_t *runtime)
{
	INVOKE_HANDLER(
		{ ctx = ivm_context_new(state, ctx); }
	);
}

// no new local context
IVM_INLINE
ivm_instr_t *
_ivm_function_invoke_r(const ivm_function_t *func,
					   ivm_vmstate_t *state,
					   ivm_context_t *ctx,
					   ivm_runtime_t *runtime)
{
	INVOKE_HANDLER(0);
}

// with base
IVM_INLINE
ivm_instr_t *
_ivm_function_invoke_b(const ivm_function_t *func,
					   ivm_vmstate_t *state,
					   ivm_context_t *ctx,
					   ivm_runtime_t *runtime,
					   ivm_object_t *base)
{
	INVOKE_HANDLER(
		{
			ctx = ivm_context_new(state, ctx);
			/* set base slot */
			ivm_context_setSlot(
				ctx, state,
				IVM_VMSTATE_CONST(state, C_BASE), base
			);
		}
	);
}

#undef INVOKE_HANDLER

IVM_INLINE
ivm_instr_t *
ivm_function_createRuntime(const ivm_function_t *func,
						   ivm_vmstate_t *state,
						   ivm_context_t *ctx,
						   ivm_coro_t *coro)
{
	ivm_runtime_t *runtime = IVM_CORO_GET(coro, RUNTIME);
	ivm_object_t **sp = ivm_vmstack_bottom(IVM_CORO_GET(coro, STACK));

	IVM_RUNTIME_SET(runtime, SP, sp);
	IVM_RUNTIME_SET(runtime, BP, sp);

	return _ivm_function_invoke_c(func, state, ctx, runtime);
}

IVM_INLINE
ivm_instr_t *
ivm_function_invoke(const ivm_function_t *func,
					ivm_vmstate_t *state,
					ivm_context_t *ctx,
					ivm_runtime_t *runtime,
					ivm_frame_stack_t *frame_st)
{
	ivm_frame_stack_push(frame_st, runtime);
	return _ivm_function_invoke_c(func, state, ctx, runtime);
}

IVM_INLINE
ivm_instr_t *
ivm_function_invokeBase(const ivm_function_t *func,
						ivm_vmstate_t *state,
						ivm_context_t *ctx,
						ivm_runtime_t *runtime,
						ivm_frame_stack_t *frame_st,
						ivm_object_t *base)
{
	ivm_frame_stack_push(frame_st, runtime);
	return _ivm_function_invoke_b(func, state, ctx, runtime, base);
}

IVM_INLINE
ivm_instr_t *
ivm_function_invoke_r(const ivm_function_t *func,
					  ivm_vmstate_t *state,
					  ivm_coro_t *coro,
					  ivm_context_t *ctx)
{
	ivm_runtime_t *runtime = IVM_CORO_GET(coro, RUNTIME);

	ivm_frame_stack_push(IVM_CORO_GET(coro, FRAME_STACK), runtime);
	return _ivm_function_invoke_c(func, state, ctx, runtime);
}

IVM_INLINE
ivm_instr_t *
ivm_function_object_invoke(ivm_function_object_t *obj,
						   ivm_vmstate_t *state,
						   ivm_coro_t *coro)
{
	ivm_runtime_t *runtime = IVM_CORO_GET(coro, RUNTIME);

	ivm_frame_stack_push(IVM_CORO_GET(coro, FRAME_STACK), runtime);
	return _ivm_function_invoke_c(
		obj->val, state, obj->scope, runtime
	);
}

IVM_INLINE
ivm_instr_t *
ivm_function_object_invokeBase(ivm_function_object_t *obj,
							   ivm_vmstate_t *state,
							   ivm_coro_t *coro,
							   ivm_object_t *base)
{
	ivm_runtime_t *runtime = IVM_CORO_GET(coro, RUNTIME);

	ivm_frame_stack_push(IVM_CORO_GET(coro, FRAME_STACK), runtime);
	return _ivm_function_invoke_b(
		obj->val, state, obj->scope, runtime, base
	);
}

IVM_COM_END

#endif
