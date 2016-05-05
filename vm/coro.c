#include "pub/mem.h"
#include "coro.h"
#include "vmstack.h"
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
	ivm_op_proc_t tmp_proc;
	ivm_caller_info_t *tmp_info;

	if (coro->runtime) {
		/* coro has been executed(is sleeping now), start restoration */
		/* root of sleeping coro cannot be reset */
		IVM_ASSERT(!root, IVM_ERROT_MSG_RESET_CORO_ROOT);
	} else {
		/* first run */
		coro->runtime = ivm_function_createRuntime(state, root);
	}

	if (ivm_function_isNative(root)) {
		ret = ivm_function_callNative(root, state, IVM_RUNTIME_GET(coro->runtime, CONTEXT), IVM_NULL, 0, IVM_NULL);
	} else if (coro->runtime) {
		while (1) {
			while (IVM_RUNTIME_GET(coro->runtime, PC)
				   < ivm_exec_length(IVM_RUNTIME_GET(coro->runtime, EXEC))) {
				tmp_proc = ivm_op_table_getProc(ivm_exec_opAt(IVM_RUNTIME_GET(coro->runtime, EXEC),
															  IVM_RUNTIME_GET(coro->runtime, PC)));
				switch (tmp_proc(state, coro)) {
					case IVM_ACTION_BREAK:
						goto ACTION_BREAK;
					case IVM_ACTION_YIELD:
						goto ACTION_YIELD;
					default:;
				}
				ivm_vmstate_checkGC(state);
			}
ACTION_BREAK:

			tmp_info = ivm_call_stack_pop(coro->call_st);
			if (tmp_info) {
				ivm_runtime_restore(coro->runtime, coro, tmp_info);
				ivm_caller_info_free(tmp_info);
			} else {
				/* no more callee to restore, end coro */
				ivm_runtime_free(coro->runtime);
				coro->runtime = IVM_NULL;
				break;
			}
		}

ACTION_YIELD:
		ret = ivm_vmstack_pop(coro->stack);
	}

	return ret ? ret : IVM_NULL_OBJ(state);
}
