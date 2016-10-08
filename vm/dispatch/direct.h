#ifndef _IVM_VM_DISPATCH_DIRECT_H_
#define _IVM_VM_DISPATCH_DIRECT_H_

#define _CORO (coro)
#define _RUNTIME (tmp_runtime)
#define _CONTEXT (tmp_context)
#define _STATE (state)
#define _FRAME_STACK (tmp_frame_st)
#define _INSTR (tmp_ip)

#define _BP (tmp_bp)

#define _DBG_RUNTIME_DEFAULT \
	.ip = tmp_ip,                                         \
	.bp = ivm_vmstack_offset(_STACK, tmp_bp),             \
	.sp = ivm_vmstack_offset(_STACK, tmp_sp),             \
	.cmp_reg = _TMP_CMP_REG,                              \
	.state = _STATE,                                      \
	.coro = _CORO,                                        \
	.stack = _STACK                                       \

#if IVM_STACK_CACHE_N_TOS == 0
	#define _DBG_RUNTIME_CACHE \
		.cst = 0
#elif IVM_STACK_CACHE_N_TOS == 1
	#define _DBG_RUNTIME_CACHE \
		.stc0 = stc0,               \
		.cst = cst
#elif IVM_STACK_CACHE_N_TOS == 2
	#define _DBG_RUNTIME_CACHE \
		.stc0 = stc0,               \
		.stc1 = stc1,               \
		.cst = cst
#endif

#define DBG_RUNTIME() \
	((ivm_dbg_runtime_t) {                        \
		.action = IVM_CORO_ACTION_NONE,           \
		.retval = IVM_NULL,                       \
		_DBG_RUNTIME_CACHE,                       \
		_DBG_RUNTIME_DEFAULT                      \
	})

#define DBG_RUNTIME_ACTION(a, ret) \
	((ivm_dbg_runtime_t) {                        \
		.action = IVM_CORO_ACTION_##a,            \
		.retval = (ivm_object_t *)(ret),          \
		_DBG_RUNTIME_CACHE,                       \
		_DBG_RUNTIME_DEFAULT                      \
	})

#define INC_INSTR() (++tmp_ip)
#define GOTO_CUR_INSTR() \
	if (!ivm_vmstate_checkGC(state)) {                \
		goto *(ivm_instr_entry(tmp_ip));              \
	} else {                                          \
		SAVE_STACK();                                 \
		ivm_vmstate_doGC(state);                      \
		goto *(ivm_instr_entry(tmp_ip));              \
	}                                                 \

#define NEXT_INSTR() \
	if (!ivm_vmstate_checkGC(state)) {                \
		goto *(ivm_instr_entry(++tmp_ip));            \
	} else {                                          \
		SAVE_STACK();                                 \
		ivm_vmstate_doGC(state);                      \
		goto *(ivm_instr_entry(++tmp_ip));            \
	}

#define NEXT_INSTR_NGC() goto *(ivm_instr_entry(++tmp_ip));

#define NEXT_N_INSTR(n) \
	goto *(ivm_instr_entry(tmp_ip += (n)));

#define GOTO_SET_INSTR(n) \
	goto *(ivm_instr_entry(tmp_ip = (n)));

#define GOTO_INSTR(opc) \
	goto OPCODE_##opc;

#define RTM_FATAL(...) \
	ivm_char_t __rtm_assert_buf__[            \
		IVM_DEFAULT_EXCEPTION_BUFFER_SIZE     \
	];                                        \
	IVM_SNPRINTF(                             \
		__rtm_assert_buf__,                   \
		IVM_ARRLEN(__rtm_assert_buf__),       \
		__VA_ARGS__                           \
	);                                        \
	RAISE(ivm_coro_newStringException(        \
		_CORO, _STATE, __rtm_assert_buf__     \
	))

#define RTM_ASSERT(cond, ...) \
	if (IVM_UNLIKELY(!(cond))) {  \
		RTM_FATAL(__VA_ARGS__);   \
	}

#define NOT_NONE(obj, slot) \
	RTM_ASSERT(!IVM_IS_NONE(_STATE, (obj)), IVM_ERROR_MSG_OP_SLOT_OF_NONE(slot));

#define NOT_NONE_OP(obj, op) \
	RTM_ASSERT(!IVM_IS_NONE(_STATE, (obj)), IVM_ERROR_MSG_OP_NONE(op));

#define CHECK_CGID() \
	RTM_ASSERT(ivm_vmstate_hasCGroup(_STATE, _TMP_CGID),          \
			   IVM_ERROR_MSG_CORO_GROUP_NOT_EXIST(_TMP_CGID));

#define YIELD() goto ACTION_YIELD
#define RETURN() tmp_sp = tmp_bp; goto ACTION_RETURN
#define INVOKE() goto ACTION_INVOKE

#define RAISE(obj) \
	_TMP_CATCH = ivm_runtime_popCurCatch(_RUNTIME);      \
	SAVE_RUNTIME(tmp_ip);                                \
	_TMP_OBJ1 = (obj);                                   \
	if (_TMP_CATCH) {                                    \
		STACK_PUSH(_TMP_OBJ1);                           \
		GOTO_SET_INSTR(_TMP_CATCH);                      \
	} else {                                             \
		/* no raise protection -> fall back */           \
		goto ACTION_RAISE;                               \
	}

/* exception detected */
#define EXCEPTION() \
	_TMP_CATCH = ivm_runtime_popCurCatch(_RUNTIME);      \
	_TMP_OBJ1 = ivm_vmstate_getException(_STATE);        \
	if (_TMP_CATCH) {                                    \
		STACK_PUSH(_TMP_OBJ1);                           \
		GOTO_SET_INSTR(_TMP_CATCH);                      \
	} else {                                             \
		/* no raise protection -> fall back */           \
		goto ACTION_EXCEPTION;                           \
	}

#define IARG() (ivm_opcode_arg_toInt(ivm_instr_arg(_INSTR)))
#define FARG() (ivm_opcode_arg_toFloat(ivm_instr_arg(_INSTR)))
// #define SARG() (ivm_exec_getString(_EXEC, ivm_opcode_arg_toInt(ivm_instr_arg(_INSTR))))
#define PARG(type) ((type)ivm_opcode_arg_toPointer(ivm_instr_arg(_INSTR)))
#define XARG() (ivm_opcode_arg_toFunc(ivm_instr_arg(_INSTR))) // (PARG(ivm_function_t *))
#define SARG() (PARG(const ivm_string_t *))
#define ADDR_ARG() (PARG(ivm_instr_t *))
#define SET_IARG(i) (ivm_instr_setArg(_INSTR, ivm_opcode_arg_fromInt(i)))

#define STACK_TOP_NOCACHE() (*(tmp_sp - 1))

#define STACK_PUSH_NOCACHE(obj) \
	(*tmp_sp = (obj),                       \
	 (IVM_UNLIKELY(++tmp_sp == tmp_st_end)  \
	  ? SAVE_STACK_NOPUSH(),                \
	    ivm_vmstack_inc_c(_STACK, _CORO),   \
	    UPDATE_STACK()                      \
	  : 0))

#define STACK_OVERRIDE(obj) \
	(*(tmp_sp - 1) = (obj))

#define STACK_POP_NOCACHE() (*--tmp_sp)

#define STACK_PREV_BLOCK_TOP(n) \
	(*ivm_runtime_getPrevBlockTop(_RUNTIME, tmp_sp, AVAIL_STACK, (n)))

#define _STACK (tmp_stack)

#if IVM_STACK_CACHE_N_TOS != 0

	#define STACK_SIZE() (ivm_vmstack_offset(_STACK, tmp_sp) + cst)

#else

	#define STACK_SIZE() (ivm_vmstack_offset(_STACK, tmp_sp))

#endif

#define STACK_CUR() (tmp_sp)
#define STACK_INC_C(i) (tmp_sp += (i))

#define STACK_ENSURE(i) \
	((IVM_UNLIKELY(tmp_sp + (i) >= tmp_st_end)   \
	  ? SAVE_STACK_NOPUSH(),                     \
	    ivm_vmstack_ensure(_STACK, _CORO, (i)),  \
	    UPDATE_STACK()                           \
	  : 0), tmp_sp)

#if IVM_STACK_CACHE_N_TOS == 0

	#define STC_PUSHBACK() 0

	#define STACK_TOP() STACK_TOP_NOCACHE()

	#define STACK_POP() STACK_POP_NOCACHE()

	#define STACK_PUSH(obj) STACK_PUSH_NOCACHE(obj)

	#define STACK_BEFORE(i) (*(tmp_sp - 1 - (i)))

	#define STACK_CUT(i) (tmp_sp -= (i))

	/*
		       from
		         |        sp
		-------------------
		|  a  |  b  |  c  |
		-------------------

                    sp
		-------------
		|  b  |  c  |
		-------------
	 */
	#define STACK_TRIM_HEAD(count) \
		((tmp_sp -= (count)), \
		 STD_MEMMOVE(tmp_bp, tmp_bp + (count), sizeof(*tmp_sp) * AVAIL_STACK))

	// move from tmp_bp
	#define STACK_MOVE(count) \
		((tmp_sp += (count)), \
		 STD_MEMMOVE(tmp_bp + (count), tmp_bp, sizeof(*tmp_sp) * AVAIL_STACK))

	#define STACK_SET(count) \
		(tmp_sp = tmp_bp + (count))

#elif IVM_STACK_CACHE_N_TOS == 1

	#define STC_PUSHBACK() \
		(cst ? (STACK_PUSH_NOCACHE(stc0), cst = 0) : 0)

	#define STACK_TOP() \
		(cst ? stc0 : STACK_TOP_NOCACHE())

	#define STACK_POP() \
		(cst ? ((cst = 0), stc0) : STACK_POP_NOCACHE())

	#define STACK_PUSH(obj) \
		(((cst) ? STC_PUSHBACK() : 0), (stc0 = (obj)), (cst = 1))

	#define STACK_BEFORE(i) \
		((i) < cst ? stc0 : *(tmp_sp - 1 - (i) + cst))

	#define STACK_CUT(i) \
		((i) ? (STC_PUSHBACK(), (tmp_sp -= (i))) : IVM_NULL)

#elif IVM_STACK_CACHE_N_TOS == 2
	#define _if		((
	#define _then	)?(
	#define _else	):(
	#define _end	))

	/* stack cache */
	#define STC_PUSHBACK() \
		_if (cst) _then                       \
			_if (cst == 2) _then              \
				STACK_PUSH_NOCACHE(stc0),     \
				STACK_PUSH_NOCACHE(stc1)      \
			_else                             \
				STACK_PUSH_NOCACHE(stc0)      \
			_end,                             \
			(cst = 0)                         \
		_else                                 \
			0                                 \
		_end

	#define STACK_TOP() \
		_if (cst == 2) _then                 \
			stc1                             \
		_else                                \
			_if (cst) _then                  \
				stc0                         \
			_else                            \
				STACK_TOP_NOCACHE()          \
			_end                             \
		_end

	#define STACK_POP() \
		_if (cst == 2) _then                 \
			(cst = 1),                       \
			stc1                             \
		_else                                \
			_if (cst) _then                  \
				(cst = 0),                   \
				stc0                         \
			_else                            \
				STACK_POP_NOCACHE()          \
			_end                             \
		_end

	#define STACK_PUSH(obj) \
		_if (cst == 2) _then                \
			STACK_PUSH_NOCACHE(stc0),       \
			stc0 = stc1,                    \
			stc1 = (obj)                    \
		_else                               \
		  	_if (cst) _then                 \
		  		(cst = 2),                  \
		  		(stc1 = (obj))              \
		  	_else                           \
		  		(cst = 1),                  \
		  		(stc0 = (obj))              \
		  	_end                            \
		_end

	#define STACK_BEFORE(i) \
		_if ((i) < cst) _then                                \
			_if (cst == 2) _then                             \
				_if (i) _then                                \
					stc0                                     \
				_else                                        \
					stc1                                     \
				_end                                         \
			_else /* cst == 1 -> i must be 0 */              \
				stc0                                         \
			_end                                             \
		_else                                                \
			*(tmp_sp - 1 - (i) + cst)                        \
		_end

	#define STACK_CUT(i) \
		_if (i) _then                                        \
			STC_PUSHBACK(),                                  \
			(tmp_sp -= (i))                                  \
		_else                                                \
			IVM_NULL                                         \
		_end

#else
	#error unsupported stack cache count
#endif

#if IVM_STACK_CACHE_N_TOS != 0
	#define AVAIL_STACK (IVM_PTR_DIFF(tmp_sp, tmp_bp, ivm_object_t *) + cst)
#else
	#define AVAIL_STACK IVM_PTR_DIFF(tmp_sp, tmp_bp, ivm_object_t *)
	#define HAS_STACK (tmp_sp != tmp_bp)
#endif

#define INSUF_STACK(req) \
	IVM_FATAL(IVM_ERROR_MSG_INSUFFICIENT_STACK((req), AVAIL_STACK))

#define CHECK_STACK(req) \
	if (AVAIL_STACK < (req)) {  \
		INSUF_STACK(req);       \
	}

#define SAVE_RUNTIME(ip) \
	(IVM_RUNTIME_SET(_RUNTIME, IP, (ip)), SAVE_STACK())

#define UPDATE_RUNTIME() \
	(tmp_ip = IVM_RUNTIME_GET(_RUNTIME, IP), UPDATE_STACK())

#define SAVE_STACK() \
	(STC_PUSHBACK(), IVM_RUNTIME_SET(_RUNTIME, SP, tmp_sp))

#define SAVE_STACK_NOPUSH() \
	(IVM_RUNTIME_SET(_RUNTIME, SP, tmp_sp))

#define UPDATE_STACK() \
	(tmp_st_end = ivm_vmstack_edge(tmp_stack),   \
	 tmp_bp = IVM_RUNTIME_GET(_RUNTIME, BP),     \
	 tmp_sp = IVM_RUNTIME_GET(_RUNTIME, SP))

#define SET_BP(val) (tmp_bp = (val))
#define UPDATE_BP() (tmp_bp = IVM_RUNTIME_GET(_RUNTIME, BP))

#define UPDATE_STACK_C() \
	(tmp_bp = IVM_RUNTIME_GET(_RUNTIME, BP), \
	 tmp_sp = IVM_RUNTIME_GET(_RUNTIME, SP))

#define INVOKE_STACK() (tmp_bp = tmp_sp)
#define RETURN_STACK(val) (tmp_bp = tmp_sp = (val))

#define _TMP_OBJ1 (tmp_obj1)
#define _TMP_OBJ2 (tmp_obj2)
#define _TMP_OBJ3 (tmp_obj3)
#define _TMP_OBJ4 (tmp_obj4)
#define _TMP_UNI_PROC (tmp_uni_proc)
#define _TMP_BIN_PROC (tmp_bin_proc)
#define _TMP_CMP_REG (tmp_cmp_reg)
#define _TMP_BLOCK (tmp_block)
#define _TMP_STR (tmp_str)
#define _TMP_FUNC (tmp_func)
#define _TMP_ARGC (tmp_argc)
#define _TMP_ARGV (tmp_argv)
#define _TMP_CATCH (tmp_catch)
#define _TMP_CGID (tmp_cgid)
#define _TMP_BOOL (tmp_bool)
#define _USE_REG (use_reg)

#endif
