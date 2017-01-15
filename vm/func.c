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
	func->has_param = !ivm_param_list_isLegacy(ivm_exec_getParam(&func->u.body));

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
	func->has_param = IVM_FALSE;

	return;
}

ivm_function_t *
ivm_function_new(ivm_vmstate_t *state,
				 ivm_exec_t *body)
{
	ivm_function_t *ret = ivm_vmstate_allocFunc(state);

	IVM_MEMCHECK(ret);

	_ivm_function_init(ret, state, body);

	return ret;
}

ivm_function_t *
ivm_function_newNative(ivm_vmstate_t *state,
					   ivm_native_function_t func)
{
	ivm_function_t *ret = ivm_vmstate_allocFunc(state);

	IVM_MEMCHECK(ret);

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
