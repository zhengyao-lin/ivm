#include "pub/com.h"
#include "pub/err.h"
#include "pub/vm.h"
#include "pub/inlines.h"

#include "std/mem.h"

#include "gc/gc.h"

#include "func.h"
#include "context.h"
#include "runtime.h"
#include "call.h"
#include "coro.h"

IVM_INLINE
void
_ivm_function_init(ivm_function_t *func,
				   ivm_vmstate_t *state,
				   ivm_exec_t *body)
{
	if (body) {
		ivm_exec_copy(body, &func->u.body);
		ivm_exec_preproc(&func->u.body, state);
	} else {
		STD_INIT(&func->u.body, sizeof(func->u.body));
	}

	func->ref = 0;
	func->is_native = IVM_FALSE;

	return;
}

IVM_INLINE
void
_ivm_function_initNative(ivm_function_t *func,
						 ivm_vmstate_t *state,
						 ivm_native_function_t native)
{
	func->u.native = native;
	func->ref = 0;
	func->is_native = IVM_TRUE;

	return;
}

ivm_function_t *
ivm_function_new(ivm_vmstate_t *state,
				 ivm_exec_t *body)
{
	ivm_function_t *ret = ivm_vmstate_allocFunc(state);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("function"));

	_ivm_function_init(ret, state, body);

	return ret;
}

ivm_function_t *
ivm_function_newNative(ivm_vmstate_t *state,
					   ivm_native_function_t func)
{
	ivm_function_t *ret = ivm_vmstate_allocFunc(state);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("native function"));

	_ivm_function_initNative(ret, state, func);

	return ret;
}

void
ivm_function_free(ivm_function_t *func,
				  ivm_vmstate_t *state)
{
	if (func && !--func->ref) {
		if (!func->is_native) {
			ivm_exec_dump(&func->u.body);
		}
		ivm_vmstate_dumpFunc(state, func);
	}

	return;
}

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
		STD_INIT(&func->u.body, sizeof(func->u.body));
	}

	return;
}

ivm_object_t *
ivm_function_object_new(ivm_vmstate_t *state,
						ivm_context_t *scope,
						ivm_function_t *func)
{
	ivm_function_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), state, IVM_FUNCTION_OBJECT_T);

	ret->scope = ivm_context_addRef(scope);
	ret->val = func;

	ivm_vmstate_addDesLog(state, IVM_AS_OBJ(ret)); /* function objects need destruction */

	return IVM_AS_OBJ(ret);
}

void
ivm_function_object_destructor(ivm_object_t *obj,
							   ivm_vmstate_t *state)
{	
	// ivm_function_object_t *func = IVM_AS(obj, ivm_function_object_t);

	ivm_context_free(IVM_AS(obj, ivm_function_object_t)->scope, state);

	return;
}

void
ivm_function_object_cloner(ivm_object_t *obj,
						   ivm_vmstate_t *state)
{
	ivm_context_addRef(IVM_AS(obj, ivm_function_object_t)->scope);
	ivm_vmstate_addDesLog(state, obj);
	
	return;
}

void
ivm_function_object_traverser(ivm_object_t *obj,
							  ivm_traverser_arg_t *arg)
{
	arg->trav_ctx(
		IVM_AS(obj, ivm_function_object_t)->scope,
		arg
	);

	return;
}

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
