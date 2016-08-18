OPCODE_GEN(NOP, "nop", N, 0, {
	NEXT_INSTR();
})

OPCODE_GEN(NEW_NIL, "new_nil", N, 1, {
	STACK_PUSH(IVM_NULL);
	NEXT_INSTR();
})

OPCODE_GEN(NEW_NULL, "new_null", N, 1, {
	STACK_PUSH(IVM_NULL_OBJ(_STATE));
	NEXT_INSTR();
})

OPCODE_GEN(NEW_OBJ, "new_obj", N, 1, {
	STACK_PUSH(ivm_object_new(_STATE));
	NEXT_INSTR();
})

OPCODE_GEN(NEW_OBJ_T, "new_obj_t", I, 1, {
	_TMP_OBJ1 = ivm_object_new_c(_STATE, IARG());
	STACK_PUSH(_TMP_OBJ1);
	NEXT_INSTR();
})

OPCODE_GEN(NEW_NUM_I, "new_num_i", I, 1, {
	STACK_PUSH(ivm_numeric_new(_STATE, IARG()));
	NEXT_INSTR();
})

OPCODE_GEN(NEW_NUM_F, "new_num_f", F, 1, {
	STACK_PUSH(ivm_numeric_new(_STATE, FARG()));
	NEXT_INSTR();
})

OPCODE_GEN(NEW_STR, "new_str", S, 1, {
	STACK_PUSH(ivm_string_object_new(_STATE, SARG()));
	NEXT_INSTR();
})

OPCODE_GEN(NEW_FUNC, "new_func", X, 1, {
	STACK_PUSH(
		ivm_function_object_new(
			_STATE, _CONTEXT,
			ivm_vmstate_getFunc(_STATE, XARG())
		)
	);
	NEXT_INSTR();
})

OPCODE_GEN(NEW_LIST, "new_list", I, 1, {
	_TMP_ARGC = IARG();

	CHECK_STACK(_TMP_ARGC);

	_TMP_ARGV = STACK_CUT(_TMP_ARGC);
	_TMP_OBJ1 = ivm_list_object_new_c(_STATE, _TMP_ARGV, _TMP_ARGC);
	STACK_PUSH(_TMP_OBJ1);

	NEXT_INSTR();
})

OPCODE_GEN(CLONE, "clone", N, 1, {
	CHECK_STACK(1);
	STACK_OVERRIDE(ivm_object_clone(STACK_TOP(), _STATE));
	NEXT_INSTR();
})

/* unary operations */
OPCODE_GEN(NOT, "not", N, 0, UNIOP_HANDLER(NOT, "!", {
	STACK_PUSH(ivm_numeric_new(_STATE, !ivm_object_toBool(_TMP_OBJ1, _STATE)));
	NEXT_INSTR();
}))
OPCODE_GEN(NEG, "neg", N, 0, UNIOP_HANDLER(NEG, "-", 0))
OPCODE_GEN(POS, "pos", N, 0, UNIOP_HANDLER(POS, "+", 0))

/* binary operations */
OPCODE_GEN(ADD, "add", N, -1, BINOP_HANDLER(ADD, "+", 0, 2, IVM_NULL))
OPCODE_GEN(SUB, "sub", N, -1, BINOP_HANDLER(SUB, "-", 0, 2, IVM_NULL))
OPCODE_GEN(MUL, "mul", N, -1, BINOP_HANDLER(MUL, "*", 0, 2, IVM_NULL))
OPCODE_GEN(DIV, "div", N, -1, BINOP_HANDLER(DIV, "/", 0, 2, IVM_NULL))
OPCODE_GEN(MOD, "mod", N, -1, BINOP_HANDLER(MOD, "%", 0, 2, IVM_NULL))

OPCODE_GEN(AND, "and", N, -1, BINOP_HANDLER(AND, "&", 0, 2, IVM_NULL))
OPCODE_GEN(EOR, "eor", N, -1, BINOP_HANDLER(EOR, "^", 0, 2, IVM_NULL))
OPCODE_GEN(IOR, "ior", N, -1, BINOP_HANDLER(IOR, "|", 0, 2, IVM_NULL))

OPCODE_GEN(IDX, "idx", N, -1, BINOP_HANDLER(IDX, "[]", 0, 2, IVM_NULL))
OPCODE_GEN(IDX_ASSIGN, "idx_assign", N, -2, BINOP_HANDLER(IDX, "[]", 0, 3, STACK_POP()))

OPCODE_GEN(SHL, "shl", N, -1, BINOP_HANDLER(SHL, "<<", 0, 2, IVM_NULL))
OPCODE_GEN(SHAR, "shar", N, -1, BINOP_HANDLER(SHAR, ">>", 0, 2, IVM_NULL))
OPCODE_GEN(SHLR, "shlr", N, -1, BINOP_HANDLER(SHLR, ">>>", 0, 2, IVM_NULL))

OPCODE_GEN(NE, "ne", N, -1, CMP_HANDLER(NE, "!=",
	{
		if (IVM_TYPE_OF(_TMP_OBJ1) != IVM_TYPE_OF(_TMP_OBJ2)) {
			STACK_PUSH(ivm_numeric_new(_STATE, IVM_TRUE));
			NEXT_INSTR();
		}
	}
))

OPCODE_GEN(EQ, "eq", N, -1, CMP_HANDLER(EQ, "==",
	{
		if (IVM_TYPE_OF(_TMP_OBJ1) != IVM_TYPE_OF(_TMP_OBJ2)) {
			STACK_PUSH(ivm_numeric_new(_STATE, IVM_FALSE));
			NEXT_INSTR();
		}
	}
))

OPCODE_GEN(GT, "gt", N, -1, CMP_HANDLER(GT, ">", 0))
OPCODE_GEN(GE, "ge", N, -1, CMP_HANDLER(GE, ">=", 0))
OPCODE_GEN(LT, "lt", N, -1, CMP_HANDLER(LT, "<", 0))
OPCODE_GEN(LE, "le", N, -1, CMP_HANDLER(LE, "<=", 0))

OPCODE_GEN(NE_R, "ne_r", N, -2, CMP_HANDLER_R(NE, "!=",
	{
		if (IVM_TYPE_OF(_TMP_OBJ1) != IVM_TYPE_OF(_TMP_OBJ2)) {
			_TMP_CMP_REG = IVM_TRUE;
			NEXT_INSTR();
		}
	}
))

OPCODE_GEN(EQ_R, "eq_r", N, -2, CMP_HANDLER_R(EQ, "==",
	{
		if (IVM_TYPE_OF(_TMP_OBJ1) != IVM_TYPE_OF(_TMP_OBJ2)) {
			_TMP_CMP_REG = IVM_FALSE;
			NEXT_INSTR();
		}
	}
))

OPCODE_GEN(GT_R, "gt_r", N, -2, CMP_HANDLER_R(GT, ">", 0))
OPCODE_GEN(GE_R, "ge_r", N, -2, CMP_HANDLER_R(GE, ">=", 0))

OPCODE_GEN(LT_R, "lt_r", N, -2, CMP_HANDLER_R(LT, "<", 0))
OPCODE_GEN(LE_R, "le_r", N, -2, CMP_HANDLER_R(LE, "<=", 0))

#if 0

OPCODE_GEN(LT_R, "lt_r", N, -2, CMP_HANDLER_R(0, LT,
	_TMP_CMP_REG = (ivm_ptr_t)_TMP_OBJ1 < 0;
))

OPCODE_GEN(LE_R, "le_r", N, -2, CMP_HANDLER_R(0, LE,
	_TMP_CMP_REG = (ivm_ptr_t)_TMP_OBJ1 <= 0;
	NEXT_INSTR();
))

OPCODE_GEN(GT_R, "gt_r", N, -2, CMP_HANDLER_R(0, GT,
	_TMP_CMP_REG = (ivm_ptr_t)_TMP_OBJ1 > 0;
	NEXT_INSTR();
))

OPCODE_GEN(GE_R, "ge_r", N, -2, CMP_HANDLER_R(0, GE,
	_TMP_CMP_REG = (ivm_ptr_t)_TMP_OBJ1 >= 0;
	NEXT_INSTR();
))

OPCODE_GEN(EQ_R, "eq_r", N, -2, CMP_HANDLER_R(
	{
		if (IVM_TYPE_OF(_TMP_OBJ1) != IVM_TYPE_OF(_TMP_OBJ2)) {
			_TMP_CMP_REG = IVM_FALSE;
			NEXT_INSTR();
		}
	}, EQ,
	_TMP_CMP_REG = (ivm_ptr_t)_TMP_OBJ1 == 0;
	NEXT_INSTR();
))

OPCODE_GEN(NE_R, "ne_r", N, -2, CMP_HANDLER_R(
	{
		if (IVM_TYPE_OF(_TMP_OBJ1) != IVM_TYPE_OF(_TMP_OBJ2)) {
			_TMP_CMP_REG = IVM_TRUE;
			NEXT_INSTR();
		}
	}, NE,
	_TMP_CMP_REG = (ivm_ptr_t)_TMP_OBJ1 != 0;
	NEXT_INSTR();
))

#endif

OPCODE_GEN(GET_SLOT, "get_slot", S, 0, {
	CHECK_STACK(1);

	_TMP_OBJ1 = ivm_object_getSlot_cc(STACK_POP(), _STATE, SARG(), _INSTR);
	STACK_PUSH(_TMP_OBJ1 ? _TMP_OBJ1 : IVM_UNDEFINED(_STATE));

	NEXT_INSTR();
})

// no pop
OPCODE_GEN(GET_SLOT_N, "get_slot_n", S, 1, {
	CHECK_STACK(1);

	_TMP_OBJ1 = ivm_object_getSlot_cc(STACK_TOP(), _STATE, SARG(), _INSTR);
	STACK_PUSH(_TMP_OBJ1 ? _TMP_OBJ1 : IVM_UNDEFINED(_STATE));

	NEXT_INSTR();
})

/*
 * set_slot/set_proto:
 *
 * stack before:
 * ---------------
 * | obj1 | obj2 | ...
 * ---------------
 *
 * stack after:
 * --------
 * | obj1 | ...
 * --------
 *
 * while obj1.key = obj2
 */

OPCODE_GEN(SET_SLOT, "set_slot", S, -1, {
	CHECK_STACK(2);

	_TMP_OBJ1 = STACK_POP();
	ivm_object_setSlot_cc(_TMP_OBJ1, _STATE, SARG(), STACK_POP(), _INSTR);
	STACK_PUSH(_TMP_OBJ1);

	NEXT_INSTR();
})

/* backward */
OPCODE_GEN(SET_SLOT_B, "set_slot_b", S, -1, {
	CHECK_STACK(2);

	_TMP_OBJ1 = STACK_POP();
	ivm_object_setSlot_cc(STACK_TOP(), _STATE, SARG(), _TMP_OBJ1, _INSTR);

	NEXT_INSTR();
})

OPCODE_GEN(GET_PROTO, "get_proto", N, 0, {
	CHECK_STACK(1);

	_TMP_OBJ1 = ivm_object_getProto(STACK_POP());
	STACK_PUSH(_TMP_OBJ1 ? _TMP_OBJ1 : IVM_UNDEFINED(_STATE));

	NEXT_INSTR();
})

OPCODE_GEN(SET_PROTO, "set_proto", N, -1, {
	CHECK_STACK(2);

	_TMP_OBJ1 = STACK_POP(); // obj
	_TMP_OBJ2 = STACK_POP(); // proto

	// no circular prototype ref
	RTM_ASSERT(
		!ivm_object_hasProto(_TMP_OBJ2, _TMP_OBJ1),
		IVM_ERROR_MSG_CIRCULAR_PROTO_REF
	);

	ivm_object_setProto(_TMP_OBJ1, _STATE, _TMP_OBJ2);
	STACK_PUSH(_TMP_OBJ1);

	NEXT_INSTR();
})

OPCODE_GEN(GET_CONTEXT_SLOT, "get_context_slot", S, 1, {
	_TMP_OBJ1 = ivm_ctchain_search_cc(
		_CONTEXT, _STATE,
		SARG(), _INSTR
	);
	STACK_PUSH(_TMP_OBJ1 ? _TMP_OBJ1 : IVM_UNDEFINED(_STATE));

/*
	if (_TMP_OBJ1) {
		IVM_TRACE("%d\n", IVM_TYPE_TAG_OF(_TMP_OBJ1));
	}
*/
	// IVM_TRACE("gcs: %p %s -> %p\n", _CONTEXT, ivm_string_trimHead(SARG()), _TMP_OBJ1);

	NEXT_INSTR();
})

OPCODE_GEN(SET_CONTEXT_SLOT, "set_context_slot", S, -1, {
	_TMP_STR = SARG();

	CHECK_STACK(1);

	_TMP_OBJ1 = STACK_POP();

	if (!ivm_ctchain_setExistSlot_cc( /* try to find and set existed slot */
			_CONTEXT, _STATE, _TMP_STR,
			_TMP_OBJ1, _INSTR
		)) {
		/* found no existed slot -> set local slot */
		ivm_context_setSlot_cc(
			ivm_ctchain_getLocal(_CONTEXT),
			_STATE, _TMP_STR, _TMP_OBJ1,
			_INSTR
		);
	}

	NEXT_INSTR();
})

/* let object in ink */
OPCODE_GEN(GET_LOCAL_CONTEXT, "get_local_context", N, 1, {
	_TMP_OBJ1 = ivm_object_new_t(
		_STATE,
		ivm_context_getSlotTable(
			ivm_ctchain_getLocal(_CONTEXT)
		)
	);
	STACK_PUSH(_TMP_OBJ1);

	NEXT_INSTR();
})

OPCODE_GEN(SET_LOCAL_CONTEXT, "set_local_context", N, -1, {
	CHECK_STACK(1);
	ivm_ctchain_setLocal(_CONTEXT, _STATE, STACK_POP());
	NEXT_INSTR();
})

/* top object in ink */
OPCODE_GEN(GET_GLOBAL_CONTEXT, "get_global_context", N, 1, {
	_TMP_OBJ1 = ivm_object_new_t(
		_STATE,
		ivm_context_getSlotTable(
			ivm_ctchain_getGlobal(_CONTEXT)
		)
	);
	STACK_PUSH(_TMP_OBJ1);

	NEXT_INSTR();
})

OPCODE_GEN(SET_GLOBAL_CONTEXT, "set_global_context", N, -1, {
	CHECK_STACK(1);
	ivm_ctchain_setGlobal(_CONTEXT, _STATE, STACK_POP());
	NEXT_INSTR();
})

#if 1

OPCODE_GEN(SET_LOCAL_SLOT, "set_local_slot", S, -1, {
	CHECK_STACK(1);

	ivm_context_setSlot_cc(
		ivm_ctchain_getLocal(_CONTEXT),
		_STATE, SARG(), STACK_POP(),
		_INSTR
	);

	NEXT_INSTR();
})

OPCODE_GEN(GET_LOCAL_SLOT, "get_local_slot", S, 1, {
	_TMP_OBJ1 = ivm_context_getSlot_cc(
		ivm_ctchain_getLocal(_CONTEXT),
		_STATE, SARG(), _INSTR
	);

	STACK_PUSH(_TMP_OBJ1 ? _TMP_OBJ1 : IVM_UNDEFINED(_STATE));

	NEXT_INSTR();
})

OPCODE_GEN(SET_GLOBAL_SLOT, "set_global_slot", S, -1, {
	CHECK_STACK(1);

	ivm_context_setSlot_cc(
		ivm_ctchain_getGlobal(_CONTEXT),
		_STATE, SARG(), STACK_POP(),
		_INSTR
	);

	NEXT_INSTR();
})

OPCODE_GEN(GET_GLOBAL_SLOT, "get_global_slot", S, 1, {
	_TMP_OBJ1 = ivm_context_getSlot_cc(
		ivm_ctchain_getGlobal(_CONTEXT),
		_STATE, SARG(), _INSTR
	);

	STACK_PUSH(_TMP_OBJ1 ? _TMP_OBJ1 : IVM_UNDEFINED(_STATE));

	NEXT_INSTR();
})

#endif

OPCODE_GEN(SET_ARG, "set_arg", S, -1, {
	if (AVAIL_STACK >= 1) {
		ivm_context_setSlot_cc(
			ivm_ctchain_getLocal(_CONTEXT),
			_STATE, SARG(), STACK_POP(),
			_INSTR
		);
	} else {
		ivm_context_setSlot_cc(
			ivm_ctchain_getLocal(_CONTEXT),
			_STATE, SARG(), IVM_UNDEFINED(_STATE),
			_INSTR
		);
	}

	NEXT_INSTR();
})

/*
	top
	--------------
	| obj | op_f |
	--------------
 */
OPCODE_GEN(SET_OOP, "set_oop", I, -1, {
	CHECK_STACK(2);

	_TMP_OBJ1 = STACK_POP();
	_TMP_OBJ2 = STACK_POP();

	ivm_object_setOop(_TMP_OBJ1, _STATE, IARG(), _TMP_OBJ2);

	STACK_PUSH(_TMP_OBJ1);

	NEXT_INSTR();
})

/* backward */
OPCODE_GEN(SET_OOP_B, "set_oop_b", I, -1, {
	CHECK_STACK(2);

	_TMP_OBJ2 = STACK_POP();
	_TMP_OBJ1 = STACK_TOP();

	ivm_object_setOop(_TMP_OBJ1, _STATE, IARG(), _TMP_OBJ2);

	NEXT_INSTR();
})

OPCODE_GEN(GET_OOP, "get_oop", I, -1, {
	CHECK_STACK(1);

	_TMP_OBJ1 = STACK_POP();
	_TMP_OBJ1 = ivm_object_getOop(_TMP_OBJ1, IARG());

	STACK_PUSH(_TMP_OBJ1 ? _TMP_OBJ1 : IVM_UNDEFINED(_STATE));

	NEXT_INSTR();
})

OPCODE_GEN(POP, "pop", N, -1, {
	CHECK_STACK(1);
	STACK_POP();
	NEXT_INSTR();
})

OPCODE_GEN(DUP_N, "dup_n", I, 1, {
	ivm_size_t i = IARG();

	CHECK_STACK(i + 1);

	_TMP_OBJ1 = STACK_BEFORE(i);
	STACK_PUSH(_TMP_OBJ1);

	NEXT_INSTR();
})

OPCODE_GEN(DUP, "dup", N, 1, {
	CHECK_STACK(1);

	_TMP_OBJ1 = STACK_TOP();
	STACK_PUSH(_TMP_OBJ1);

	NEXT_INSTR();
})

#if 1

OPCODE_GEN(OUT, "out", S, 0, {
	IVM_TRACE("%s\n", ivm_string_trimHead(SARG()));
	NEXT_INSTR();
})

OPCODE_GEN(OUT_NUM, "out_num", N, 0, {
	CHECK_STACK(1);

	_TMP_OBJ1 = STACK_TOP();
	if (IVM_OBJECT_GET(_TMP_OBJ1, TYPE_TAG) == IVM_NUMERIC_T) {
		IVM_TRACE("%.3f\n", IVM_AS(_TMP_OBJ1, ivm_numeric_t)->val);
	} else {
		IVM_TRACE("cannot print number of object of type <%s>\n", IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME));
	}

	NEXT_INSTR();
})

OPCODE_GEN(OUT_STR, "out_str", N, 0, {
	CHECK_STACK(1);

	_TMP_OBJ1 = STACK_TOP();
	if (IVM_OBJECT_GET(_TMP_OBJ1, TYPE_TAG) == IVM_STRING_OBJECT_T) {
		IVM_TRACE("%s\n", ivm_string_trimHead(IVM_AS(_TMP_OBJ1, ivm_string_object_t)->val));
	} else {
		IVM_TRACE("cannot print string of object of type <%s>\n", IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME));
	}

	NEXT_INSTR();
})

OPCODE_GEN(OUT_STACK_SIZE, "out_stack_size", N, 0, {
	IVM_TRACE("%ld\n", STACK_SIZE());
	NEXT_INSTR();
})

OPCODE_GEN(OUT_TYPE, "out_type", N, 0, {
	CHECK_STACK(1);

	_TMP_OBJ1 = STACK_TOP();
	IVM_TRACE("%s\n", _TMP_OBJ1 ? IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME) : "<null pointer>");
	
	NEXT_INSTR();
})

#endif

/*
 * invoke:
 * ----------------------
 * | func | arg1 | arg2 | ...f
 * ----------------------
 *
 * invoke_base:
 * -----------------------------
 * | func | base | arg1 | arg2 | ...
 * -----------------------------
 * 
 */
OPCODE_GEN(INVOKE, "invoke", I, -(IVM_OPCODE_VARIABLE_STACK_INC), {
	_TMP_ARGC = IARG();

	CHECK_STACK(_TMP_ARGC + 1);
	_TMP_OBJ1 = STACK_POP();

	// IVM_TRACE("hi\n");
	// IVM_TRACE("%p\n", _TMP_OBJ1);

	// IVM_TRACE("invoke! %p\n", _TMP_OBJ1);
	// 
	
	// if (!IVM_IS_TYPE(_TMP_OBJ1, IVM_FUNCTION_OBJECT_T)) {
	// 	STACK_PUSH(ivm_numeric_new(_STATE, 10016));
	//	NEXT_INSTR();
	// }

	_TMP_ARGV = STACK_CUR();
	STACK_CUT(_TMP_ARGC);

	if (!IVM_IS_TYPE(_TMP_OBJ1, IVM_FUNCTION_OBJECT_T)) {
		_TMP_OBJ2 = ivm_object_getOop(_TMP_OBJ1, IVM_OOP_ID(CALL));
		if (_TMP_OBJ2) {
			STACK_INC(_TMP_ARGC);
			STACK_PUSH(_TMP_OBJ1);
			STACK_PUSH(_TMP_OBJ2);
			GOTO_INSTR(INVOKE_BASE);
		} else {
			RTM_FATAL(IVM_ERROR_MSG_UNABLE_TO_INVOKE(IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME)));
		}
	}

	//RTM_ASSERT(IVM_IS_TYPE(_TMP_OBJ1, IVM_FUNCTION_OBJECT_T),
	//		   IVM_ERROR_MSG_UNABLE_TO_INVOKE(IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME)));
	
	_TMP_FUNC = ivm_function_object_getFunc(IVM_AS(_TMP_OBJ1, ivm_function_object_t));

	SAVE_RUNTIME(_INSTR + 1);

	if (ivm_function_invoke(
			_TMP_FUNC, _STATE,
			ivm_function_object_getScope(
				IVM_AS(_TMP_OBJ1, ivm_function_object_t)
			), _RUNTIME, _FRAME_STACK
		)) {
		IVM_PER_INSTR_DBG(DBG_RUNTIME_ACTION(INVOKE, 1 /* native invoke */));

		INVOKE_STACK();

		_TMP_BOOL = IVM_CORO_GET(_CORO, HAS_NATIVE);
		IVM_CORO_SET(_CORO, HAS_NATIVE, IVM_TRUE);

		_TMP_OBJ1 = ivm_function_callNative(
			_TMP_FUNC, _STATE, _CORO, _CONTEXT,
			IVM_FUNCTION_SET_ARG_2(_TMP_ARGC, _TMP_ARGV)
		);

		IVM_CORO_SET(_CORO, HAS_NATIVE, _TMP_BOOL);

		if (ivm_vmstate_getException(_STATE)) {
			RAISE(ivm_vmstate_popException(_STATE))
		}

		if (!_TMP_OBJ1) {
			_TMP_OBJ1 = IVM_NULL_OBJ(_STATE);
		}

		RETURN();
	} else {
		INVOKE_STACK();
		IVM_PER_INSTR_DBG(DBG_RUNTIME_ACTION(INVOKE, IVM_NULL));
		STACK_INC(_TMP_ARGC);
		INVOKE();
	}
})

OPCODE_GEN(INVOKE_BASE, "invoke_base", I, -(1 + IVM_OPCODE_VARIABLE_STACK_INC), {
	_TMP_ARGC = IARG();

	CHECK_STACK(_TMP_ARGC + 2);

	_TMP_OBJ1 = STACK_POP();
	_TMP_OBJ2 = STACK_POP();

	_TMP_ARGV = STACK_CUR();
	STACK_CUT(_TMP_ARGC);

	while (!IVM_IS_TYPE(_TMP_OBJ1, IVM_FUNCTION_OBJECT_T)) {
		/* _TMP_OBJ2 is the base */
		_TMP_OBJ2 = _TMP_OBJ1;
		_TMP_OBJ1 = ivm_object_getOop(_TMP_OBJ1, IVM_OOP_ID(CALL));
		RTM_ASSERT(_TMP_OBJ1, IVM_ERROR_MSG_UNABLE_TO_INVOKE(IVM_OBJECT_GET(_TMP_OBJ2, TYPE_NAME)));
	}

	// IVM_TRACE("%p %p %d\n", _TMP_OBJ1, _TMP_OBJ1->type, IVM_OBJECT_GET(_TMP_OBJ1, GEN));

	// RTM_ASSERT(IVM_IS_TYPE(_TMP_OBJ1, IVM_FUNCTION_OBJECT_T),
	// 		   IVM_ERROR_MSG_UNABLE_TO_INVOKE(IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME)));

	_TMP_FUNC = ivm_function_object_getFunc(IVM_AS(_TMP_OBJ1, ivm_function_object_t));

	SAVE_RUNTIME(_INSTR + 1);

	if (ivm_function_invokeBase(
			_TMP_FUNC, _STATE,
			ivm_function_object_getScope(
				IVM_AS(_TMP_OBJ1, ivm_function_object_t)
			), _RUNTIME, _FRAME_STACK, _TMP_OBJ2
		)) {
		IVM_PER_INSTR_DBG(DBG_RUNTIME_ACTION(INVOKE, 1 /* native invoke */));

		INVOKE_STACK();

		_TMP_BOOL = IVM_CORO_GET(_CORO, HAS_NATIVE);
		IVM_CORO_SET(_CORO, HAS_NATIVE, IVM_TRUE);

		_TMP_OBJ1 = ivm_function_callNative(
			_TMP_FUNC, _STATE, _CORO, _CONTEXT,
			IVM_FUNCTION_SET_ARG_3(_TMP_OBJ2, _TMP_ARGC, _TMP_ARGV)
		);

		IVM_CORO_SET(_CORO, HAS_NATIVE, _TMP_BOOL);

		if (ivm_vmstate_getException(_STATE)) {
			RAISE(ivm_vmstate_popException(_STATE))
		}

		if (!_TMP_OBJ1) {
			_TMP_OBJ1 = IVM_NULL_OBJ(_STATE);
		}

		RETURN();
	} else {
		INVOKE_STACK();
		IVM_PER_INSTR_DBG(DBG_RUNTIME_ACTION(INVOKE, IVM_NULL));
		STACK_INC(_TMP_ARGC);
		INVOKE();
	}
})

OPCODE_GEN(FORK, "fork", N, -1, {
	CHECK_STACK(1);

	_TMP_OBJ1 = STACK_POP();

	RTM_ASSERT(IVM_IS_TYPE(_TMP_OBJ1, IVM_FUNCTION_OBJECT_T),
			   IVM_ERROR_MSG_NOT_TYPE("function", IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME)));

	ivm_vmstate_addToCurrentGroup(
		_STATE,
		IVM_AS(_TMP_OBJ1, ivm_function_object_t)
	);

	NEXT_INSTR();
})

OPCODE_GEN(GROUP, "group", N, 0, {
	CHECK_STACK(1);

	_TMP_OBJ1 = STACK_POP();

	RTM_ASSERT(IVM_IS_TYPE(_TMP_OBJ1, IVM_FUNCTION_OBJECT_T),
			   IVM_ERROR_MSG_NOT_TYPE("function", IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME)));

	STACK_PUSH(
		ivm_numeric_new(_STATE,
			ivm_vmstate_addGroup(
				_STATE,
				IVM_AS(_TMP_OBJ1, ivm_function_object_t)
			)
		)
	);

	NEXT_INSTR();
})

OPCODE_GEN(GROUP_TO, "group_to", N, -1, {
	CHECK_STACK(2);

	_TMP_OBJ1 = STACK_POP();
	_TMP_OBJ2 = STACK_POP();

	RTM_ASSERT(IVM_IS_TYPE(_TMP_OBJ1, IVM_FUNCTION_OBJECT_T),
			   IVM_ERROR_MSG_NOT_TYPE("function", IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME)));

	RTM_ASSERT(IVM_IS_TYPE(_TMP_OBJ2, IVM_NUMERIC_T),
			   IVM_ERROR_MSG_ILLEGAL_GID_TYPE(IVM_OBJECT_GET(_TMP_OBJ2, TYPE_NAME)));

	STACK_PUSH(
		ivm_numeric_new(_STATE,
			ivm_vmstate_addToGroup(
				_STATE,
				IVM_AS(_TMP_OBJ1, ivm_function_object_t),
				ivm_numeric_getValue(_TMP_OBJ2)
			)
		)
	);

	NEXT_INSTR();
})

OPCODE_GEN(YIELD, "yield", N, 0, {
	if (AVAIL_STACK) {
		_TMP_OBJ1 = STACK_POP();
	} else {
		_TMP_OBJ1 = IVM_NULL_OBJ(_STATE);
	}

	INC_INSTR();
	SAVE_RUNTIME(_INSTR);

	if (IVM_CORO_GET(_CORO, HAS_NATIVE)) {
		_TMP_OBJ1 = ivm_vmstate_schedule_r(_STATE, _TMP_OBJ1);
		UPDATE_RUNTIME();
		STACK_PUSH(_TMP_OBJ1);
		GOTO_CUR_INSTR();
	} else {
		YIELD();
	}
})

/*
	yield_to
	-------------------------
	|  gid  | obj(optional) |
	-------------------------
 */
OPCODE_GEN(YIELD_TO, "yield_to", N, -1, {
	CHECK_STACK(1);

	_TMP_OBJ2 = STACK_POP();

	if (AVAIL_STACK) {
		_TMP_OBJ1 = STACK_POP();
	} else {
		_TMP_OBJ1 = IVM_NULL_OBJ(_STATE);
	}

	RTM_ASSERT(IVM_IS_TYPE(_TMP_OBJ2, IVM_NUMERIC_T),
			   IVM_ERROR_MSG_ILLEGAL_GID_TYPE(IVM_OBJECT_GET(_TMP_OBJ2, TYPE_NAME)));

	INC_INSTR();
	SAVE_RUNTIME(_INSTR);

	ivm_vmstate_yieldTo(_STATE, ivm_numeric_getValue(_TMP_OBJ2));

	if (IVM_CORO_GET(_CORO, HAS_NATIVE)) {
		_TMP_OBJ1 = ivm_vmstate_schedule_r(_STATE, _TMP_OBJ1);
		UPDATE_RUNTIME();
		STACK_PUSH(_TMP_OBJ1);
		GOTO_CUR_INSTR();
	} else {
		YIELD();
	}
})

/*
	raise protection set/cancel
 */
OPCODE_GEN(RPROT_SET, "rprot_set", A, 0, {
	IVM_RUNTIME_SET(_RUNTIME, CATCH, ADDR_ARG());
	NEXT_INSTR();
})

OPCODE_GEN(RPROT_CAC, "rprot_cac", N, 0, {
	IVM_RUNTIME_SET(_RUNTIME, CATCH, IVM_NULL);
	NEXT_INSTR();
})

OPCODE_GEN(RAISE, "raise", N, -1, {
	CHECK_STACK(1);
	RAISE(STACK_POP());
})

OPCODE_GEN(RETURN, "return", N, -1, {
	if (AVAIL_STACK) {
		_TMP_OBJ1 = STACK_POP();
	} else {
		_TMP_OBJ1 = IVM_NULL_OBJ(_STATE);
	}

	RETURN();
})

OPCODE_GEN(JUMP, "jump", A, 0, {
	GOTO_SET_INSTR(ADDR_ARG());
})

OPCODE_GEN(JUMP_TRUE, "jump_true", A, -1, {
	if (ivm_object_toBool(STACK_POP(), _STATE)) {
		GOTO_SET_INSTR(ADDR_ARG());
	} else {
		NEXT_INSTR();
	}
})

OPCODE_GEN(JUMP_FALSE, "jump_false", A, -1, {
	if (!ivm_object_toBool(STACK_POP(), _STATE)) {
		GOTO_SET_INSTR(ADDR_ARG());
	} else {
		NEXT_INSTR();
	}
})

#if 1

OPCODE_GEN(JUMP_TRUE_R, "jump_true_r", A, 0, {
	if (IVM_RUNTIME_GET(_RUNTIME, NO_REG)) {
		IVM_RUNTIME_SET(_RUNTIME, NO_REG, IVM_FALSE);
		GOTO_INSTR(JUMP_TRUE);
	}

	if (_TMP_CMP_REG) {
		GOTO_SET_INSTR(ADDR_ARG());
	} else {
		NEXT_INSTR();
	}
})

OPCODE_GEN(JUMP_FALSE_R, "jump_false_r", A, 0, {
	if (IVM_RUNTIME_GET(_RUNTIME, NO_REG)) {
		IVM_RUNTIME_SET(_RUNTIME, NO_REG, IVM_FALSE);
		GOTO_INSTR(JUMP_FALSE);
	}

	if (!_TMP_CMP_REG) {
		GOTO_SET_INSTR(ADDR_ARG());
	} else {
		NEXT_INSTR();
	}
})

#endif

/* no pop */
OPCODE_GEN(JUMP_TRUE_N, "jump_true_n", A, -1, {
	if (ivm_object_toBool(STACK_TOP(), _STATE)) {
		GOTO_SET_INSTR(ADDR_ARG());
	} else {
		NEXT_INSTR();
	}
})

OPCODE_GEN(JUMP_FALSE_N, "jump_false_n", A, -1, {
	if (!ivm_object_toBool(STACK_TOP(), _STATE)) {
		GOTO_SET_INSTR(ADDR_ARG());
	} else {
		NEXT_INSTR();
	}
})

#if 0

OPCODE_GEN(JUMP_LT, "jump_lt", A, -2, CMP_HANDLER(0,
	if ((ivm_ptr_t)_TMP_OBJ1 < 0) {
		GOTO_SET_INSTR(ADDR_ARG());
	} else {
		NEXT_INSTR();
	}
))

OPCODE_GEN(JUMP_LE, "jump_le", A, -2, CMP_HANDLER(0,
	if ((ivm_ptr_t)_TMP_OBJ1 <= 0) {
		GOTO_SET_INSTR(ADDR_ARG());
	} else {
		NEXT_INSTR();
	}
))

OPCODE_GEN(JUMP_GT, "jump_gt", A, -2, CMP_HANDLER(0,
	if ((ivm_ptr_t)_TMP_OBJ1 > 0) {
		GOTO_SET_INSTR(ADDR_ARG());
	} else {
		NEXT_INSTR();
	}
))

OPCODE_GEN(JUMP_GE, "jump_ge", A, -2, CMP_HANDLER(0,
	if ((ivm_ptr_t)_TMP_OBJ1 >= 0) {
		GOTO_SET_INSTR(ADDR_ARG());
	} else {
		NEXT_INSTR();
	}
))

#endif
