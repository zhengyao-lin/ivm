#include "pub/mem.h"
#include "coro.h"
#include "stack.h"
#include "call.h"
#include "vm.h"
#include "context.h"
#include "op.h"
#include "err.h"

ivm_coro_t *
ivm_coro_new()
{
	ivm_coro_t *ret = MEM_ALLOC(sizeof(*ret));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("coroutine"));

	ret->stack = ivm_vmstack_new();
	ret->call_st = ivm_call_stack_new();
	ret->runtime = IVM_NULL;

	return ret;
}

void
ivm_coro_free(ivm_coro_t *coro)
{
	if (coro) {
		ivm_call_stack_foreach(coro->call_st, (ivm_stack_foreach_proc_t)ivm_caller_info_free);
		ivm_vmstack_free(coro->stack);
		ivm_call_stack_free(coro->call_st);
		ivm_runtime_free(coro->runtime);
		MEM_FREE(coro);
	}

	return;
}

ivm_object_t *
ivm_coro_start(ivm_coro_t *coro, ivm_vmstate_t *state, ivm_function_t *root)
{
	ivm_object_t *ret = IVM_NULL;
#if 0
	ivm_function_t *tmp_func;
	ivm_exec_t *exec = ivm_exec_new();
#endif
	ivm_op_proc_t tmp_proc;
	ivm_caller_info_t *tmp_info;

#if 0
	tmp_func = ivm_function_new(NULL, exec, IVM_INTSIG_NONE);

	ivm_call_stack_push(coro->call_st, ivm_function_invoke(tmp_func, coro));

	ivm_exec_addCode(exec, IVM_OP_NEW_OBJ, 0);
	ivm_exec_addCode(exec, IVM_OP_NEW_OBJ, 0);
#endif

#if 0
	ivm_function_free(tmp_func);
	ivm_exec_free(exec);
#endif

	if (coro->runtime) {
		/* coro has been executed(is sleeping now), start restoration */
		/* root of sleeping coro cannot be reset */
		IVM_ASSERT(!root, IVM_ERROT_MSG_RESET_CORO_ROOT);
	} else {
		/* first run */
		coro->runtime = ivm_function_createRuntime(root);
	}

	if (ivm_function_isNative(root)) {
		ret = ivm_function_callNative(root, state, coro->runtime->context, NULL, 0, NULL);
	} else {
		while (1) {
			while (coro->runtime->pc < ivm_exec_length(coro->runtime->exec)) {
				tmp_proc = ivm_op_table_getProc(ivm_exec_opAt(coro->runtime->exec,
															  coro->runtime->pc));
				tmp_proc(state, coro);
			}

			tmp_info = ivm_call_stack_pop(coro->call_st);
			if (tmp_info)
				ivm_caller_info_free(tmp_info);
			else break; /* no more callee to restore, end executing */
		}

		ret = ivm_vmstack_top(coro->stack);
	}

	return ret ? ret : IVM_NULL_OBJ(state);
}
