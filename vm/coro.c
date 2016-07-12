#include <stdlib.h>

#include "pub/const.h"
#include "pub/mem.h"
#include "pub/err.h"
#include "pub/vm.h"
#include "pub/inlines.h"

#include "coro.h"
#include "vmstack.h"
#include "context.h"
#include "call.h"
#include "opcode.h"

ivm_coro_t *
ivm_coro_new(ivm_vmstate_t *state)
{
	ivm_coro_t *ret = ivm_vmstate_allocCoro(state);

	ivm_vmstack_init(&ret->stack);
	ivm_frame_stack_init(&ret->frame_st);
	ret->runtime = IVM_NULL;

	return ret;
}

void
ivm_coro_free(ivm_coro_t *coro,
			  ivm_vmstate_t *state)
{
	ivm_frame_stack_iterator_t fsiter;

	if (coro) {
		IVM_FRAME_STACK_EACHPTR(&coro->frame_st, fsiter) {
			ivm_frame_free(IVM_FRAME_STACK_ITER_GET(fsiter), state);
		}

		ivm_vmstack_dump(&coro->stack);
		ivm_frame_stack_dump(&coro->frame_st);
		ivm_runtime_free(coro->runtime, state);

		ivm_vmstate_dumpCoro(state, coro);
	}

	return;
}

void
ivm_coro_setRoot(ivm_coro_t *coro,
				 ivm_vmstate_t *state,
				 ivm_function_object_t *root)
{
	coro->runtime
	= ivm_function_createRuntime(
		ivm_function_object_getFunc(root),
		state,
		ivm_function_object_getClosure(root),
		coro
	);

	return;
}

#define ivm_coro_kill(coro, state) \
	ivm_runtime_free((coro)->runtime, (state)); \
	(coro)->runtime = IVM_NULL;

#include "opcode.req.h"

ivm_object_t *
ivm_coro_start_c(ivm_coro_t *coro, ivm_vmstate_t *state,
				 ivm_function_object_t *root, ivm_bool_t get_opcode_entry)
{
	ivm_frame_t *tmp_frame;
	ivm_runtime_t *tmp_runtime;
	ivm_vmstack_t *tmp_stack;
	ivm_frame_stack_t *tmp_frame_st;

	ivm_ctchain_t *tmp_context;
	const ivm_function_t *tmp_func = IVM_NULL;

	register ivm_instr_t *tmp_ip;
	register ivm_object_t **tmp_st_end;
	register ivm_object_t **tmp_bp, **tmp_sp;

	register ivm_object_t *tmp_obj1 = IVM_NULL;
	register ivm_object_t *tmp_obj2 = IVM_NULL;
	register ivm_uniop_proc_t tmp_uni_proc;
	register ivm_binop_proc_t tmp_bin_proc;
	register ivm_ptr_t tmp_cmp_reg = 0;

	register const ivm_string_t *tmp_str;

	/*****************************
	* stack cache(support only 1 or 2 TOS cache)
	* 
	* cst = 2:
	* ----------------------
	* | stc1 | stc0 | .... |
	* ----------------------
	*    ^ stack top
	*
	* cst = 1:
	* ----------------------
	* | stc0 | .... | .... |
	* ----------------------
	*    ^ stack top 
	*    
	* cst = 0:
	* ----------------------
	* | .... | .... | .... |
	* ----------------------
	*    ^ stack top 
	*
	*****************************/

#if IVM_STACK_CACHE_N_TOS == 1
	register ivm_object_t *stc0 = IVM_NULL;
#elif IVM_STACK_CACHE_N_TOS == 2
	register ivm_object_t *stc0 = IVM_NULL,
						  *stc1 = IVM_NULL;
#endif

#if IVM_STACK_CACHE_N_TOS != 0
	register ivm_int_t cst = 0; /* cache state */
#endif

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
		= ivm_function_createRuntime(
			tmp_func, state,
			ivm_function_object_getClosure(root),
			coro
		);
	}

	if (ivm_function_isNative(tmp_func)) {
		_TMP_OBJ = ivm_function_callNative(tmp_func, state,
										   IVM_RUNTIME_GET(coro->runtime, CONTEXT),
										   IVM_FUNCTION_SET_ARG_2(0, IVM_NULL));
		if (!_TMP_OBJ) {
			_TMP_OBJ = IVM_NULL_OBJ(state);
		}

		ivm_coro_kill(coro, state);
	} else if (ivm_coro_isAsleep(coro)) {
		tmp_runtime = coro->runtime;
		tmp_stack = &coro->stack;
		tmp_frame_st = &coro->frame_st;
		tmp_st_end = ivm_vmstack_edge(tmp_stack);

		UPDATE_STACK();

		while (1) {
ACTION_INVOKE:
			tmp_ip = IVM_RUNTIME_GET(tmp_runtime, IP);
			tmp_context = IVM_RUNTIME_GET(tmp_runtime, CONTEXT);

			if (tmp_ip) {

#if IVM_DISPATCH_METHOD_DIRECT_THREAD
			
				/* for single line debug */
				// IVM_PER_INSTR_DBG(DBG_RUNTIME());

				/* jump to the first opcode */
				goto *(ivm_instr_entry(tmp_ip));

				#define OPCODE_GEN(o, name, arg, ...) \
					OPCODE_##o:                             \
						IVM_PER_INSTR_DBG(DBG_RUNTIME());   \
						__VA_ARGS__

					#include "opcode.def.h"
				#undef OPCODE_GEN

#else
				#error require a dispatch method
#endif
			}
#if 0
END_EXEC:

			if (AVAIL_STACK) {
				_TMP_OBJ = STACK_POP();
			} else {
				_TMP_OBJ = IVM_NULL_OBJ(state);
			}
#endif

ACTION_RETURN:

			IVM_PER_INSTR_DBG(DBG_RUNTIME_ACTION(RETURN, _TMP_OBJ));

			ivm_runtime_dump(tmp_runtime, state);

			tmp_frame = ivm_frame_stack_pop(tmp_frame_st, tmp_runtime);
			if (tmp_frame) {
				if (IVM_RUNTIME_GET(tmp_runtime, IS_NATIVE)) {
					goto END;
				}
				UPDATE_STACK();
				STACK_PUSH(_TMP_OBJ);
			} else {
				/* no more callee to restore, end coro */
				ivm_coro_kill(coro, state);
				break;
			}
		}

goto ACTION_YIELD_END;
ACTION_YIELD:
		IVM_PER_INSTR_DBG(DBG_RUNTIME_ACTION(YIELD, _TMP_OBJ));
ACTION_YIELD_END: ;
	}

END:

	return _TMP_OBJ;
}
