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
		ivm_runtime_invokeNative(runtime, state, ctx, dump);        \
		return IVM_NULL;                                            \
	}                                                               \
                                                                    \
	e1;                                                             \
                                                                    \
	return ivm_runtime_invoke(runtime, state, ctx, &func->u.body, dump);

IVM_INLINE
ivm_instr_t *
_ivm_function_invoke_c(const ivm_function_t *func,
					   ivm_vmstate_t *state,
					   ivm_context_t *ctx,
					   ivm_runtime_t *runtime,
					   ivm_uint_t dump)
{
	INVOKE_HANDLER(
		{ ctx = ivm_context_new(state, ctx); }
	);
}

// with base
IVM_INLINE
ivm_instr_t *
_ivm_function_invoke_b(const ivm_function_t *func,
					   ivm_vmstate_t *state,
					   ivm_context_t *ctx,
					   ivm_runtime_t *runtime,
					   ivm_object_t *base,
					   ivm_uint_t dump)
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
	IVM_RUNTIME_SET(runtime, BCUR, 0);

	return _ivm_function_invoke_c(func, state, ctx, runtime, 0);
}

IVM_INLINE
ivm_instr_t *
ivm_function_invoke(const ivm_function_t *func,
					ivm_vmstate_t *state,
					ivm_context_t *ctx,
					ivm_block_stack_t *bstack,
					ivm_frame_stack_t *frame_st,
					ivm_runtime_t *runtime)
{
	ivm_coro_pushFrame_c(bstack, frame_st, runtime);
	return _ivm_function_invoke_c(func, state, ctx, runtime, 1);
}

IVM_INLINE
ivm_instr_t *
ivm_function_invokeBase(const ivm_function_t *func,
						ivm_vmstate_t *state,
						ivm_context_t *ctx,
						ivm_object_t *base,
						ivm_block_stack_t *bstack,
						ivm_frame_stack_t *frame_st,
						ivm_runtime_t *runtime)
{
	// ivm_frame_stack_push(frame_st, runtime);
	ivm_coro_pushFrame_c(bstack, frame_st, runtime);
	return _ivm_function_invoke_b(func, state, ctx, runtime, base, 2);
}

IVM_INLINE
ivm_instr_t *
ivm_function_invoke_r(const ivm_function_t *func,
					  ivm_vmstate_t *state,
					  ivm_coro_t *coro,
					  ivm_context_t *ctx)
{
	ivm_coro_pushFrame(coro);
	return _ivm_function_invoke_c(func, state, ctx, IVM_CORO_GET(coro, RUNTIME), 0);
}

IVM_INLINE
ivm_instr_t *
ivm_function_object_invoke(ivm_function_object_t *obj,
						   ivm_vmstate_t *state,
						   ivm_coro_t *coro)
{
	ivm_runtime_t *runtime = IVM_CORO_GET(coro, RUNTIME);

	ivm_coro_pushFrame(coro);
	
	return _ivm_function_invoke_c(obj->val, state, obj->scope, runtime, 0);
}

IVM_INLINE
ivm_instr_t *
ivm_function_object_invokeBase(ivm_function_object_t *obj,
							   ivm_vmstate_t *state,
							   ivm_coro_t *coro,
							   ivm_object_t *base)
{
	ivm_runtime_t *runtime = IVM_CORO_GET(coro, RUNTIME);

	ivm_coro_pushFrame(coro);
	
	return _ivm_function_invoke_b(obj->val, state, obj->scope, runtime, base, 0);
}

IVM_INLINE
ivm_object_t *
ivm_function_object_new(ivm_vmstate_t *state,
						ivm_context_t *scope,
						ivm_function_t *func)
{
	ivm_function_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), IVM_BTTYPE(state, IVM_FUNCTION_OBJECT_T));

	ret->scope = ivm_context_addRef(scope);
	ret->val = func;

	ivm_vmstate_addDesLog(state, IVM_AS_OBJ(ret)); /* function objects need destruction */

	return IVM_AS_OBJ(ret);
}

IVM_INLINE
void
ivm_function_object_init(ivm_function_object_t *obj,
						 ivm_vmstate_t *state,
						 ivm_context_t *scope,
						 ivm_function_t *func)
{
	ivm_object_init(IVM_AS_OBJ(obj), IVM_BTTYPE(state, IVM_FUNCTION_OBJECT_T));

	obj->scope = ivm_context_addRef(scope);
	obj->val = func;

	ivm_vmstate_addDesLog(state, IVM_AS_OBJ(obj)); /* function objects need destruction */

	return;
}

IVM_INLINE
ivm_object_t *
ivm_function_object_newNative(ivm_vmstate_t *state,
							  ivm_native_function_t func)
{
	return ivm_function_object_new(state, IVM_NULL, ivm_function_newNative(state, func));
}

IVM_INLINE
void
ivm_function_object_initNative(ivm_function_object_t *obj,
							   ivm_vmstate_t *state,
							   ivm_native_function_t func)
{
	ivm_function_object_init(obj, state, IVM_NULL, ivm_function_newNative(state, func));
	return;
}

IVM_COM_END

#endif
