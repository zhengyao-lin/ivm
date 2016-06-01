#include "stdlib.h"
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
	ivm_coro_t *ret = MEM_ALLOC(sizeof(*ret),
								ivm_coro_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("coroutine"));

	ret->stack = ivm_vmstack_new();
	ret->frame_st = ivm_frame_stack_new();
	ret->runtime = IVM_NULL;

	return ret;
}

void
ivm_coro_free(ivm_coro_t *coro,
			  ivm_vmstate_t *state)
{
	if (coro) {
		ivm_frame_stack_foreach(coro->frame_st, (ivm_stack_foreach_proc_t)ivm_frame_free);
		ivm_vmstack_free(coro->stack);
		ivm_frame_stack_free(coro->frame_st);
		ivm_runtime_free(coro->runtime, state);
		MEM_FREE(coro);
	}

	return;
}

#define ivm_coro_kill(coro, state) \
	ivm_runtime_free((coro)->runtime, (state)); \
	(coro)->runtime = IVM_NULL;

ivm_object_t *
ivm_coro_start(ivm_coro_t *coro, ivm_vmstate_t *state,
			   ivm_function_object_t *root)
{
	ivm_object_t *ret = IVM_NULL;
	ivm_frame_t *tmp_frame;
	ivm_runtime_t *tmp_runtime;
	ivm_vmstack_t *tmp_stack;

	ivm_exec_t *tmp_exec;
	ivm_ctchain_t *tmp_context;
	ivm_function_t *tmp_func = IVM_NULL;

	ivm_instr_t *tmp_ip, *tmp_ip_end;

	if (root) {
		/* root of sleeping coro cannot be reset */
		IVM_ASSERT(!coro->runtime, IVM_ERROR_MSG_RESET_CORO_ROOT);

		tmp_func = ivm_function_object_getFunc(root);
		coro->runtime
		= ivm_function_createRuntime(tmp_func, state,
									 ivm_function_object_getClosure(root));
	}

	if (ivm_function_isNative(tmp_func)) {
		ret = ivm_function_callNative(tmp_func, state,
									  IVM_RUNTIME_GET(coro->runtime, CONTEXT),
									  IVM_FUNCTION_SET_ARG_2(0, IVM_NULL));
		ivm_coro_kill(coro, state);
	} else if (coro->runtime) {
		tmp_runtime = coro->runtime;
		tmp_stack = coro->stack;

		while (1) {
			ret = IVM_NULL;

IVM_ACTION_INVOKE:

			tmp_exec = IVM_RUNTIME_GET(tmp_runtime, EXEC);
			tmp_context = IVM_RUNTIME_GET(tmp_runtime, CONTEXT);

			if (tmp_exec) {
				tmp_ip = IVM_RUNTIME_GET(tmp_runtime, IP);
				tmp_ip_end = ivm_exec_instrPtrEnd(tmp_exec);

				while (tmp_ip != tmp_ip_end) {
					switch (tmp_ip->proc(state, coro, tmp_stack, tmp_context,
										 tmp_exec->pool, &tmp_ip)) {
						case IVM_ACTION_INVOKE:
							goto IVM_ACTION_INVOKE;
						case IVM_ACTION_RETURN:
							IVM_RUNTIME_SET(tmp_runtime, IP, tmp_ip);
							goto ACTION_RETURN;
						case IVM_ACTION_YIELD:
							IVM_RUNTIME_SET(tmp_runtime, IP, tmp_ip);
							goto ACTION_YIELD;
						default:;
					}
					ivm_vmstate_checkGC(state);
				}

				IVM_RUNTIME_SET(tmp_runtime, IP, tmp_ip);
			}
ACTION_RETURN:
			
			tmp_frame = ivm_frame_stack_pop(coro->frame_st);

			if (tmp_frame) {
				if ((ivm_vmstack_size(tmp_stack)
					 - IVM_FRAME_GET(tmp_frame, STACK_TOP)) > 0) {
					ret = ivm_vmstack_pop(tmp_stack);
				}
		
				ivm_runtime_restore(tmp_runtime, state, coro, tmp_frame);
				ivm_frame_free(tmp_frame, state);

				if (IVM_RUNTIME_GET(tmp_runtime, IS_NATIVE)) {
					goto END;
				}

				ivm_vmstack_push(tmp_stack,
								 ret ? ret
								 	 : IVM_NULL_OBJ(state));
			} else {
				/* no more callee to restore, end coro */
				ivm_coro_kill(coro, state);
				break;
			}
		}

ACTION_YIELD:
		ret = ivm_vmstack_pop(tmp_stack);
	}

END:

	return ret ? ret : IVM_NULL_OBJ(state);
}
