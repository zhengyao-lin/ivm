#include <stdlib.h>

#include "pub/const.h"
#include "pub/mem.h"
#include "pub/err.h"
#include "pub/vm.h"

#include "inline/call.h"
#include "inline/runtime.h"
#include "inline/vm.h"
#include "inline/func.h"
#include "coro.h"
#include "vmstack.h"
#include "context.h"
#include "call.h"
#include "opcode.h"

#include "opcode.req.h"

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
	ivm_frame_stack_iterator_t fsiter;

	if (coro) {
		IVM_FRAME_STACK_EACHPTR(coro->frame_st, fsiter) {
			ivm_frame_free(IVM_FRAME_STACK_ITER_GET(fsiter), state);
		}

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

#if IVM_DISPATCH_METHOD_DIRECT_THREAD
	#include "dispatch/direct.h"
#endif

ivm_object_t *
ivm_coro_start_c(ivm_coro_t *coro, ivm_vmstate_t *state,
				 ivm_function_object_t *root, ivm_bool_t get_opcode_entry)
{
	ivm_object_t *ret = IVM_NULL;
	ivm_frame_t *tmp_frame;
	ivm_runtime_t *tmp_runtime;
	ivm_vmstack_t *tmp_stack;

	ivm_exec_t *tmp_exec;
	ivm_ctchain_t *tmp_context;
	ivm_function_t *tmp_func = IVM_NULL;

	ivm_instr_t *tmp_ip, *tmp_ip_end;
	ivm_size_t tmp_bp, tmp_sp;

#if IVM_DISPATCH_METHOD_DIRECT_THREAD
	static void *opcode_entry[] = {
		#define OPCODE_GEN(o, name, arg, ...) &&OPCODE_##o,
			#include "opcode.def.h"
		#undef OPCODE_GEN
	};

	if (get_opcode_entry) {
		return (ivm_object_t *)opcode_entry;
	}
#endif

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
			UPDATE_STACK();

ACTION_INVOKE:
			ret = IVM_NULL;
			tmp_exec = IVM_RUNTIME_GET(tmp_runtime, EXEC);
			tmp_context = IVM_RUNTIME_GET(tmp_runtime, CONTEXT);

			if (tmp_exec) {
				tmp_ip = IVM_RUNTIME_GET(tmp_runtime, IP);
				tmp_ip_end = ivm_exec_instrPtrEnd(tmp_exec);

#if IVM_DISPATCH_METHOD_DIRECT_THREAD
				
				/* jump to the first opcode */
				if (tmp_ip != tmp_ip_end) {
					// IVM_TRACE("%p\n", tmp_ip);
					goto *(tmp_ip->entry);
					#define OPCODE_GEN(o, name, arg, ...) OPCODE_##o: __VA_ARGS__
						#include "opcode.def.h"
					#undef OPCODE_GEN
				}
#else
				#error require a dispatch method
#endif

END_EXEC:

				SAVE_RUNTIME(tmp_runtime, tmp_ip);
			}
ACTION_RETURN:

			if (tmp_sp - tmp_bp > 0) {
				ret = ivm_vmstack_at(tmp_stack, --tmp_sp);
			}

			ivm_runtime_dump(tmp_runtime, state);

			tmp_frame = ivm_frame_stack_pop(coro->frame_st, coro->runtime);
			if (tmp_frame) {
				if (IVM_RUNTIME_GET(tmp_runtime, IS_NATIVE)) {
					goto END;
				}

				ivm_vmstack_pushAt(tmp_stack, IVM_RUNTIME_GET(tmp_runtime, SP_INC),
								   ret ? ret : IVM_NULL_OBJ(state));
			} else {
				/* no more callee to restore, end coro */
				ivm_coro_kill(coro, state);
				break;
			}
		}

goto ACTION_YIELD_END;
ACTION_YIELD:
		if (tmp_sp - tmp_bp > 0) {
			ret = ivm_vmstack_at(tmp_stack, IVM_RUNTIME_GET(tmp_runtime, DEC_SP));
		}
ACTION_YIELD_END: ;
	}

END:

	return ret ? ret : IVM_NULL_OBJ(state);
}
