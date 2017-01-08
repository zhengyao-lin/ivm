#define INVOKE_C(argc) \
	_TMP_ARGC = (argc); \
	CHECK_STACK(_TMP_ARGC + 1); \
	_TMP_OBJ1 = STACK_BEFORE(_TMP_ARGC); \
 \
	_TMP_ARGV = STACK_CUR(); \
	STACK_CUT(_TMP_ARGC); \
 \
	if (IVM_IS_BTTYPE(_TMP_OBJ1, _STATE, IVM_FUNCTION_OBJECT_T)) { \
		_TMP_FUNC = ivm_function_object_getFunc(IVM_AS(_TMP_OBJ1, ivm_function_object_t)); \
 \
		SAVE_RUNTIME(_INSTR + 1); \
 \
		_INSTR = ivm_function_invoke( \
			_TMP_FUNC, _STATE, \
			ivm_function_object_getScope( \
				IVM_AS(_TMP_OBJ1, ivm_function_object_t) \
			), _BLOCK_STACK, _FRAME_STACK, _RUNTIME \
		); \
 \
		if (_INSTR) { \
			INVOKE_STACK(); \
			IVM_PER_INSTR_DBG(DBG_RUNTIME_ACTION(INVOKE, IVM_NULL)); \
			STACK_INC_C(_TMP_ARGC); \
			 \
			INVOKE(); \
		} else { \
			INVOKE_STACK(); \
			IVM_PER_INSTR_DBG(DBG_RUNTIME_ACTION(INVOKE, 1 /* native invoke */)); \
 \
			STACK_INC_C(_TMP_ARGC); \
			SAVE_STACK(); \
 \
			_TMP_BOOL = IVM_CORO_GET(_CORO, HAS_NATIVE); \
			IVM_CORO_SET(_CORO, HAS_NATIVE, IVM_TRUE); \
 \
			_TMP_OBJ1 = ivm_function_callNative( \
				_TMP_FUNC, _STATE, _CORO, _CONTEXT, \
				IVM_FUNCTION_SET_ARG_2(_TMP_ARGC, _TMP_ARGV) \
			); \
 \
			UPDATE_STACK(); \
 \
			IVM_CORO_SET(_CORO, HAS_NATIVE, _TMP_BOOL); \
 \
			if (_TMP_OBJ1) { \
				RETURN(); \
			} else { \
				EXCEPTION_N(); \
			} \
		} \
	} else { \
		_TMP_OBJ2 = ivm_object_getOop(_TMP_OBJ1, IVM_OOP_ID(CALL)); \
		if (_TMP_OBJ2) { \
			STACK_INC_C(_TMP_ARGC); \
			STACK_ENSURE(1); \
			STACK_MOVE_N(1, _TMP_ARGC + 1); \
			STACK_SET_BEFORE(_TMP_ARGC + 1, _TMP_OBJ2); \
			GOTO_INSTR(INVOKE_BASE); \
		} else { \
			RTM_FATAL(IVM_ERROR_MSG_UNABLE_TO_INVOKE(IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME))); \
		} \
	}

#define INVOKE_BASE_C(argc) \
	_TMP_ARGC = (argc); \
 \
	CHECK_STACK(_TMP_ARGC + 2); \
 \
	_TMP_OBJ1 = STACK_BEFORE(_TMP_ARGC); \
	_TMP_OBJ2 = STACK_BEFORE(_TMP_ARGC + 1); \
 \
	_TMP_ARGV = STACK_CUR(); \
	STACK_CUT(_TMP_ARGC); \
 \
	if (!IVM_IS_BTTYPE(_TMP_OBJ1, _STATE, IVM_FUNCTION_OBJECT_T)) { \
		do { \
			/* _TMP_OBJ2 is the base */ \
			_TMP_OBJ2 = _TMP_OBJ1; \
			_TMP_OBJ1 = ivm_object_getOop(_TMP_OBJ1, IVM_OOP_ID(CALL)); \
			RTM_ASSERT(_TMP_OBJ1, IVM_ERROR_MSG_UNABLE_TO_INVOKE(IVM_OBJECT_GET(_TMP_OBJ2, TYPE_NAME))); \
		} while (!IVM_IS_BTTYPE(_TMP_OBJ1, _STATE, IVM_FUNCTION_OBJECT_T)); \
	} \
 \
	_TMP_FUNC = ivm_function_object_getFunc(IVM_AS(_TMP_OBJ1, ivm_function_object_t)); \
 \
	SAVE_RUNTIME(_INSTR + 1); \
 \
	_INSTR = ivm_function_invokeBase( \
		_TMP_FUNC, _STATE, \
		ivm_function_object_getScope( \
			IVM_AS(_TMP_OBJ1, ivm_function_object_t) \
		), _TMP_OBJ2, _BLOCK_STACK, _FRAME_STACK, _RUNTIME \
	); \
 \
	if (_INSTR) { \
		INVOKE_STACK(); \
		IVM_PER_INSTR_DBG(DBG_RUNTIME_ACTION(INVOKE, IVM_NULL)); \
		STACK_INC_C(_TMP_ARGC); \
		INVOKE(); \
	} else { \
		INVOKE_STACK(); \
		IVM_PER_INSTR_DBG(DBG_RUNTIME_ACTION(INVOKE, 1 /* native invoke */)); \
 \
		STACK_INC_C(_TMP_ARGC); \
		SAVE_STACK(); \
 \
		_TMP_BOOL = IVM_CORO_GET(_CORO, HAS_NATIVE); \
		IVM_CORO_SET(_CORO, HAS_NATIVE, IVM_TRUE); \
 \
		_TMP_OBJ1 = ivm_function_callNative( \
			_TMP_FUNC, _STATE, _CORO, _CONTEXT, \
			IVM_FUNCTION_SET_ARG_3(_TMP_OBJ2, _TMP_ARGC, _TMP_ARGV) \
		); \
 \
		UPDATE_STACK(); \
 \
		IVM_CORO_SET(_CORO, HAS_NATIVE, _TMP_BOOL); \
 \
		if (_TMP_OBJ1) { \
			RETURN(); \
		} else { \
			EXCEPTION_N(); \
		} \
	}
