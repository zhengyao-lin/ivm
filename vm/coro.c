#include <stdlib.h>

#include "pub/const.h"
#include "std/mem.h"
#include "pub/err.h"
#include "pub/vm.h"
#include "pub/inlines.h"

#include "std/string.h"
#include "std/time.h"
#include "std/thread.h"

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
	ivm_block_stack_init(&ret->bstack);
	ivm_frame_stack_init(&ret->frame_st);
	// ret->runtime

	ivm_vmstate_addCoroSet(state, ret);
	
	ret->param = IVM_NULL;
	ret->exitv = IVM_NULL;

	ret->ref = 0;
	ret->cid = 0;
	ret->alive = IVM_FALSE;
	ret->has_native = IVM_FALSE;
	ret->active = IVM_FALSE;
	ret->spawned = IVM_FALSE;
	ret->wb = IVM_FALSE;

	return ret;
}

void
ivm_coro_free(ivm_coro_t *coro,
				ivm_vmstate_t *state)
{
	ivm_frame_stack_iterator_t fsiter;

	if (coro && !ivm_coro_decRef(coro)) {
		ivm_vmstate_removeCoroSet(state, coro);

		if (coro->alive) {
			ivm_runtime_dump(&coro->runtime, state);
		}

		IVM_FRAME_STACK_EACHPTR(&coro->frame_st, fsiter) {
			ivm_frame_dump(IVM_FRAME_STACK_ITER_GET(fsiter), state);
		}

		ivm_vmstack_dump(&coro->stack);
		ivm_block_stack_dump(&coro->bstack);
		ivm_frame_stack_dump(&coro->frame_st);

		ivm_vmstate_dumpCoro(state, coro);
	}

	return;
}

IVM_INLINE
void
_ivm_coro_setExceptionPos(ivm_vmstate_t *state,
							ivm_object_t *obj,
							ivm_instr_t *ip)
{
	ivm_long_t line = 0;
	const ivm_char_t *file = "<untraceable>";
	ivm_exception_t *exc;

	if (ip) {
		file = ivm_source_pos_getFile(ivm_instr_pos(ip));
		line = ivm_instr_lineno(ip);
	}

	if (IVM_IS_BTTYPE(obj, state, IVM_EXCEPTION_T)) {
		exc = IVM_AS(obj, ivm_exception_t);
		ivm_exception_setPos(exc, state, file, line);
	} else {
		ivm_object_setSlot(obj, state,
			IVM_VMSTATE_CONST(state, C_FILE),
			ivm_string_object_new(state, IVM_CSTR(state, file))
		);

		ivm_object_setSlot(obj, state,
			IVM_VMSTATE_CONST(state, C_LINE),
			ivm_numeric_new(state, line)
		);
	}

	return;
}

ivm_object_t *
ivm_coro_newStringException(ivm_coro_t *coro,
							ivm_vmstate_t *state,
							const ivm_char_t *msg)
{
	ivm_runtime_t *runtime = IVM_CORO_GET(coro, RUNTIME);
	ivm_instr_t *tmp_ip;
	ivm_object_t *ret;

	tmp_ip = IVM_RUNTIME_GET(runtime, IP);

	ret = ivm_exception_new(state, msg, IVM_NULL, 0);
	_ivm_coro_setExceptionPos(state, ret, tmp_ip);

	return ret;

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
}

void
ivm_coro_printException(ivm_coro_t *coro,
						ivm_vmstate_t *state,
						ivm_object_t *obj)
{
	const ivm_char_t *msg, *file;
	ivm_long_t line;
	ivm_exception_t *exc;

	if (obj && IVM_IS_BTTYPE(obj, state, IVM_EXCEPTION_T)) {
		exc = IVM_AS(obj, ivm_exception_t);

		msg = ivm_exception_getMsg_r(exc);
		file = ivm_exception_getFile_r(exc);
		line = ivm_exception_getLine(exc);
	} else {
		msg = "custom exception";
		file = "<unknown>";
		line = 0;
	}

	IVM_TRACE("coro %p exception trapped: ", (void *)coro);
	IVM_TRACE(IVM_ERROR_MSG_EXCEPTION(file, line, msg));
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

IVM_INLINE
ivm_bool_t
_ivm_coro_otherInt(ivm_vmstate_t *state,
					 ivm_coro_int_t intr)
{
	switch (intr) {
#if IVM_USE_MULTITHREAD

		case IVM_CORO_INT_THREAD_YIELD:
			// IVM_TRACE("################ thread switch\n");

			ivm_vmstate_threadYield(state);

			return IVM_TRUE;

#endif

		case IVM_CORO_INT_NONE:
			return IVM_TRUE;
		default:
			return IVM_FALSE;
	}
}

#define ivm_coro_kill(coro, state) \
	(coro)->alive = IVM_FALSE;

#include "opcode.req.h"

#define IVM_REG // register

ivm_object_t *
ivm_coro_execute_c(ivm_coro_t *coro,
					 ivm_vmstate_t *state,
					 ivm_object_t *arg,
					 ivm_bool_t get_opcode_entry)
{
	register ivm_instr_t *tmp_ip;
	register ivm_object_t **tmp_bp, **tmp_sp;

	// register ivm_frame_t *tmp_frame;
	register ivm_runtime_t *tmp_runtime;
	ivm_vmstack_t *tmp_stack;
	ivm_block_stack_t *tmp_bstack;
	ivm_frame_stack_t *tmp_frame_st;

	register ivm_context_t *tmp_context;
	register ivm_object_t **tmp_st_end;

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
	IVM_REG ivm_function_t *tmp_func = IVM_NULL;
	IVM_REG ivm_sint32_t tmp_argc;
	IVM_REG ivm_object_t **tmp_argv;

	IVM_REG ivm_instr_t *tmp_catch;
	// IVM_REG void *tmp_jump_back = IVM_NULL;
	// IVM_REG ivm_bool_t tmp_has_jump = IVM_FALSE;
	// IVM_REG ivm_cgid_t tmp_cgid;
	IVM_REG ivm_coro_t *tmp_coro;
	IVM_REG ivm_bool_t tmp_bool;
	IVM_REG ivm_coro_int_t tmp_int;

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

	ivm_uint_t tmp_dump;

	static void *opcode_entry[] = {
		#define OPCODE_GEN(o, name, arg, ...) &&OPCODE_##o,
			#include "opcode.def.h"
		#undef OPCODE_GEN
	};

	if (get_opcode_entry) {
		return (ivm_object_t *)opcode_entry;
	}

	coro->active = IVM_TRUE;
	// IVM_WBCORO(state, coro);

	if (ivm_coro_isAlive(coro)) {
		tmp_runtime = &coro->runtime;
		tmp_stack = &coro->stack;
		tmp_bstack = &coro->bstack;
		tmp_frame_st = &coro->frame_st;
		tmp_st_end = ivm_vmstack_edge(tmp_stack);

		UPDATE_STACK();

		if (arg) {
			if (coro->param) {
				if (!ivm_param_list_isLegacy(coro->param)) {
					ivm_param_list_match(coro->param, state, IVM_RUNTIME_GET(tmp_runtime, CONTEXT), 1, &arg);
				}
				coro->param = IVM_NULL;
			} else {
				STACK_PUSH(arg);
			}
		}

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

			while (!(tmp_ip = ivm_coro_popToCatch(tmp_bstack, tmp_runtime))) {
				// IVM_TRACE("kill!\n");
				tmp_dump = IVM_RUNTIME_GET(tmp_runtime, DUMP);
				ivm_runtime_dump(tmp_runtime, state);
				tmp_bp = ivm_coro_popFrame(coro);

				if (tmp_bp) {
					UPDATE_STACK();
					
					if (tmp_dump) {
						switch (tmp_dump) {
							case 2: STACK_POP();
							case 1: STACK_POP();
						}
					}

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

			tmp_dump = IVM_RUNTIME_GET(tmp_runtime, DUMP);
			ivm_runtime_dump(tmp_runtime, state);

			if (ivm_coro_popFrame(coro)) {
				UPDATE_STACK();

				if (tmp_dump) {
					switch (tmp_dump) {
						case 2: STACK_POP();
						case 1: STACK_POP();
					}
				}

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

	coro->active = IVM_FALSE;
	coro->exitv = _TMP_OBJ1;

	return _TMP_OBJ1;
}

ivm_object_t *
ivm_coro_resume(ivm_coro_t *coro,
				ivm_vmstate_t *state,
				ivm_object_t *arg)
{
	ivm_object_t *ret;

	ret = ivm_coro_execute_c(coro, state, arg, IVM_FALSE);

	if (!ret) {
		// ret = ivm_vmstate_popException(state);
		ivm_coro_printException(coro, state, ivm_vmstate_popException(state));
		ret = IVM_NONE(state);
	}

	return ret;
}

/* no trap */
IVM_INLINE
ivm_object_t *
ivm_coro_resume_n(ivm_coro_t *coro,
					ivm_vmstate_t *state,
					ivm_object_t *arg)
{
	ivm_object_t *ret;

	ret = ivm_coro_execute_c(coro, state, arg, IVM_FALSE);

	return ret;
}

/* call function from native code */

ivm_object_t *
ivm_coro_call_0(ivm_coro_t *coro,
				ivm_vmstate_t *state,
				ivm_function_object_t *func)
{
	ivm_runtime_t *runtime = IVM_CORO_GET(coro, RUNTIME);

	if (ivm_function_object_invoke(func, state, coro)) {
		/* non-native */
		return ivm_coro_resume_n(coro, state, IVM_NULL);
	}

	ivm_object_t *ret = ivm_function_callNative(
		ivm_function_object_getFunc(func), state, coro,
		ivm_function_object_getScope(func),
		IVM_FUNCTION_SET_ARG_2(0, IVM_NULL)
	);

	ivm_runtime_dump(runtime, state);
	ivm_coro_popFrame(coro);

	return ret;
}

ivm_object_t *
ivm_coro_call_1(ivm_coro_t *coro,
				ivm_vmstate_t *state,
				ivm_function_object_t *func,
				ivm_object_t *arg)
{
	ivm_runtime_t *runtime = IVM_CORO_GET(coro, RUNTIME);

	if (ivm_function_object_invoke(func, state, coro)) {
		/* non-native */
		return ivm_coro_resume_n(coro, state, arg);
	}

	ivm_vmstack_push(coro, arg);

	ivm_object_t *ret = ivm_function_callNative(
		ivm_function_object_getFunc(func), state, coro,
		ivm_function_object_getScope(func),
		IVM_FUNCTION_SET_ARG_2(1, &arg)
	);

	ivm_runtime_dump(runtime, state);
	ivm_coro_popFrame(coro);

	return ret;
}

ivm_object_t *
ivm_coro_callBase_0(ivm_coro_t *coro,
					ivm_vmstate_t *state,
					ivm_function_object_t *func,
					ivm_object_t *base)
{
	ivm_runtime_t *runtime = IVM_CORO_GET(coro, RUNTIME);

	if (ivm_function_object_invokeBase(func, state, coro, base)) {
		/* non-native */
		return ivm_coro_resume_n(coro, state, IVM_NULL);
	}

	// IVM_TRACE("call base: %p %p\n", runtime->bp, runtime->sp);

	ivm_vmstack_push(coro, base);

	ivm_object_t *ret = ivm_function_callNative(
		ivm_function_object_getFunc(func), state, coro,
		ivm_function_object_getScope(func),
		IVM_FUNCTION_SET_ARG_3(base, 0, IVM_NULL)
	);

	ivm_runtime_dump(runtime, state);
	ivm_coro_popFrame(coro);

	return ret;
}

ivm_object_t *
ivm_coro_callBase_1(ivm_coro_t *coro,
					ivm_vmstate_t *state,
					ivm_function_object_t *func,
					ivm_object_t *base,
					ivm_object_t *arg)
{
	ivm_runtime_t *runtime = IVM_CORO_GET(coro, RUNTIME);

	if (ivm_function_object_invokeBase(func, state, coro, base)) {
		/* non-native */
		return ivm_coro_resume_n(coro, state, arg);
	}

	// IVM_TRACE("call base: %p %p\n", runtime->bp, runtime->sp);

	ivm_vmstack_push(coro, arg);
	ivm_vmstack_push(coro, base);

	ivm_object_t *ret = ivm_function_callNative(
		ivm_function_object_getFunc(func), state, coro,
		ivm_function_object_getScope(func),
		IVM_FUNCTION_SET_ARG_3(base, 1, &arg)
	);

	ivm_runtime_dump(runtime, state);
	ivm_coro_popFrame(coro);

	return ret;
}

ivm_object_t *
ivm_coro_object_new(ivm_vmstate_t *state,
					ivm_coro_t *coro)
{
	ivm_coro_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), IVM_BTTYPE(state, IVM_CORO_OBJECT_T));

	ret->coro = coro;
	// IVM_TRACE("coro obj: %p\n", coro);

	if (coro) {
		ivm_coro_addRef(coro);
		ivm_vmstate_addDesLog(state, IVM_AS_OBJ(ret));
	}

	return IVM_AS_OBJ(ret);
}

void
ivm_coro_object_destructor(ivm_object_t *obj,
							 ivm_vmstate_t *state)
{
	ivm_coro_t *coro = ivm_coro_object_getCoro(obj);

	ivm_coro_free(coro, state);

	return;
}

void
ivm_coro_object_cloner(ivm_object_t *obj,
						 ivm_vmstate_t *state)
{
	ivm_coro_addRef(IVM_AS(obj, ivm_coro_object_t)->coro);
	ivm_vmstate_addDesLog(state, obj);
	return;
}

void
ivm_coro_object_traverser(ivm_object_t *obj,
						  ivm_traverser_arg_t *arg)
{
	return;
}

#if IVM_USE_MULTITHREAD

ivm_coro_thread_t *
ivm_coro_thread_new(ivm_vmstate_t *state,
					ivm_coro_t *coro)
{
	ivm_coro_thread_t *ret = ivm_vmstate_allocCThread(state);

	ret->coro = coro;
	ivm_coro_addRef(coro);

	return ret;
}

void
ivm_coro_thread_free(ivm_coro_thread_t *thread,
					 ivm_vmstate_t *state)
{
	if (thread) {
		ivm_coro_free(thread->coro, state);
		ivm_vmstate_dumpCThread(state, thread);
	}

	return;
}

#endif
