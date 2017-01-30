#define UNIOP_HANDLER(op, op_name, def) \
	{                                                                                          \
		CHECK_STACK(1);                                                                        \
                                                                                               \
		_TMP_OBJ1 = STACK_POP();                                                               \
		_TMP_UNI_PROC = ivm_object_getUniOp(_TMP_OBJ1, IVM_UNIOP_ID(op),                       \
											IVM_OOP_ID(op), &_TMP_OBJ2);                       \
		if (_TMP_UNI_PROC || ((def) && !_TMP_OBJ2 && (_TMP_UNI_PROC = (def)))) {               \
	        _TMP_OBJ1 = _TMP_UNI_PROC(_STATE, _CORO, _TMP_OBJ1);                               \
	        if (_TMP_OBJ1) {                                                                   \
	        	STACK_PUSH(_TMP_OBJ1);                                                         \
				NEXT_INSTR();                                                                  \
			}                                                                                  \
                                                                                               \
			EXCEPTION();                                                                       \
		} else if (_TMP_OBJ2) {                                                                \
			STACK_PUSH(_TMP_OBJ1);                                                             \
			STACK_PUSH(_TMP_OBJ2);                                                             \
			SET_IARG(0);                                                                       \
			GOTO_INSTR(INVOKE_BASE);                                                           \
		} else {                                                                               \
			RTM_FATAL(IVM_ERROR_MSG_NO_UNIOP_FOR(op_name,                                      \
												 IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME)));       \
		}                                                                                      \
	}

#define BINOP_HANDLER(op, op_name, def) \
	{                                                                                          \
		CHECK_STACK(2);                                                                        \
                                                                                               \
		_TMP_OBJ2 = STACK_POP();                                                               \
		_TMP_OBJ1 = STACK_POP();                                                               \
		_TMP_BIN_PROC = ivm_object_getBinOp(_TMP_OBJ1, IVM_BINOP_ID(op), IVM_OOP_ID(op),       \
											_TMP_OBJ2, &_TMP_OBJ3);                            \
		if (_TMP_BIN_PROC || ((def) && !_TMP_OBJ3 && (_TMP_BIN_PROC = (def)))) {               \
	        _TMP_OBJ1 = _TMP_BIN_PROC(_STATE, _CORO, _TMP_OBJ1, _TMP_OBJ2);                    \
	        if (_TMP_OBJ1) {                                                                   \
	        	STACK_PUSH(_TMP_OBJ1);                                                         \
				NEXT_INSTR();                                                                  \
			}                                                                                  \
			                                                                                   \
			EXCEPTION();                                                                       \
		} else if (_TMP_OBJ3) {                                                                \
			STACK_PUSH(_TMP_OBJ1);                                                             \
			STACK_PUSH(_TMP_OBJ3);                                                             \
			STACK_PUSH(_TMP_OBJ2);                                                             \
			SET_IARG(1);                                                                       \
			GOTO_INSTR(INVOKE_BASE);                                                           \
		} else {                                                                               \
			RTM_FATAL(IVM_ERROR_MSG_NO_BINOP_FOR(IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME),         \
												 op_name,                                      \
												 IVM_OBJECT_GET(_TMP_OBJ2, TYPE_NAME)));       \
		}                                                                                      \
	}

#define TRIOP_HANDLER(op, op_name, def) \
	{                                                                                          \
		CHECK_STACK(3);                                                                        \
                                                                                               \
		_TMP_OBJ2 = STACK_POP();                                                               \
		_TMP_OBJ1 = STACK_POP();                                                               \
		_TMP_OBJ3 = STACK_POP();                                                               \
		_TMP_BIN_PROC = ivm_object_getBinOp(_TMP_OBJ1, IVM_BINOP_ID(op), IVM_OOP_ID(op),       \
											_TMP_OBJ2, &_TMP_OBJ4);                            \
		if (_TMP_BIN_PROC || ((def) && !_TMP_OBJ4 && (_TMP_BIN_PROC = (def)))) {               \
	        _TMP_OBJ1 = ((ivm_triop_proc_t)_TMP_BIN_PROC)                                      \
	        			(_STATE, _CORO, _TMP_OBJ1, _TMP_OBJ2, _TMP_OBJ3);                      \
	        if (_TMP_OBJ1) {                                                                   \
	        	STACK_PUSH(_TMP_OBJ1);                                                         \
				NEXT_INSTR();                                                                  \
			}                                                                                  \
			                                                                                   \
			EXCEPTION();                                                                       \
		} else if (_TMP_OBJ4) {                                                                \
			STACK_PUSH(_TMP_OBJ1);                                                             \
			STACK_PUSH(_TMP_OBJ4);                                                             \
			STACK_PUSH(_TMP_OBJ2);                                                             \
			STACK_PUSH(_TMP_OBJ3 ? _TMP_OBJ3 : IVM_NONE(_STATE));                              \
			/* assign value could be null */                                                   \
			SET_IARG(2);                                                                       \
			GOTO_INSTR(INVOKE_BASE);                                                           \
		} else {                                                                               \
			RTM_FATAL(IVM_ERROR_MSG_NO_BINOP_FOR(IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME),         \
												 op_name,                                      \
												 IVM_OBJECT_GET(_TMP_OBJ2, TYPE_NAME)));       \
		}                                                                                      \
	}

#define CMP_HANDLER(op, op_name, has_exc, def) \
	{                                                                                          \
		CHECK_STACK(2);                                                                        \
                                                                                               \
		_TMP_OBJ2 = STACK_POP();                                                               \
		_TMP_OBJ1 = STACK_POP();                                                               \
		_TMP_BIN_PROC = ivm_object_getBinOp(_TMP_OBJ1, IVM_BINOP_ID(op), IVM_OOP_ID(op),       \
											_TMP_OBJ2, &_TMP_OBJ3);                            \
		if (_TMP_BIN_PROC || ((def) && !_TMP_OBJ3 && (_TMP_BIN_PROC = (def)))) {               \
			_TMP_CMP_REG                                                                       \
			= (ivm_ptr_t)_TMP_BIN_PROC(_STATE, _CORO, _TMP_OBJ1, _TMP_OBJ2);                   \
			if (!(has_exc) || !ivm_vmstate_getException(_STATE)) {                             \
				STACK_PUSH(ivm_numeric_new(_STATE, _TMP_CMP_REG));                             \
				NEXT_INSTR();                                                                  \
			}                                                                                  \
			                                                                                   \
	        EXCEPTION();                                                                       \
		} else if (_TMP_OBJ3) {                                                                \
			STACK_PUSH(_TMP_OBJ1);                                                             \
			STACK_PUSH(_TMP_OBJ3);                                                             \
			STACK_PUSH(_TMP_OBJ2);                                                             \
			SET_IARG(1);                                                                       \
			GOTO_INSTR(INVOKE_BASE);                                                           \
		} else {                                                                               \
			RTM_FATAL(IVM_ERROR_MSG_NO_BINOP_FOR(IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME),         \
												 op_name,                                      \
												 IVM_OBJECT_GET(_TMP_OBJ2, TYPE_NAME)));       \
		}                                                                                      \
	}

#define CMP_HANDLER_R(op, op_name, has_exc, def) \
	{                                                                                          \
		CHECK_STACK(2);                                                                        \
                                                                                               \
		_TMP_OBJ2 = STACK_POP();                                                               \
		_TMP_OBJ1 = STACK_POP();                                                               \
		_TMP_BIN_PROC = ivm_object_getBinOp(_TMP_OBJ1, IVM_BINOP_ID(op), IVM_OOP_ID(op),       \
											_TMP_OBJ2, &_TMP_OBJ3);                            \
		if (_TMP_BIN_PROC || ((def) && !_TMP_OBJ3 && (_TMP_BIN_PROC = (def)))) {               \
			_USE_REG = IVM_TRUE;                                                               \
			_TMP_CMP_REG                                                                       \
			= (ivm_ptr_t)_TMP_BIN_PROC(_STATE, _CORO, _TMP_OBJ1, _TMP_OBJ2);                   \
			if (!(has_exc) || !ivm_vmstate_getException(_STATE)) {                             \
				NEXT_INSTR();                                                                  \
			}                                                                                  \
			                                                                                   \
	        EXCEPTION();                                                                       \
		} else if (_TMP_OBJ3) {                                                                \
			STACK_PUSH(_TMP_OBJ1);                                                             \
			STACK_PUSH(_TMP_OBJ3);                                                             \
			STACK_PUSH(_TMP_OBJ2);                                                             \
			SET_IARG(1);                                                                       \
			GOTO_INSTR(INVOKE_BASE);                                                           \
		} else {                                                                               \
			RTM_FATAL(IVM_ERROR_MSG_NO_BINOP_FOR(IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME),         \
												 op_name,                                      \
												 IVM_OBJECT_GET(_TMP_OBJ2, TYPE_NAME)));       \
		}                                                                                      \
	}
