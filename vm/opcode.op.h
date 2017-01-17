#define UNIOP_HANDLER(op, op_name, e) \
	{                                                                                          \
		CHECK_STACK(1);                                                                        \
                                                                                               \
		_TMP_OBJ1 = STACK_POP();                                                               \
		_TMP_OBJ2 = ivm_object_getOop(_TMP_OBJ1, IVM_OOP_ID(op));                              \
		if (!_TMP_OBJ2) {                                                                      \
			e;                                                                                 \
	                                                                                           \
			_TMP_UNI_PROC = IVM_OBJECT_GET_UNIOP_PROC(_TMP_OBJ1, op);                          \
	                                                                                           \
			if (_TMP_UNI_PROC) {                                                               \
		        _TMP_OBJ1 = _TMP_UNI_PROC(_STATE, _CORO, _TMP_OBJ1);                           \
		        if (_TMP_OBJ1) {                                                               \
		        	STACK_PUSH(_TMP_OBJ1);                                                     \
					NEXT_INSTR();                                                              \
				}                                                                              \
	                                                                                           \
				EXCEPTION();                                                                   \
			} else {                                                                           \
				RTM_FATAL(IVM_ERROR_MSG_NO_UNIOP_FOR(op_name,                                  \
													 IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME)));   \
			}                                                                                  \
		} else {                                                                               \
			STACK_PUSH(_TMP_OBJ1);                                                             \
			STACK_PUSH(_TMP_OBJ2);                                                             \
			SET_IARG(0);                                                                       \
			GOTO_INSTR(INVOKE_BASE);                                                           \
		}                                                                                      \
	}

#define BINOP_HANDLER(op, op_name, e) \
	{                                                                                          \
		CHECK_STACK(2);                                                                        \
                                                                                               \
		_TMP_OBJ2 = STACK_POP();                                                               \
		_TMP_OBJ1 = STACK_POP();                                                               \
		_TMP_OBJ3 = ivm_object_getOop(_TMP_OBJ1, IVM_OOP_ID(op));                              \
		if (!_TMP_OBJ3) {                                                                      \
			e;                                                                                 \
			                                                                                   \
			_TMP_BIN_PROC = IVM_OBJECT_GET_BINOP_PROC(_TMP_OBJ1, op, _TMP_OBJ2);               \
	                                                                                           \
			if (_TMP_BIN_PROC) {                                                               \
		        _TMP_OBJ1 = _TMP_BIN_PROC(_STATE, _CORO, _TMP_OBJ1, _TMP_OBJ2);                \
		        if (_TMP_OBJ1) {                                                               \
		        	STACK_PUSH(_TMP_OBJ1);                                                     \
					NEXT_INSTR();                                                              \
				}                                                                              \
				                                                                               \
				EXCEPTION();                                                                   \
			} else {                                                                           \
				RTM_FATAL(IVM_ERROR_MSG_NO_BINOP_FOR(IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME),     \
													 op_name,                                  \
													 IVM_OBJECT_GET(_TMP_OBJ2, TYPE_NAME)));   \
			}                                                                                  \
		} else {                                                                               \
			STACK_PUSH(_TMP_OBJ1);                                                             \
			STACK_PUSH(_TMP_OBJ3);                                                             \
			STACK_PUSH(_TMP_OBJ2);                                                             \
			SET_IARG(1);                                                                       \
			GOTO_INSTR(INVOKE_BASE);                                                           \
		}                                                                                      \
	}

#define TRIOP_HANDLER(op, op_name, e) \
	{                                                                                          \
		CHECK_STACK(3);                                                                        \
                                                                                               \
		_TMP_OBJ2 = STACK_POP();                                                               \
		_TMP_OBJ1 = STACK_POP();                                                               \
		_TMP_OBJ3 = STACK_POP();                                                               \
		_TMP_OBJ4 = ivm_object_getOop(_TMP_OBJ1, IVM_OOP_ID(op));                              \
		if (!_TMP_OBJ4) {                                                                      \
			e;                                                                                 \
			                                                                                   \
			_TMP_BIN_PROC = IVM_OBJECT_GET_BINOP_PROC(_TMP_OBJ1, op, _TMP_OBJ2);               \
	                                                                                           \
	        if (_TMP_BIN_PROC) {                                                               \
		        _TMP_OBJ1 = ((ivm_triop_proc_t)_TMP_BIN_PROC)                                  \
		        			(_STATE, _CORO, _TMP_OBJ1, _TMP_OBJ2, _TMP_OBJ3);                  \
		        if (_TMP_OBJ1) {                                                               \
		        	STACK_PUSH(_TMP_OBJ1);                                                     \
					NEXT_INSTR();                                                              \
				}                                                                              \
				                                                                               \
				EXCEPTION();                                                                   \
			} else {                                                                           \
				RTM_FATAL(IVM_ERROR_MSG_NO_BINOP_FOR(IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME),     \
													 op_name,                                  \
													 IVM_OBJECT_GET(_TMP_OBJ2, TYPE_NAME)));   \
			}                                                                                  \
		} else {                                                                               \
			STACK_PUSH(_TMP_OBJ1);                                                             \
			STACK_PUSH(_TMP_OBJ4);                                                             \
			STACK_PUSH(_TMP_OBJ2);                                                             \
			STACK_PUSH(_TMP_OBJ3 ? _TMP_OBJ3 : IVM_NONE(_STATE));                              \
			/* assign value could be null */                                                   \
			SET_IARG(2);                                                                       \
			GOTO_INSTR(INVOKE_BASE);                                                           \
		}                                                                                      \
	}

#define CMP_HANDLER(op, op_name, e, has_exc) \
	{                                                                                          \
		CHECK_STACK(2);                                                                        \
                                                                                               \
		_TMP_OBJ2 = STACK_POP();                                                               \
		_TMP_OBJ1 = STACK_POP();                                                               \
		_TMP_OBJ4 = ivm_object_getOop(_TMP_OBJ1, IVM_OOP_ID(op));                              \
		if (!_TMP_OBJ4) {                                                                      \
			e;                                                                                 \
			_TMP_BIN_PROC = IVM_OBJECT_GET_BINOP_PROC(_TMP_OBJ1, op, _TMP_OBJ2);               \
	                                                                                           \
			if (_TMP_BIN_PROC) {                                                               \
				_TMP_CMP_REG                                                                   \
				= (ivm_ptr_t)_TMP_BIN_PROC(_STATE, _CORO, _TMP_OBJ1, _TMP_OBJ2);               \
				if (!(has_exc) || !ivm_vmstate_getException(_STATE)) {                         \
					STACK_PUSH(ivm_numeric_new(_STATE, _TMP_CMP_REG));                         \
					NEXT_INSTR();                                                              \
				}                                                                              \
				                                                                               \
		        EXCEPTION();                                                                   \
		    } else {                                                                           \
		    	RTM_FATAL(IVM_ERROR_MSG_NO_BINOP_FOR(IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME),     \
													 op_name,                                  \
													 IVM_OBJECT_GET(_TMP_OBJ2, TYPE_NAME)));   \
		    }                                                                                  \
		} else {                                                                               \
			STACK_PUSH(_TMP_OBJ1);                                                             \
			STACK_PUSH(_TMP_OBJ4);                                                             \
			STACK_PUSH(_TMP_OBJ2);                                                             \
			SET_IARG(1);                                                                       \
			GOTO_INSTR(INVOKE_BASE);                                                           \
		}                                                                                      \
	}

#define CMP_HANDLER_R(op, op_name, e, has_exc) \
	{                                                                                          \
		CHECK_STACK(2);                                                                        \
                                                                                               \
		_TMP_OBJ2 = STACK_POP();                                                               \
		_TMP_OBJ1 = STACK_POP();                                                               \
		_TMP_OBJ4 = ivm_object_getOop(_TMP_OBJ1, IVM_OOP_ID(op));                              \
		if (!_TMP_OBJ4) {                                                                      \
			_USE_REG = IVM_TRUE;                                                               \
			e;                                                                                 \
			_TMP_BIN_PROC = IVM_OBJECT_GET_BINOP_PROC(_TMP_OBJ1, op, _TMP_OBJ2);               \
	                                                                                           \
			if (_TMP_BIN_PROC) {                                                               \
		        _TMP_CMP_REG                                                                   \
		        = (ivm_ptr_t)_TMP_BIN_PROC(_STATE, _CORO, _TMP_OBJ1, _TMP_OBJ2);               \
		        if (!(has_exc) || !ivm_vmstate_getException(_STATE)) {                         \
		        	NEXT_INSTR();                                                              \
				}                                                                              \
				                                                                               \
				EXCEPTION();                                                                   \
			} else {                                                                           \
				RTM_FATAL(IVM_ERROR_MSG_NO_BINOP_FOR(IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME),     \
													 op_name,                                  \
													 IVM_OBJECT_GET(_TMP_OBJ2, TYPE_NAME)));   \
			}                                                                                  \
		} else {                                                                               \
			STACK_PUSH(_TMP_OBJ1);                                                             \
			STACK_PUSH(_TMP_OBJ4);                                                             \
			STACK_PUSH(_TMP_OBJ2);                                                             \
			SET_IARG(1);                                                                       \
			GOTO_INSTR(INVOKE_BASE);                                                           \
		}                                                                                      \
	}
