OPCODE_GEN(NOP, "nop", N, 0, {
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

OPCODE_GEN(CLONE, "clone", N, 1, {
	CHECK_STACK(1);
	STACK_OVERRIDE(ivm_object_clone(STACK_TOP(), _STATE));
	NEXT_INSTR();
})

/* unary operations */
OPCODE_GEN(NOT, "not", N, 0, DEFAULT_UNIOP_HANDLER(NOT, "!"))

/* binary operations */
OPCODE_GEN(ADD, "add", N, -1, DEFAULT_BINOP_HANDLER(ADD, "+", IVM_NULL))
OPCODE_GEN(SUB, "sub", N, -1, DEFAULT_BINOP_HANDLER(SUB, "-", IVM_NULL))
OPCODE_GEN(MUL, "mul", N, -1, DEFAULT_BINOP_HANDLER(MUL, "*", IVM_NULL))
OPCODE_GEN(DIV, "div", N, -1, DEFAULT_BINOP_HANDLER(DIV, "/", IVM_NULL))
OPCODE_GEN(MOD, "mod", N, -1, DEFAULT_BINOP_HANDLER(MOD, "%", IVM_NULL))

OPCODE_GEN(AND, "and", N, -1, DEFAULT_BINOP_HANDLER(AND, "&", IVM_NULL))
OPCODE_GEN(EOR, "eor", N, -1, DEFAULT_BINOP_HANDLER(EOR, "^", IVM_NULL))
OPCODE_GEN(IOR, "ior", N, -1, DEFAULT_BINOP_HANDLER(IOR, "|", IVM_NULL))

OPCODE_GEN(IDX, "idx", N, -1, DEFAULT_BINOP_HANDLER(IDX, "[]", IVM_NULL))
OPCODE_GEN(IDX_ASSIGN, "idx_assign", N, -2, DEFAULT_BINOP_HANDLER(IDX, "[]", STACK_POP()))

OPCODE_GEN(LT, "lt", N, -1, CMP_BINOP_HANDLER(
	STACK_PUSH(ivm_numeric_new(_STATE, (ivm_ptr_t)_TMP_OBJ1 < 0));
	NEXT_INSTR();
))

OPCODE_GEN(LE, "le", N, -1, CMP_BINOP_HANDLER(
	STACK_PUSH(ivm_numeric_new(_STATE, (ivm_ptr_t)_TMP_OBJ1 <= 0));
	NEXT_INSTR();
))

OPCODE_GEN(GT, "gt", N, -1, CMP_BINOP_HANDLER(
	STACK_PUSH(ivm_numeric_new(_STATE, (ivm_ptr_t)_TMP_OBJ1 > 0));
	NEXT_INSTR();
))

OPCODE_GEN(GE, "ge", N, -1, CMP_BINOP_HANDLER(
	STACK_PUSH(ivm_numeric_new(_STATE, (ivm_ptr_t)_TMP_OBJ1 >= 0));
	NEXT_INSTR();
))

OPCODE_GEN(EQ, "eq", N, -1, CMP_BINOP_HANDLER(
	STACK_PUSH(ivm_numeric_new(_STATE, (ivm_ptr_t)_TMP_OBJ1 == 0));
	NEXT_INSTR();
))

OPCODE_GEN(NE, "ne", N, -1, CMP_BINOP_HANDLER(
	STACK_PUSH(ivm_numeric_new(_STATE, (ivm_ptr_t)_TMP_OBJ1 != 0));
	NEXT_INSTR();
))

OPCODE_GEN(LT_R, "lt_r", N, -2, CMP_BINOP_HANDLER(
	_TMP_CMP_REG = (ivm_ptr_t)_TMP_OBJ1 < 0;
	NEXT_INSTR();
))

OPCODE_GEN(LE_R, "le_r", N, -2, CMP_BINOP_HANDLER(
	_TMP_CMP_REG = (ivm_ptr_t)_TMP_OBJ1 <= 0;
	NEXT_INSTR();
))

OPCODE_GEN(GT_R, "gt_r", N, -2, CMP_BINOP_HANDLER(
	_TMP_CMP_REG = (ivm_ptr_t)_TMP_OBJ1 > 0;
	NEXT_INSTR();
))

OPCODE_GEN(GE_R, "ge_r", N, -2, CMP_BINOP_HANDLER(
	_TMP_CMP_REG = (ivm_ptr_t)_TMP_OBJ1 >= 0;
	NEXT_INSTR();
))

OPCODE_GEN(EQ_R, "eq_r", N, -2, CMP_BINOP_HANDLER(
	_TMP_CMP_REG = (ivm_ptr_t)_TMP_OBJ1 == 0;
	NEXT_INSTR();
))

OPCODE_GEN(NE_R, "ne_r", N, -2, CMP_BINOP_HANDLER(
	_TMP_CMP_REG = (ivm_ptr_t)_TMP_OBJ1 != 0;
	NEXT_INSTR();
))

OPCODE_GEN(GET_SLOT, "get_slot", S, 0, {
	CHECK_STACK(1);

	_TMP_OBJ1 = ivm_object_getSlot_cc(STACK_POP(), _STATE, SARG(), _INSTR_CACHE);
	STACK_PUSH(_TMP_OBJ1 ? _TMP_OBJ1 : IVM_UNDEFINED(_STATE));

	NEXT_INSTR();
})

// no pop
OPCODE_GEN(GET_SLOT_N, "get_slot_n", S, 1, {
	CHECK_STACK(1);

	_TMP_OBJ1 = ivm_object_getSlot_cc(STACK_TOP(), _STATE, SARG(), _INSTR_CACHE);
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
	ivm_object_setSlot_cc(_TMP_OBJ1, _STATE, SARG(), STACK_POP(), _INSTR_CACHE);
	STACK_PUSH(_TMP_OBJ1);

	NEXT_INSTR();
})

/* backward */
OPCODE_GEN(SET_SLOT_B, "set_slot_b", S, -1, {
	CHECK_STACK(2);

	_TMP_OBJ1 = STACK_POP();
	ivm_object_setSlot_cc(STACK_TOP(), _STATE, SARG(), _TMP_OBJ1, _INSTR_CACHE);

	NEXT_INSTR();
})

OPCODE_GEN(GET_PROTO, "get_proto", N, 0, {
	CHECK_STACK(1);

	_TMP_OBJ1 = IVM_OBJECT_GET(STACK_POP(), PROTO);
	STACK_PUSH(_TMP_OBJ1 ? _TMP_OBJ1 : IVM_UNDEFINED(_STATE));

	NEXT_INSTR();
})

OPCODE_GEN(SET_PROTO, "set_proto", N, -1, {
	CHECK_STACK(2);

	_TMP_OBJ1 = STACK_POP();
	IVM_OBJECT_SET(_TMP_OBJ1, PROTO, STACK_POP());
	STACK_PUSH(_TMP_OBJ1);

	NEXT_INSTR();
})

OPCODE_GEN(GET_CONTEXT_SLOT, "get_context_slot", S, 1, {
	_TMP_OBJ1 = ivm_ctchain_search_cc(
		_CONTEXT, _STATE,
		SARG(), _INSTR_CACHE
	);
	STACK_PUSH(_TMP_OBJ1 ? _TMP_OBJ1 : IVM_UNDEFINED(_STATE));

	NEXT_INSTR();
})

OPCODE_GEN(SET_CONTEXT_SLOT, "set_context_slot", S, -1, {
	_TMP_STR = SARG();

	CHECK_STACK(1);

	_TMP_OBJ1 = STACK_POP();

	if (!ivm_ctchain_setExistSlot_cc( /* try to find and set existed slot */
			_CONTEXT, _STATE, _TMP_STR,
			_TMP_OBJ1, _INSTR_CACHE
		)) {
		/* found no existed slot -> set local slot */
		ivm_context_setSlot_cc(
			ivm_ctchain_getLocal(_CONTEXT),
			_STATE, _TMP_STR, _TMP_OBJ1,
			_INSTR_CACHE
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

	ivm_context_setSlotTable(
		ivm_ctchain_getLocal(_CONTEXT),
		IVM_OBJECT_GET(STACK_POP(), SLOTS)
	);

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
	
	ivm_context_setSlotTable(
		ivm_ctchain_getGlobal(_CONTEXT),
		IVM_OBJECT_GET(STACK_POP(), SLOTS)
	);

	NEXT_INSTR();
})

#if 1

OPCODE_GEN(SET_LOCAL_SLOT, "set_local_slot", S, -1, {
	CHECK_STACK(1);

	ivm_context_setSlot_cc(
		ivm_ctchain_getLocal(_CONTEXT),
		_STATE, SARG(), STACK_POP(),
		_INSTR_CACHE
	);

	NEXT_INSTR();
})

OPCODE_GEN(GET_LOCAL_SLOT, "get_local_slot", S, 1, {
	_TMP_OBJ1 = ivm_context_getSlot_cc(
		ivm_ctchain_getLocal(_CONTEXT),
		_STATE, SARG(), _INSTR_CACHE
	);

	STACK_PUSH(_TMP_OBJ1 ? _TMP_OBJ1 : IVM_UNDEFINED(_STATE));

	NEXT_INSTR();
})

OPCODE_GEN(SET_GLOBAL_SLOT, "set_global_slot", S, -1, {
	CHECK_STACK(1);

	ivm_context_setSlot_cc(
		ivm_ctchain_getGlobal(_CONTEXT),
		_STATE, SARG(), STACK_POP(),
		_INSTR_CACHE
	);

	NEXT_INSTR();
})

OPCODE_GEN(GET_GLOBAL_SLOT, "get_global_slot", S, 1, {
	_TMP_OBJ1 = ivm_context_getSlot_cc(
		ivm_ctchain_getGlobal(_CONTEXT),
		_STATE, SARG(), _INSTR_CACHE
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
			_INSTR_CACHE
		);
	}

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
 * | func | arg1 | arg2 | ...
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

	IVM_ASSERT(IVM_IS_TYPE(_TMP_OBJ1, IVM_FUNCTION_OBJECT_T),
			   IVM_ERROR_MSG_NOT_TYPE("function", IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME)));
	
	_TMP_FUNC = ivm_function_object_getFunc(IVM_AS(_TMP_OBJ1, ivm_function_object_t));
	_TMP_ARGV = STACK_CUT(_TMP_ARGC);

	SAVE_RUNTIME(_INSTR + 1);

	// STACK_ENSURE(ivm_function_getMaxStack(_TMP_FUNC));

	ivm_function_invoke(_TMP_FUNC, _STATE,
						ivm_function_object_getScope(
							IVM_AS(_TMP_OBJ1, ivm_function_object_t)
						), _RUNTIME, _FRAME_STACK);
	
	INVOKE_STACK();
	// UPDATE_STACK();

	if (ivm_function_isNative(_TMP_FUNC)) {
		IVM_PER_INSTR_DBG(DBG_RUNTIME_ACTION(INVOKE, 1 /* native invoke */));

		_TMP_OBJ1 = ivm_function_callNative(
			_TMP_FUNC, _STATE, _CONTEXT,
			IVM_FUNCTION_SET_ARG_2(_TMP_ARGC, _TMP_ARGV)
		);

		if (!_TMP_OBJ1) {
			_TMP_OBJ1 = IVM_NULL_OBJ(_STATE);
		}

		RETURN();
	} else {
		IVM_PER_INSTR_DBG(DBG_RUNTIME_ACTION(INVOKE, IVM_NULL));
		STACK_INC(_TMP_ARGC);
		INVOKE();
	}
})

OPCODE_GEN(INVOKE_BASE, "invoke_base", I, -(1 + IVM_OPCODE_VARIABLE_STACK_INC), {
	_TMP_ARGC = IARG();

	CHECK_STACK(_TMP_ARGC + 2);
	_TMP_OBJ1 = STACK_POP();

	IVM_ASSERT(IVM_IS_TYPE(_TMP_OBJ1, IVM_FUNCTION_OBJECT_T),
			   IVM_ERROR_MSG_NOT_TYPE("function", IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME)));
	
	_TMP_FUNC = ivm_function_object_getFunc(IVM_AS(_TMP_OBJ1, ivm_function_object_t));
	_TMP_ARGV = STACK_CUT(_TMP_ARGC + 1);

	SAVE_RUNTIME(_INSTR + 1);

	// STACK_ENSURE(ivm_function_getMaxStack(_TMP_FUNC));

	ivm_function_invokeBase(
		_TMP_FUNC, _STATE,
		ivm_function_object_getScope(
			IVM_AS(_TMP_OBJ1, ivm_function_object_t)
		), _RUNTIME, _FRAME_STACK, _TMP_ARGV[_TMP_ARGC]);
	
	INVOKE_STACK();
	// UPDATE_STACK();

	if (ivm_function_isNative(_TMP_FUNC)) {
		IVM_PER_INSTR_DBG(DBG_RUNTIME_ACTION(INVOKE, 1 /* native invoke */));

		_TMP_OBJ1 = ivm_function_callNative(
			_TMP_FUNC, _STATE, _CONTEXT,
			IVM_FUNCTION_SET_ARG_3(_TMP_ARGV[_TMP_ARGC], _TMP_ARGC, _TMP_ARGV)
		);

		if (!_TMP_OBJ1) {
			_TMP_OBJ1 = IVM_NULL_OBJ(_STATE);
		}

		RETURN();
	} else {
		IVM_PER_INSTR_DBG(DBG_RUNTIME_ACTION(INVOKE, IVM_NULL));
		STACK_INC(_TMP_ARGC);
		INVOKE();
	}
})

OPCODE_GEN(FORK, "fork", N, -1, {
	CHECK_STACK(1);

	_TMP_OBJ1 = STACK_POP();

	IVM_ASSERT(IVM_IS_TYPE(_TMP_OBJ1, IVM_FUNCTION_OBJECT_T),
			   IVM_ERROR_MSG_NOT_TYPE("function", IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME)));

	ivm_vmstate_addCoro(
		_STATE,
		IVM_AS(_TMP_OBJ1, ivm_function_object_t)
	);

	NEXT_INSTR();
})

OPCODE_GEN(YIELD, "yield", N, -1, {
	if (AVAIL_STACK) {
		_TMP_OBJ1 = STACK_POP();
	} else {
		_TMP_OBJ1 = IVM_NULL_OBJ(_STATE);
	}

	YIELD();
})

OPCODE_GEN(RETURN, "return", N, -1, {
	if (AVAIL_STACK) {
		_TMP_OBJ1 = STACK_POP();
	} else {
		_TMP_OBJ1 = IVM_NULL_OBJ(_STATE);
	}

	RETURN();
})

OPCODE_GEN(JUMP, "jump", I, 0, {
	NEXT_N_INSTR(IARG());
})

OPCODE_GEN(JUMP_TRUE, "jump_true", I, -1, {
	if (ivm_object_toBool(STACK_POP(), _STATE)) {
		NEXT_N_INSTR(IARG());
	} else {
		NEXT_INSTR();
	}
})

OPCODE_GEN(JUMP_FALSE, "jump_false", I, -1, {
	if (!ivm_object_toBool(STACK_POP(), _STATE)) {
		NEXT_N_INSTR(IARG());
	} else {
		NEXT_INSTR();
	}
})

OPCODE_GEN(JUMP_TRUE_R, "jump_true_r", I, 0, {
	if (_TMP_CMP_REG) {
		NEXT_N_INSTR(IARG());
	} else {
		NEXT_INSTR();
	}
})

OPCODE_GEN(JUMP_FALSE_R, "jump_false_r", I, 0, {
	if (!_TMP_CMP_REG) {
		NEXT_N_INSTR(IARG());
	} else {
		NEXT_INSTR();
	}
})

/*
	jump_true/false_n
		no pop
 */
OPCODE_GEN(JUMP_TRUE_N, "jump_true_n", I, -1, {
	if (ivm_object_toBool(STACK_TOP(), _STATE)) {
		NEXT_N_INSTR(IARG());
	} else {
		NEXT_INSTR();
	}
})

OPCODE_GEN(JUMP_FALSE_N, "jump_false_n", I, -1, {
	if (!ivm_object_toBool(STACK_TOP(), _STATE)) {
		NEXT_N_INSTR(IARG());
	} else {
		NEXT_INSTR();
	}
})

OPCODE_GEN(JUMP_LT, "jump_lt", I, -2, CMP_BINOP_HANDLER(
	if ((ivm_ptr_t)_TMP_OBJ1 < 0) {
		NEXT_N_INSTR(IARG());
	} else {
		NEXT_INSTR();
	}
))

OPCODE_GEN(JUMP_LE, "jump_le", I, -2, CMP_BINOP_HANDLER(
	if ((ivm_ptr_t)_TMP_OBJ1 <= 0) {
		NEXT_N_INSTR(IARG());
	} else {
		NEXT_INSTR();
	}
))

OPCODE_GEN(JUMP_GT, "jump_gt", I, -2, CMP_BINOP_HANDLER(
	if ((ivm_ptr_t)_TMP_OBJ1 > 0) {
		NEXT_N_INSTR(IARG());
	} else {
		NEXT_INSTR();
	}
))

OPCODE_GEN(JUMP_GE, "jump_ge", I, -2, CMP_BINOP_HANDLER(
	if ((ivm_ptr_t)_TMP_OBJ1 >= 0) {
		NEXT_N_INSTR(IARG());
	} else {
		NEXT_INSTR();
	}
))
