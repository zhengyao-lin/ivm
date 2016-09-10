#include <stdlib.h>

#include "pub/const.h"
#include "std/mem.h"
#include "pub/err.h"
#include "pub/vm.h"
#include "pub/inlines.h"

#include "std/string.h"

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
	// ret->runtime
	ret->alive = IVM_FALSE;
	ret->has_native = IVM_FALSE;

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

		ivm_vmstate_dumpCoro(state, coro);
	}

	return;
}

ivm_object_t *
ivm_coro_newException_s(ivm_coro_t *coro,
						ivm_vmstate_t *state,
						const ivm_char_t *msg)
{
	ivm_object_t *ret = ivm_object_new(state);
	ivm_runtime_t *runtime = IVM_CORO_GET(coro, RUNTIME);
	ivm_instr_t *tmp_ip;
	ivm_long_t line = -1;
	const ivm_char_t *tmp_file = "<untraceable>";

	tmp_ip = IVM_RUNTIME_GET(runtime, IP);

	ivm_object_setSlot(ret, state,
		IVM_VMSTATE_CONST(state, C_MSG),
		ivm_string_object_new(state, IVM_CSTR(state, msg))
	);

	if (tmp_ip) {
		tmp_file = ivm_source_pos_getFile(ivm_instr_pos(tmp_ip));
		line = ivm_instr_lineno(tmp_ip);
	}

	ivm_object_setSlot(ret, state,
		IVM_VMSTATE_CONST(state, C_FILE),
		ivm_string_object_new(state, IVM_CSTR(state, tmp_file))
	);

	ivm_object_setSlot(ret, state,
		IVM_VMSTATE_CONST(state, C_LINE),
		ivm_numeric_new(state, line)
	);

#if 0
	ivm_frame_stack_t *frame_st = IVM_CORO_GET(coro, FRAME_STACK);
	ivm_frame_stack_iterator_t fiter;
	ivm_frame_t *frame;

	IVM_TRACE("trace:\n");

#define PRINT_POS() \
	if (tmp_ip) {                                                                       \
		IVM_TRACE(IVM_TAB "%s: line %d: %s\n",                                          \
				  ivm_source_pos_getFile(ivm_instr_pos(tmp_ip)),                        \
				  ivm_instr_lineno(tmp_ip),                                             \
				  ivm_opcode_table_getName(ivm_instr_opcode(tmp_ip)));                  \
	}

	tmp_ip = IVM_RUNTIME_GET(runtime, IP);
	PRINT_POS();

	IVM_FRAME_STACK_EACHPTR(frame_st, fiter) {
		frame = IVM_FRAME_STACK_ITER_GET(fiter);
		tmp_ip = IVM_FRAME_GET(frame, IP);
		PRINT_POS();
	}
#endif

	return ret;
}

void
ivm_coro_printException(ivm_coro_t *coro,
						ivm_vmstate_t *state,
						ivm_object_t *except)
{
	ivm_object_t *msg_obj, *file_obj, *line_obj;
	const ivm_char_t *msg = "custom exception",
					 *file = "<unknown file>";
	ivm_long_t line = -1;

	if (except) {
		msg_obj = ivm_object_getSlot(
			except, state,
			IVM_VMSTATE_CONST(state, C_MSG)
		);

		file_obj = ivm_object_getSlot(
			except, state,
			IVM_VMSTATE_CONST(state, C_FILE)
		);

		line_obj = ivm_object_getSlot(
			except, state,
			IVM_VMSTATE_CONST(state, C_LINE)
		);

		if (msg_obj && IVM_IS_TYPE(msg_obj, IVM_STRING_OBJECT_T)) {
			msg = ivm_string_trimHead(ivm_string_object_getValue(msg_obj));
		}

		if (file_obj && IVM_IS_TYPE(msg_obj, IVM_STRING_OBJECT_T)) {
			file = ivm_string_trimHead(ivm_string_object_getValue(file_obj));
		}

		if (line_obj && IVM_IS_TYPE(line_obj, IVM_NUMERIC_T)) {
			line = ivm_numeric_getValue(line_obj);
		}
	} else {
		msg = "unknown exception";
	}

	IVM_TRACE(IVM_ERROR_MSG_CORO_EXCEPTION(coro, file, line, msg));
	IVM_TRACE("\n");

	return;
}

void
ivm_coro_setRoot(ivm_coro_t *coro,
				 ivm_vmstate_t *state,
				 ivm_function_object_t *root)
{
	const ivm_function_t *tmp_func = ivm_function_object_getFunc(root);

	IVM_ASSERT(!ivm_coro_isAlive(coro), IVM_ERROR_MSG_RESET_CORO_ROOT);

	// IVM_TRACE("init: %d\n", ivm_function_getMaxStack(tmp_func));
	// ivm_vmstack_inc_c(&coro->stack, coro, ivm_function_getMaxStack(tmp_func));

	IVM_ASSERT(ivm_function_createRuntime( /*  start ip not null*/
		tmp_func, state,
		ivm_function_object_getScope(root),
		coro
	), IVM_ERROR_MSG_CORO_NATIVE_ROOT);

	coro->alive = IVM_TRUE;

	return;
}

#define ivm_coro_kill(coro, state) \
	(coro)->alive = IVM_FALSE;

#include "opcode.req.h"

#define IVM_REG // register

ivm_object_t *
ivm_coro_start_c(ivm_coro_t *coro, ivm_vmstate_t *state,
				 ivm_function_object_t *root, ivm_bool_t get_opcode_entry)
{
	register ivm_instr_t *tmp_ip;
	register ivm_object_t **tmp_bp, **tmp_sp;

	// register ivm_frame_t *tmp_frame;
	register ivm_runtime_t *tmp_runtime;
	ivm_vmstack_t *tmp_stack;
	ivm_frame_stack_t *tmp_frame_st;

	register ivm_context_t *tmp_context;
	IVM_REG ivm_object_t **tmp_st_end;

	IVM_REG ivm_object_t *tmp_obj1 = IVM_NULL;
	IVM_REG ivm_object_t *tmp_obj2 = IVM_NULL;
	IVM_REG ivm_object_t *tmp_obj3 = IVM_NULL;
	IVM_REG ivm_object_t *tmp_obj4 = IVM_NULL;
	IVM_REG ivm_uniop_proc_t tmp_uni_proc;
	IVM_REG ivm_binop_proc_t tmp_bin_proc;
	IVM_REG ivm_ptr_t tmp_cmp_reg = 0;
	// IVM_REG ivm_block_t tmp_block = IVM_NULL;

	IVM_REG const ivm_string_t *tmp_str;
	// IVM_REG ivm_context_t *tmp_ctx;
	IVM_REG const ivm_function_t *tmp_func = IVM_NULL;
	IVM_REG ivm_sint32_t tmp_argc;
	IVM_REG ivm_object_t **tmp_argv;

	IVM_REG ivm_instr_t *tmp_catch;
	// IVM_REG void *tmp_jump_back = IVM_NULL;
	// IVM_REG ivm_bool_t tmp_has_jump = IVM_FALSE;
	IVM_REG ivm_cgid_t tmp_cgid;
	IVM_REG ivm_bool_t tmp_bool;

	IVM_REG ivm_bool_t use_reg = IVM_FALSE;

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
	IVM_REG ivm_object_t *stc0 = IVM_NULL;
#elif IVM_STACK_CACHE_N_TOS == 2
	IVM_REG ivm_object_t *stc0 = IVM_NULL,
						  *stc1 = IVM_NULL;
#endif

#if IVM_STACK_CACHE_N_TOS != 0
	IVM_REG ivm_int_t cst = 0; /* cache state */
#endif

	static void *opcode_entry[] = {
		#define OPCODE_GEN(o, name, arg, ...) &&OPCODE_##o,
			#include "opcode.def.h"
		#undef OPCODE_GEN
	};

	if (get_opcode_entry) {
		return (ivm_object_t *)opcode_entry;
	}

	if (root) {
		/* root of sleeping coro cannot be reset */
		ivm_coro_setRoot(coro, state, root);
	}

	/* if (ivm_function_isNative(tmp_func)) {
		_TMP_OBJ1 = ivm_function_callNative(tmp_func, state,
											IVM_RUNTIME_GET(&coro->runtime, CONTEXT),
											IVM_FUNCTION_SET_ARG_2(0, IVM_NULL));
		if (!_TMP_OBJ1) {
			_TMP_OBJ1 = IVM_NONE(state);
		}

		ivm_coro_kill(coro, state);
	} else */

	if (ivm_coro_isAlive(coro)) {
		tmp_runtime = &coro->runtime;
		tmp_stack = &coro->stack;
		tmp_frame_st = &coro->frame_st;
		tmp_st_end = ivm_vmstack_edge(tmp_stack);

		UPDATE_STACK();

		while (1) {
			tmp_ip = IVM_RUNTIME_GET(tmp_runtime, IP);
			
ACTION_INVOKE:
ACTION_RAISE_NEXT:
			tmp_context = IVM_RUNTIME_GET(tmp_runtime, CONTEXT);

#if IVM_DISPATCH_METHOD_DIRECT_THREAD
			
			/* for single line debug */
			// IVM_PER_INSTR_DBG(DBG_RUNTIME());

			/* jump to the first opcode */
			goto *(ivm_instr_entry(tmp_ip));

			#define OPCODE_GEN(o, name, arg, st_inc, ...) \
				OPCODE_##o:                               \
					IVM_PER_INSTR_DBG(DBG_RUNTIME());     \
					__VA_ARGS__

				#include "opcode.def.h"
			#undef OPCODE_GEN

#else
			#error require a dispatch method
#endif

#if 0
END_EXEC:

			if (AVAIL_STACK) {
				_TMP_OBJ1 = STACK_POP();
			} else {
				_TMP_OBJ1 = IVM_NONE(state);
			}
#endif
			IVM_ASSERT(!tmp_ip, "impossible");
			_TMP_OBJ1 = IVM_NONE(state);
			goto ACTION_RETURN;
			
ACTION_RAISE:
			ivm_vmstate_setException(state, _TMP_OBJ1);
ACTION_EXCEPTION:
			IVM_PER_INSTR_DBG(DBG_RUNTIME_ACTION(EXCEPTION, _TMP_OBJ1));

			SAVE_STACK();

			while (!(tmp_ip = ivm_runtime_popToCatch(_RUNTIME))) {
				ivm_runtime_dump(tmp_runtime, state);
				tmp_bp = ivm_frame_stack_pop(tmp_frame_st, tmp_runtime);
				if (tmp_bp) {
					if (IVM_RUNTIME_GET(tmp_runtime, IS_NATIVE)) {
						_TMP_OBJ1 = IVM_NULL;
						goto END;
					}
				} else {
					ivm_coro_kill(coro, state);
					_TMP_OBJ1 = IVM_NULL;
					goto END;
				}
			}
			// find a frame with raise protection

			UPDATE_STACK();

			ivm_vmstate_popException(state);
			// push raised object
			STACK_PUSH(_TMP_OBJ1 ? _TMP_OBJ1 : IVM_NONE(state));
			
			goto ACTION_RAISE_NEXT;

ACTION_RETURN:

			IVM_PER_INSTR_DBG(DBG_RUNTIME_ACTION(RETURN, _TMP_OBJ1));

			ivm_runtime_dump(tmp_runtime, state);
			tmp_bp = ivm_frame_stack_pop(tmp_frame_st, tmp_runtime);
			
			if (tmp_bp) {
				if (IVM_RUNTIME_GET(tmp_runtime, IS_NATIVE)) {
					goto END;
				}
				STACK_PUSH(_TMP_OBJ1);
			} else {
				/* no more callee to restore, end coro */
				ivm_coro_kill(coro, state);
				break;
			}
		}
	}

goto END;
ACTION_YIELD:
	IVM_PER_INSTR_DBG(DBG_RUNTIME_ACTION(YIELD, _TMP_OBJ1));

END:
	return _TMP_OBJ1;
}

ivm_bool_t
ivm_cgroup_switchCoro(ivm_cgroup_t *group)
{
	ivm_coro_list_iterator_t i, end;
	ivm_coro_list_t *list = &group->coros;

	for (i = IVM_CORO_LIST_ITER_AT(list, group->cur + 1),
		 end = IVM_CORO_LIST_ITER_END(list);
		 i != end; i++) {
		if (ivm_coro_isAlive(IVM_CORO_LIST_ITER_GET(i))) {
			group->cur = IVM_CORO_LIST_ITER_INDEX(list, i);
			return IVM_TRUE;
		}
	}

	for (end = i,
		 i = IVM_CORO_LIST_ITER_BEGIN(list);
		 i != end; i++) {
		if (ivm_coro_isAlive(IVM_CORO_LIST_ITER_GET(i))) {
			group->cur = IVM_CORO_LIST_ITER_INDEX(list, i);
			return IVM_TRUE;
		}
	}

	return IVM_FALSE;
}

void
ivm_cgroup_travAndCompact(ivm_cgroup_t *group,
						  ivm_traverser_arg_t *arg)
{
	ivm_size_t ocur = group->cur,
			   ncur = 0, i = 0;
	ivm_coro_list_iterator_t cur_iter, citer;
	ivm_coro_t *tmp_coro;
	ivm_bool_t cur_set = IVM_FALSE;

	cur_iter = IVM_CORO_LIST_ITER_BEGIN(&group->coros);
	IVM_CORO_LIST_EACHPTR(&group->coros, citer) {
		tmp_coro = IVM_CORO_LIST_ITER_GET(citer);
		
		if (ivm_coro_isAlive(tmp_coro)) {
			arg->trav_coro(tmp_coro, arg);
			IVM_CORO_LIST_ITER_SET(cur_iter, tmp_coro);
			if (i == ocur) {
				cur_set = IVM_TRUE;
				group->cur = ncur;
			}
			cur_iter++;
			ncur++;
		} else {
			ivm_coro_free(tmp_coro, arg->state);
		}

		i++;
	}
	ivm_coro_list_setSize(&group->coros, ncur);

	IVM_ASSERT(cur_set || !ocur, "impossible");

	return;
}
