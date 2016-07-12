#ifndef _IVM_VM_DISPATCH_DIRECT_H_
#define _IVM_VM_DISPATCH_DIRECT_H_

#define _CORO (coro)
#define _RUNTIME (tmp_runtime)
#define _CONTEXT (tmp_context)
#define _STATE (state)
#define _INSTR (tmp_ip)
#define _INSTR_CACHE (&(_INSTR->cache))
#define _INSTR_CACHE_UID (_INSTR->cache.id)
#define _INSTR_CACHE_DATA (_INSTR->cache.data)

#define CLEAR_INSTR_CACHE() \
	(*_INSTR_CACHE = ivm_instr_cache_build(IVM_NULL, IVM_NULL))

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


#define NEXT_INSTR() \
	if (ivm_vmstate_checkGC(state)) {         \
		SAVE_STACK();                         \
		ivm_vmstate_doGC(state);              \
	}                                         \
	goto *(ivm_instr_entry(++tmp_ip));

#define NEXT_N_INSTR(n) \
	goto *(ivm_instr_entry(tmp_ip += (n)));

#define YIELD() tmp_ip++; SAVE_RUNTIME(tmp_ip); goto ACTION_YIELD
#define RETURN() goto ACTION_RETURN
#define INVOKE() goto ACTION_INVOKE

#define IARG() (ivm_opcode_arg_toInt(ivm_instr_arg(_INSTR)))
#define FARG() (ivm_opcode_arg_toFloat(ivm_instr_arg(_INSTR)))
#define XARG() (ivm_opcode_arg_toFunc(ivm_instr_arg(_INSTR)))
// #define SARG() (ivm_exec_getString(_EXEC, ivm_opcode_arg_toInt(ivm_instr_arg(_INSTR))))
#define PARG(type) ((type)ivm_opcode_arg_toPointer(ivm_instr_arg(_INSTR)))
#define SARG() (PARG(const ivm_string_t *))

#define STACK_TOP_NOCACHE() (*(tmp_sp - 1))

#define STACK_PUSH_NOCACHE(obj) \
	(*tmp_sp++ = (obj), \
	 (tmp_sp == tmp_st_end \
	  ? SAVE_STACK_NOPUSH(), ivm_vmstack_inc_c(_STACK, _CORO), UPDATE_STACK() \
	  : 0))

#define STACK_TOP_OVERRIDE(obj) \
	(*(tmp_sp - 1) = (obj))

#define STACK_POP_NOCACHE() (*--tmp_sp)

#define _STACK (tmp_stack)

#if IVM_STACK_CACHE_N_TOS != 0

	#define STACK_SIZE() (ivm_vmstack_offset(_STACK, tmp_sp) + cst)

#else

	#define STACK_SIZE() (ivm_vmstack_offset(_STACK, tmp_sp))

#endif

#define STACK_INC(i) (tmp_sp += (i))

#if IVM_STACK_CACHE_N_TOS == 0

	#define STC_PUSHBACK() 0

	#define STACK_TOP() (STACK_TOP_NOCACHE())

	#define STACK_POP() (STACK_POP_NOCACHE())

	#define STACK_PUSH(obj) (STACK_PUSH_NOCACHE(obj))

	#define STACK_BEFORE(i) (*(tmp_sp - 1 - (i)))

	#define STACK_CUT(i) (tmp_sp -= (i))

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
	#error unsupported stack cache number
#endif

#if IVM_STACK_CACHE_N_TOS != 0

	#define AVAIL_STACK ((((ivm_ptr_t)tmp_sp - (ivm_ptr_t)tmp_bp) / sizeof(ivm_object_t *)) + cst)

#else

	#define AVAIL_STACK (((ivm_ptr_t)tmp_sp - (ivm_ptr_t)tmp_bp) / sizeof(ivm_object_t *))

#endif

#define CHECK_STACK(req) \
	IVM_ASSERT(AVAIL_STACK >= (req), \
			   IVM_ERROR_MSG_INSUFFICIENT_STACK((req), AVAIL_STACK))

#define SAVE_RUNTIME(ip) \
	(IVM_RUNTIME_SET(_RUNTIME, IP, (ip)), SAVE_STACK())

#define SAVE_STACK() \
	(STC_PUSHBACK(),                          \
	 IVM_RUNTIME_SET(_RUNTIME, BP, tmp_bp),   \
	 IVM_RUNTIME_SET(_RUNTIME, SP, tmp_sp))

#define SAVE_STACK_NOPUSH() \
	(IVM_RUNTIME_SET(_RUNTIME, BP, tmp_bp),   \
	 IVM_RUNTIME_SET(_RUNTIME, SP, tmp_sp))

#define UPDATE_STACK() \
	(tmp_st_end = ivm_vmstack_edge(tmp_stack),   \
	 tmp_bp = IVM_RUNTIME_GET(_RUNTIME, BP),     \
	 tmp_sp = IVM_RUNTIME_GET(_RUNTIME, SP))

#define _TMP_OBJ (tmp_obj1)
#define _TMP_OBJ2 (tmp_obj2)
#define _TMP_UNI_PROC (tmp_uni_proc)
#define _TMP_BIN_PROC (tmp_bin_proc)
#define _TMP_CMP_REG (tmp_cmp_reg)
#define _TMP_STR (tmp_str)

#if IVM_USE_INLINE_CACHE

	/* return to _TMP_OBJ */
	#define GET_SLOT(a, key) \
		if (ivm_object_checkCacheValid((a), _INSTR_CACHE)) {                            \
			_TMP_OBJ = ivm_object_getCacheSlotValue(_STATE, _INSTR_CACHE);              \
		} else {                                                                        \
			_TMP_OBJ = ivm_object_getSlotValue_cc(                                      \
				(a), _STATE,                                                            \
				(key), _INSTR_CACHE                                                     \
			);                                                                          \
		}
	
	/* a[key] = b */
	#define SET_SLOT(a, key, b) \
		if (ivm_object_checkCacheValid((a), _INSTR_CACHE)) {                      \
			ivm_object_setCacheSlotValue(_STATE, _INSTR_CACHE, (b));              \
		} else {                                                                  \
			ivm_object_setSlot_cc(                                                \
				(a), _STATE, (key),                                               \
				(b), _INSTR_CACHE                                                 \
			);                                                                    \
		}

#else

	#define GET_SLOT(a, key) \
		(_TMP_OBJ = ivm_object_getSlotValue((a), _STATE, (key)))

	#define SET_SLOT(a, key, b) \
		(ivm_object_setSlot((a), _STATE, (key), (b)))

#endif

#endif
