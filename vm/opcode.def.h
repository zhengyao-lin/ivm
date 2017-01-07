OPCODE_GEN(NOP, "nop", N, 0, {
	NEXT_INSTR_NINT();
})

OPCODE_GEN(NEW_NIL, "new_nil", N, 1, {
	STACK_PUSH(IVM_NULL);
	NEXT_INSTR_NINT();
})

OPCODE_GEN(NEW_NONE, "new_none", N, 1, {
	STACK_PUSH(IVM_NONE(_STATE));
	NEXT_INSTR_NINT();
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
			ivm_vmstate_getFunc(
				_STATE, IVM_RUNTIME_GET(_RUNTIME, OFFSET) + XARG()
			)
		)
	);
	NEXT_INSTR();
})

OPCODE_GEN(NEW_LIST, "new_list", I, 1, {
	_TMP_ARGC = IARG();

	CHECK_STACK(_TMP_ARGC);

	_TMP_ARGV = STACK_CUT(_TMP_ARGC);
	STACK_PUSH(ivm_list_object_new_c(_STATE, _TMP_ARGV, _TMP_ARGC));

	NEXT_INSTR();
})

OPCODE_GEN(NEW_LIST_ALL, "new_list_all", N, -1, {
	_TMP_ARGC = AVAIL_STACK;

	_TMP_ARGV = STACK_CUT(_TMP_ARGC);
	STACK_PUSH(ivm_list_object_new_c(_STATE, _TMP_ARGV, _TMP_ARGC));

	NEXT_INSTR();
})

/* pack up all elem on the stack except x elems on the bottom */
OPCODE_GEN(NEW_VARG, "new_varg", I, -1, {
	_TMP_ARGC = AVAIL_STACK - IARG();

	// IVM_TRACE("%d %d\n", AVAIL_STACK, _TMP_ARGC);

	if (_TMP_ARGC > 0) {
		_TMP_OBJ1 = ivm_list_object_new_c(_STATE, STACK_CUT(_TMP_ARGC), _TMP_ARGC);
		ivm_list_object_reverse(IVM_AS(_TMP_OBJ1, ivm_list_object_t));
		STACK_PUSH(_TMP_OBJ1);
	} else {
		STACK_PUSH(ivm_list_object_new(_STATE, 0));
	}

	NEXT_INSTR();
})

OPCODE_GEN(ENSURE_NONE, "ensure_none", N, 1, {
	if (!AVAIL_STACK) {
		STACK_PUSH(IVM_NONE(_STATE));
	}
	
	NEXT_INSTR();
})

OPCODE_GEN(TO_LIST, "to_list", N, 0, {
	CHECK_STACK(1);

	_TMP_OBJ1 = STACK_TOP();

	if (!_TMP_OBJ1) { // del flag detected
		NEXT_INSTR_NINT();
	}

	// non-normal list object -> use for to iterate
	_TMP_FUNC = ivm_vmstate_getTypeCons(_STATE, IVM_LIST_OBJECT_T);
	STACK_PUSH(ivm_function_object_new(_STATE, IVM_NULL, _TMP_FUNC));

	SET_IARG(1);
	GOTO_INSTR(INVOKE);
	// unreachable
})

OPCODE_GEN(UNPACK_LIST, "unpack_list", I, 1, {
	CHECK_STACK(1);

	_TMP_ARGC = IARG();

	_TMP_OBJ1 = STACK_POP();

	RTM_ASSERT(
		!_TMP_OBJ1 /* del */ || IVM_IS_BTTYPE(_TMP_OBJ1, _STATE, IVM_LIST_OBJECT_T),
		IVM_ERROR_MSG_UNPACK_NON_LIST(IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME))
	);

	if (_TMP_ARGC) {
		_TMP_ARGV = STACK_ENSURE(_TMP_ARGC);

		if (_TMP_OBJ1) {
			_ivm_list_object_unpackTo(
				IVM_AS(_TMP_OBJ1, ivm_list_object_t),
				_STATE, _TMP_ARGV, _TMP_ARGC
			);
		} else {
			STACK_BZERO(_TMP_ARGC);
		}

		STACK_INC_C(_TMP_ARGC);
	}

	NEXT_INSTR_NINT();
})

OPCODE_GEN(UNPACK_LIST_ALL, "unpack_list_all", N, 1, {
	CHECK_STACK(1);

	_TMP_OBJ1 = STACK_POP();

	RTM_ASSERT(_TMP_OBJ1, IVM_ERROR_MSG_DEL_VARG);

	RTM_ASSERT(
		IVM_IS_BTTYPE(_TMP_OBJ1, _STATE, IVM_LIST_OBJECT_T),
		IVM_ERROR_MSG_UNPACK_NON_LIST(IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME))
	);

	_TMP_ARGC = ivm_list_object_getSize(IVM_AS(_TMP_OBJ1, ivm_list_object_t));
	_TMP_ARGV = STACK_ENSURE(_TMP_ARGC);

	_ivm_list_object_unpackAll(IVM_AS(_TMP_OBJ1, ivm_list_object_t), _STATE, _TMP_ARGV);

	STACK_INC_C(_TMP_ARGC);

	NEXT_INSTR_NINT();
})

/* reversed unpack */
OPCODE_GEN(UNPACK_LIST_ALL_R, "unpack_list_all_r", N, 1, {
	CHECK_STACK(1);

	_TMP_OBJ1 = STACK_POP();

	RTM_ASSERT(_TMP_OBJ1, IVM_ERROR_MSG_DEL_VARG);

	RTM_ASSERT(
		IVM_IS_BTTYPE(_TMP_OBJ1, _STATE, IVM_LIST_OBJECT_T),
		IVM_ERROR_MSG_UNPACK_NON_LIST(IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME))
	);

	_TMP_ARGC = ivm_list_object_getSize(IVM_AS(_TMP_OBJ1, ivm_list_object_t));
	_TMP_ARGV = STACK_ENSURE(_TMP_ARGC);

	_ivm_list_object_unpackAll_r(IVM_AS(_TMP_OBJ1, ivm_list_object_t), _STATE, _TMP_ARGV);

	STACK_INC_C(_TMP_ARGC);

	NEXT_INSTR_NINT();
})

/* unary operations */
OPCODE_GEN(NOT, "not", N, 0, UNIOP_HANDLER(NOT, "!", {
	STACK_PUSH(ivm_numeric_new(_STATE, !ivm_object_toBool(_TMP_OBJ1, _STATE)));
	NEXT_INSTR();
}))
OPCODE_GEN(NEG, "neg", N, 0, UNIOP_HANDLER(NEG, "-", 0))
OPCODE_GEN(POS, "pos", N, 0, UNIOP_HANDLER(POS, "+", 0))
OPCODE_GEN(BNOT, "bnot", N, 0, UNIOP_HANDLER(BNOT, "~", 0))

/* binary operations */
OPCODE_GEN(ADD, "add", N, -1, BINOP_HANDLER(ADD, "+", 0))
OPCODE_GEN(SUB, "sub", N, -1, BINOP_HANDLER(SUB, "-", 0))
OPCODE_GEN(MUL, "mul", N, -1, BINOP_HANDLER(MUL, "*", 0))
OPCODE_GEN(DIV, "div", N, -1, BINOP_HANDLER(DIV, "/", 0))
OPCODE_GEN(MOD, "mod", N, -1, BINOP_HANDLER(MOD, "%", 0))

OPCODE_GEN(AND, "and", N, -1, BINOP_HANDLER(AND, "&", 0))
OPCODE_GEN(EOR, "eor", N, -1, BINOP_HANDLER(EOR, "^", 0))
OPCODE_GEN(IOR, "ior", N, -1, BINOP_HANDLER(IOR, "|", 0))

OPCODE_GEN(IDX, "idx", N, -1, BINOP_HANDLER(IDX, "[]", 0))

/*
	top
	-------------------------------
	|   id   |   obj   |  assign  |
	-------------------------------
 */
OPCODE_GEN(IDXA, "idxa", N, -2, TRIOP_HANDLER(IDXA, "[=]", 0))

OPCODE_GEN(SHL, "shl", N, -1, BINOP_HANDLER(SHL, "<<", 0))
OPCODE_GEN(SHAR, "shar", N, -1, BINOP_HANDLER(SHAR, ">>", 0))
OPCODE_GEN(SHLR, "shlr", N, -1, BINOP_HANDLER(SHLR, ">>>", 0))

/* inplace op */

OPCODE_GEN(INADD, "inadd", N, -1, BINOP_HANDLER(INADD, "+=", 0))
OPCODE_GEN(INSUB, "insub", N, -1, BINOP_HANDLER(INSUB, "-=", 0))
OPCODE_GEN(INMUL, "inmul", N, -1, BINOP_HANDLER(INMUL, "*=", 0))
OPCODE_GEN(INDIV, "indiv", N, -1, BINOP_HANDLER(INDIV, "/=", 0))
OPCODE_GEN(INMOD, "inmod", N, -1, BINOP_HANDLER(INMOD, "%=", 0))

OPCODE_GEN(INAND, "inand", N, -1, BINOP_HANDLER(INAND, "&=", 0))
OPCODE_GEN(INEOR, "ineor", N, -1, BINOP_HANDLER(INEOR, "^=", 0))
OPCODE_GEN(INIOR, "inior", N, -1, BINOP_HANDLER(INIOR, "|=", 0))

OPCODE_GEN(INSHL, "inshl", N, -1, BINOP_HANDLER(INSHL, "<<=", 0))
OPCODE_GEN(INSHAR, "inshar", N, -1, BINOP_HANDLER(INSHAR, ">>=", 0))
OPCODE_GEN(INSHLR, "inshlr", N, -1, BINOP_HANDLER(INSHLR, ">>>=", 0))

OPCODE_GEN(NE, "ne", N, -1, CMP_HANDLER(NE, "!=",
	{
		if (IVM_TYPE_OF(_TMP_OBJ1) != IVM_TYPE_OF(_TMP_OBJ2)) {
			STACK_PUSH(ivm_numeric_new(_STATE, IVM_TRUE));
			NEXT_INSTR();
		}
	}, IVM_FALSE
))

OPCODE_GEN(EQ, "eq", N, -1, CMP_HANDLER(EQ, "==",
	{
		if (IVM_TYPE_OF(_TMP_OBJ1) != IVM_TYPE_OF(_TMP_OBJ2)) {
			STACK_PUSH(ivm_numeric_new(_STATE, IVM_FALSE));
			NEXT_INSTR();
		}
	}, IVM_FALSE
))

OPCODE_GEN(GT, "gt", N, -1, CMP_HANDLER(GT, ">", 0, IVM_FALSE))
OPCODE_GEN(GE, "ge", N, -1, CMP_HANDLER(GE, ">=", 0, IVM_FALSE))
OPCODE_GEN(LT, "lt", N, -1, CMP_HANDLER(LT, "<", 0, IVM_FALSE))
OPCODE_GEN(LE, "le", N, -1, CMP_HANDLER(LE, "<=", 0, IVM_FALSE))

OPCODE_GEN(NE_R, "ne_r", N, -2, CMP_HANDLER_R(NE, "!=",
	{
		if (IVM_TYPE_OF(_TMP_OBJ1) != IVM_TYPE_OF(_TMP_OBJ2)) {
			_TMP_CMP_REG = IVM_TRUE;
			NEXT_INSTR();
		}
	}, IVM_FALSE
))

OPCODE_GEN(EQ_R, "eq_r", N, -2, CMP_HANDLER_R(EQ, "==",
	{
		if (IVM_TYPE_OF(_TMP_OBJ1) != IVM_TYPE_OF(_TMP_OBJ2)) {
			_TMP_CMP_REG = IVM_FALSE;
			NEXT_INSTR();
		}
	}, IVM_FALSE
))

OPCODE_GEN(GT_R, "gt_r", N, -2, CMP_HANDLER_R(GT, ">", 0, IVM_FALSE))
OPCODE_GEN(GE_R, "ge_r", N, -2, CMP_HANDLER_R(GE, ">=", 0, IVM_FALSE))

OPCODE_GEN(LT_R, "lt_r", N, -2, CMP_HANDLER_R(LT, "<", 0, IVM_FALSE))
OPCODE_GEN(LE_R, "le_r", N, -2, CMP_HANDLER_R(LE, "<=", 0, IVM_FALSE))

OPCODE_GEN(REMOVE_LOC, "remove_loc", N, 0, {
	_CONTEXT = ivm_runtime_removeContextNode(_RUNTIME, _STATE);
	NEXT_INSTR_NINT();
})

OPCODE_GEN(EXPAND_LOC, "expand_loc", I, 0, {
	ivm_context_expandSlotTable(_CONTEXT, _STATE, IARG());
	NEXT_INSTR();
})

/*
	top
	-------------------
	|  obj1  |  obj2  |
	-------------------

	tmp1 = pop obj1.proto
	tmp2 = pop obj2.proto

	if tmp1 == tmp2 == none:
		ret false

	while tmp2 && tmp1 != tmp2:
		tmp2 = tmp2.proto

	if tmp1 == tmp2: ret true
	else: ret false
 */
OPCODE_GEN(CHECK_PROTO, "check_proto", N, 0, {
	_TMP_OBJ1 = STACK_POP();
	_TMP_OBJ2 = STACK_POP();

	if (IVM_IS_NONE(_STATE, _TMP_OBJ1) &&
		IVM_IS_NONE(_STATE, _TMP_OBJ2)) {
		STACK_PUSH(ivm_bool_new(_STATE, IVM_TRUE));
		NEXT_INSTR();
	}

	_TMP_OBJ1 = ivm_object_getProto(_TMP_OBJ1);
	_TMP_OBJ2 = ivm_object_getProto(_TMP_OBJ2);

	while (_TMP_OBJ2) {
		if (_TMP_OBJ2 == _TMP_OBJ1) {
			STACK_PUSH(ivm_bool_new(_STATE, IVM_TRUE));
			NEXT_INSTR();
		}
		_TMP_OBJ2 = ivm_object_getProto(_TMP_OBJ2);
	}

	STACK_PUSH(ivm_bool_new(_STATE, IVM_FALSE));
	NEXT_INSTR();
})

OPCODE_GEN(ASSERT_TRUE, "assert_true", N, 0, {
	CHECK_STACK(1);
	RTM_ASSERT(ivm_object_toBool(STACK_POP(), _STATE), IVM_ERROR_MSG_ASSERT_FAILED);
	NEXT_INSTR_NINT();
})

// no pop
OPCODE_GEN(ASSERT_TRUE_N, "assert_true_n", N, 0, {
	CHECK_STACK(1);
	RTM_ASSERT(ivm_object_toBool(STACK_TOP(), _STATE), IVM_ERROR_MSG_ASSERT_FAILED);
	NEXT_INSTR_NINT();
})

/*
	assert_* is similar to get_*, but if the slot doesn't exist,
	these instrs will create a new object and set it to the slot
 */

OPCODE_GEN(ASSERT_SLOT, "assert_slot", S, 0, {
	CHECK_STACK(1);
	
	_TMP_STR = SARG();

	_TMP_OBJ1 = STACK_POP();
	NOT_NONE(_TMP_OBJ1, ivm_string_trimHead(_TMP_STR));

	_TMP_OBJ2 = ivm_object_getSlot_cc(_TMP_OBJ1, _STATE, _TMP_STR, _INSTR);

	if (!_TMP_OBJ2) {
		_TMP_OBJ2 = ivm_object_new(_STATE);
		ivm_object_setSlot_cc(_TMP_OBJ1, _STATE, _TMP_STR, _TMP_OBJ2, _INSTR);
	}

	STACK_PUSH(_TMP_OBJ2);

	NEXT_INSTR();
})

OPCODE_GEN(ASSERT_LOCAL_SLOT, "assert_local_slot", S, 1, {
	_TMP_STR = SARG();

	_TMP_OBJ1 = ivm_context_getSlot_cc(
		_CONTEXT, _STATE, _TMP_STR, _INSTR
	);

	if (!_TMP_OBJ1) {
		_TMP_OBJ1 = ivm_object_new(_STATE);
		ivm_context_setSlot_cc(_CONTEXT, _STATE, _TMP_STR, _TMP_OBJ1, _INSTR);
	}

	STACK_PUSH(_TMP_OBJ1);

	NEXT_INSTR();
})

OPCODE_GEN(GET_SLOT, "get_slot", S, 0, {
	CHECK_STACK(1);

	_TMP_OBJ1 = STACK_POP();
	NOT_NONE(_TMP_OBJ1, ivm_string_trimHead(SARG()));

	_TMP_OBJ2 = ivm_object_getSlot_cc(_TMP_OBJ1, _STATE, SARG(), _INSTR);

	if (_TMP_OBJ2) {
		STACK_PUSH(_TMP_OBJ2);
		NEXT_INSTR_NINT();
	} else {
		_TMP_OBJ2 = ivm_object_getSlot(_TMP_OBJ1, _STATE, IVM_VMSTATE_CONST(_STATE, C_NOSLOT));
		if (_TMP_OBJ2) {
			STACK_PUSH(ivm_string_object_new(_STATE, SARG()));
			STACK_PUSH(_TMP_OBJ1);
			STACK_PUSH(_TMP_OBJ2);
			INVOKE_BASE_C(1);
			// unreachable
		}

		STACK_PUSH(IVM_NONE(_STATE));
		NEXT_INSTR_NINT();
	}
})

// no pop
OPCODE_GEN(GET_SLOT_N, "get_slot_n", S, 1, {
	CHECK_STACK(1);

	_TMP_OBJ1 = STACK_TOP();
	NOT_NONE(_TMP_OBJ1, ivm_string_trimHead(SARG()));

	_TMP_OBJ2 = ivm_object_getSlot_cc(_TMP_OBJ1, _STATE, SARG(), _INSTR);

	if (_TMP_OBJ2) {
		STACK_PUSH(_TMP_OBJ2);
		NEXT_INSTR_NINT();
	} else {
		_TMP_OBJ2 = ivm_object_getSlot(_TMP_OBJ1, _STATE, IVM_VMSTATE_CONST(_STATE, C_NOSLOT));
		if (_TMP_OBJ2) {
			STACK_PUSH(ivm_string_object_new(_STATE, SARG()));
			STACK_PUSH(_TMP_OBJ1);
			STACK_PUSH(_TMP_OBJ2);
			INVOKE_BASE_C(1);
			// unreachable
		}

		STACK_PUSH(IVM_NONE(_STATE));
		NEXT_INSTR_NINT();
	}
})

OPCODE_GEN(GET_PROTO, "get_proto", N, 0, {
	CHECK_STACK(1);

	_TMP_OBJ1 = STACK_POP();
	NOT_NONE(_TMP_OBJ1, "proto");

	_TMP_OBJ1 = ivm_object_getProto(_TMP_OBJ1);
	STACK_PUSH(_TMP_OBJ1 ? _TMP_OBJ1 : IVM_NONE(_STATE));

	NEXT_INSTR_NINT();
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
	NOT_NONE(_TMP_OBJ1, ivm_string_trimHead(SARG()));

	ivm_object_setSlot_cc(_TMP_OBJ1, _STATE, SARG(), STACK_POP(), _INSTR);
	STACK_PUSH(_TMP_OBJ1);

	NEXT_INSTR();
})

/* backward */
OPCODE_GEN(SET_SLOT_B, "set_slot_b", S, -1, {
	CHECK_STACK(2);

	_TMP_OBJ2 = STACK_POP();
	_TMP_OBJ1 = STACK_TOP();
	NOT_NONE(_TMP_OBJ1, ivm_string_trimHead(SARG()));

	ivm_object_setSlot_cc(_TMP_OBJ1, _STATE, SARG(), _TMP_OBJ2, _INSTR);

	NEXT_INSTR();
})

OPCODE_GEN(SET_PROTO, "set_proto", N, -1, {
	CHECK_STACK(2);

	_TMP_OBJ1 = STACK_POP(); // obj
	_TMP_OBJ2 = STACK_POP(); // proto

	NOT_NONE(_TMP_OBJ1, "proto");

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
	_TMP_OBJ1 = ivm_context_search_cc(
		_CONTEXT, _STATE,
		SARG(), _INSTR
	);
	STACK_PUSH(_TMP_OBJ1 ? _TMP_OBJ1 : IVM_NONE(_STATE));

/*
	if (_TMP_OBJ1) {
		IVM_TRACE("%d\n", IVM_TYPE_TAG_OF(_TMP_OBJ1));
	}
*/
	// IVM_TRACE("gcs: %p %s -> %p\n", _CONTEXT, ivm_string_trimHead(SARG()), _TMP_OBJ1);

	NEXT_INSTR_NINT();
})

OPCODE_GEN(SET_CONTEXT_SLOT, "set_context_slot", S, -1, {
	_TMP_STR = SARG();

	CHECK_STACK(1);

	_TMP_OBJ1 = STACK_POP();

	if (!ivm_context_searchAndSetExistSlot_cc(
			_CONTEXT, _STATE, _TMP_STR,
			_TMP_OBJ1, _INSTR
		)) {
		/* found no existed slot, set local slot */
		ivm_context_setSlot_cc(_CONTEXT, _STATE, _TMP_STR, _TMP_OBJ1, _INSTR);
	}

	NEXT_INSTR();
})

/* loc object in ink */
OPCODE_GEN(GET_LOCAL_CONTEXT, "get_local_context", N, 1, {
	_TMP_OBJ1 = ivm_context_getObject(_CONTEXT, _STATE);
	STACK_PUSH(_TMP_OBJ1);

	NEXT_INSTR_NINT();
})

OPCODE_GEN(SET_LOCAL_CONTEXT, "set_local_context", N, -1, {
	CHECK_STACK(1);
	ivm_context_setObject(_CONTEXT, _STATE, STACK_POP());
	NEXT_INSTR();
})

/* top object in ink */
OPCODE_GEN(GET_GLOBAL_CONTEXT, "get_global_context", N, 1, {
	_TMP_OBJ1 = ivm_context_getObject(ivm_context_getGlobal(_CONTEXT), _STATE);
	STACK_PUSH(_TMP_OBJ1);

	NEXT_INSTR_NINT();
})

OPCODE_GEN(SET_GLOBAL_CONTEXT, "set_global_context", N, -1, {
	CHECK_STACK(1);
	ivm_context_setObject(ivm_context_getGlobal(_CONTEXT), _STATE, STACK_POP());
	NEXT_INSTR();
})

#if 1

OPCODE_GEN(SET_LOCAL_SLOT, "set_local_slot", S, -1, {
	CHECK_STACK(1);

	ivm_context_setSlot_cc(
		_CONTEXT,
		_STATE, SARG(), STACK_POP(),
		_INSTR
	);

	NEXT_INSTR();
})

OPCODE_GEN(GET_LOCAL_SLOT, "get_local_slot", S, 1, {
	_TMP_OBJ1 = ivm_context_getSlot_cc(
		_CONTEXT,
		_STATE, SARG(), _INSTR
	);

	STACK_PUSH(_TMP_OBJ1 ? _TMP_OBJ1 : IVM_NONE(_STATE));

	NEXT_INSTR_NINT();
})

OPCODE_GEN(SET_GLOBAL_SLOT, "set_global_slot", S, -1, {
	CHECK_STACK(1);

	ivm_context_setSlot_cc(
		ivm_context_getGlobal(_CONTEXT),
		_STATE, SARG(), STACK_POP(),
		_INSTR
	);

	NEXT_INSTR();
})

OPCODE_GEN(GET_GLOBAL_SLOT, "get_global_slot", S, 1, {
	_TMP_OBJ1 = ivm_context_getSlot_cc(
		ivm_context_getGlobal(_CONTEXT),
		_STATE, SARG(), _INSTR
	);

	STACK_PUSH(_TMP_OBJ1 ? _TMP_OBJ1 : IVM_NONE(_STATE));

	NEXT_INSTR_NINT();
})

#endif

OPCODE_GEN(SET_PA_ARG, "set_pa_arg", I, 0, {
	_TMP_ARGC = IARG();

	if (AVAIL_STACK >= _TMP_ARGC) {
		// too many arguments => trim the head
		_TMP_ARGC = AVAIL_STACK - _TMP_ARGC;
		STACK_TRIM_HEAD(_TMP_ARGC);
	} else {
		// too little argument => fill in none
		STACK_ENSURE(_TMP_ARGC);

		_TMP_ARGC = _TMP_ARGC - AVAIL_STACK;
		STACK_MOVE(_TMP_ARGC);

		_TMP_OBJ1 = IVM_NONE(_STATE);
		STACK_FILLIN(_BP, _TMP_OBJ1, _TMP_ARGC);
	}

	NEXT_INSTR();
})

OPCODE_GEN(SET_ARG, "set_arg", S, -1, {
	ivm_context_setSlot_cc(
		_CONTEXT,
		_STATE, SARG(), HAS_STACK ? STACK_POP() : IVM_NONE(_STATE),
		_INSTR
	);

	NEXT_INSTR();
})

// set default
OPCODE_GEN(SET_DEF, "set_def", S, -2, {
	switch (AVAIL_STACK) {
		case 1:
			ivm_context_setSlot_cc(_CONTEXT, _STATE, SARG(), STACK_POP(), _INSTR);
			break;
		case 0: INSUF_STACK(1); break;
		default:
			_TMP_OBJ1 = STACK_POP(); // default
			_TMP_OBJ2 = STACK_POP();

			if (IVM_IS_NONE(_STATE, _TMP_OBJ2)) {
				ivm_context_setSlot_cc(_CONTEXT, _STATE, SARG(), _TMP_OBJ1, _INSTR);
			} else {
				ivm_context_setSlot_cc(_CONTEXT, _STATE, SARG(), _TMP_OBJ2, _INSTR);
			}
			break;
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

	NOT_NONE(_TMP_OBJ1, "overload op");

	ivm_object_setOop(_TMP_OBJ1, _STATE, IARG(), _TMP_OBJ2);

	STACK_PUSH(_TMP_OBJ1);

	NEXT_INSTR();
})

/* backward */
OPCODE_GEN(SET_OOP_B, "set_oop_b", I, -1, {
	CHECK_STACK(2);

	_TMP_OBJ2 = STACK_POP();
	_TMP_OBJ1 = STACK_TOP();

	NOT_NONE(_TMP_OBJ1, "overload op");

	ivm_object_setOop(_TMP_OBJ1, _STATE, IARG(), _TMP_OBJ2);

	NEXT_INSTR();
})

OPCODE_GEN(GET_OOP, "get_oop", I, -1, {
	CHECK_STACK(1);

	_TMP_ARGC = IARG();
	_TMP_OBJ1 = STACK_POP();
	NOT_NONE(_TMP_OBJ1, "overload op");

	_TMP_OBJ2 = ivm_object_getOop(_TMP_OBJ1, _TMP_ARGC);
	if (!_TMP_OBJ2) {
		if (IVM_IS_BTTYPE(_TMP_OBJ1, _STATE, IVM_FUNCTION_OBJECT_T) &&
			_TMP_ARGC == IVM_OOP_ID(CALL)) {
			_TMP_OBJ2 = _TMP_OBJ1;
		} else {
			_TMP_FUNC = ivm_type_getDefaultOop(IVM_TYPE_OF(_TMP_OBJ1), _TMP_ARGC);
			// IVM_TRACE("%d\n", _TMP_ARGC);
			_TMP_OBJ2 = _TMP_FUNC ? ivm_function_object_new(_STATE, IVM_NULL, _TMP_FUNC) : IVM_NONE(_STATE);
		}
	}

	STACK_PUSH(_TMP_OBJ2);

	NEXT_INSTR_NINT();
})

OPCODE_GEN(POP, "pop", N, -1, {
	CHECK_STACK(1);
	STACK_POP();
	NEXT_INSTR_NINT();
})

OPCODE_GEN(POP_N, "pop_n", I, -1, {
	_TMP_ARGC = IARG();
	CHECK_STACK(_TMP_ARGC);
	STACK_CUT(_TMP_ARGC);
	NEXT_INSTR_NINT();
})

OPCODE_GEN(POP_ALL, "pop_all", N, -1, {
	STACK_SET(0);
	NEXT_INSTR_NINT();
})

OPCODE_GEN(DUP_N, "dup_n", I, 1, {
	_TMP_ARGC = IARG();

	CHECK_STACK(_TMP_ARGC + 1);

	_TMP_OBJ1 = STACK_BEFORE(_TMP_ARGC);
	STACK_PUSH(_TMP_OBJ1);

	NEXT_INSTR_NINT();
})

OPCODE_GEN(DUP_ABS, "dup_abs", I, 1, {
	STACK_PUSH(*(_BP + IARG()));
	NEXT_INSTR_NINT();
})

OPCODE_GEN(DUP, "dup", N, 1, {
	CHECK_STACK(1);

	_TMP_OBJ1 = STACK_TOP();
	STACK_PUSH(_TMP_OBJ1);

	NEXT_INSTR_NINT();
})

/* duplicate the top element in the nth previous block */
OPCODE_GEN(DUP_PREV_BLOCK, "dup_prev_block", I, 1, {
	_TMP_ARGC = IARG();
	
	// RTM_ASSERT(ivm_runtime_hasNBlock(_RUNTIME, _TMP_ARGC), IVM_ERROR_MSG_NO_ENOUGH_BLOCK);

	_TMP_OBJ1 = STACK_PREV_BLOCK_TOP(_TMP_ARGC);
	STACK_PUSH(_TMP_OBJ1);

	NEXT_INSTR_NINT();
})

/* duplicate the nth element in the 1st previous block, n = 0 means the stack top */
OPCODE_GEN(DUP_PREV_BLOCK_N, "dup_prev_block_n", I, 1, {
	_TMP_ARGC = IARG();
	
	// RTM_ASSERT(ivm_runtime_hasNBlock(_RUNTIME, _TMP_ARGC), IVM_ERROR_MSG_NO_ENOUGH_BLOCK);

	_TMP_OBJ1 = STACK_PREV_BLOCK_N(_TMP_ARGC);
	STACK_PUSH(_TMP_OBJ1);

	NEXT_INSTR_NINT();
})

#if 0

OPCODE_GEN(OUT, "out", S, 0, {
	IVM_TRACE("%s\n", ivm_string_trimHead(SARG()));
	NEXT_INSTR_NINT();
})

OPCODE_GEN(OUT_NUM, "out_num", N, 0, {
	CHECK_STACK(1);

	_TMP_OBJ1 = STACK_TOP();
	if (IVM_OBJECT_GET(_TMP_OBJ1, TYPE_TAG) == IVM_NUMERIC_T) {
		IVM_TRACE("%.3f\n", IVM_AS(_TMP_OBJ1, ivm_numeric_t)->val);
	} else {
		IVM_TRACE("cannot print number of object of type <%s>\n", IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME));
	}

	NEXT_INSTR_NINT();
})

OPCODE_GEN(OUT_STR, "out_str", N, 0, {
	CHECK_STACK(1);

	_TMP_OBJ1 = STACK_TOP();
	if (IVM_OBJECT_GET(_TMP_OBJ1, TYPE_TAG) == IVM_STRING_OBJECT_T) {
		IVM_TRACE("%s\n", ivm_string_trimHead(IVM_AS(_TMP_OBJ1, ivm_string_object_t)->val));
	} else {
		IVM_TRACE("cannot print string of object of type <%s>\n", IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME));
	}

	NEXT_INSTR_NINT();
})

OPCODE_GEN(OUT_STACK_SIZE, "out_stack_size", N, 0, {
	IVM_TRACE("%ld\n", STACK_SIZE());
	NEXT_INSTR_NINT();
})

OPCODE_GEN(OUT_TYPE, "out_type", N, 0, {
	CHECK_STACK(1);

	_TMP_OBJ1 = STACK_TOP();
	IVM_TRACE("%s\n", _TMP_OBJ1 ? IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME) : "<null pointer>");
	
	NEXT_INSTR_NINT();
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

#include "opcode.def.invoke.h"

OPCODE_GEN(INVOKE, "invoke", I, -(IVM_OPCODE_VARIABLE_STACK_INC), {
	INVOKE_C(IARG());
})

OPCODE_GEN(INVOKE_BASE, "invoke_base", I, -(1 + IVM_OPCODE_VARIABLE_STACK_INC), {
	INVOKE_BASE_C(IARG());
})

OPCODE_GEN(INVOKE_VAR, "invoke_var", N, -1, {
	CHECK_STACK(1);
	SET_IARG(AVAIL_STACK - 1);
	GOTO_INSTR(INVOKE);
})

OPCODE_GEN(INVOKE_BASE_VAR, "invoke_base_var", N, -1, {
	CHECK_STACK(2);
	SET_IARG(AVAIL_STACK - 2);
	GOTO_INSTR(INVOKE_BASE);
})

OPCODE_GEN(FORK, "fork", N, 0, {
	CHECK_STACK(1);

	_TMP_OBJ1 = STACK_POP();

	RTM_ASSERT(IVM_IS_BTTYPE(_TMP_OBJ1, _STATE, IVM_FUNCTION_OBJECT_T),
			   IVM_ERROR_MSG_NOT_TYPE("function", IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME)));

	_TMP_CORO = ivm_coro_new(_STATE);
	ivm_coro_setRoot(_TMP_CORO, _STATE, IVM_AS(_TMP_OBJ1, ivm_function_object_t));

	_TMP_OBJ2 = ivm_coro_object_new(_STATE, _TMP_CORO);
	STACK_PUSH(_TMP_OBJ2);

	NEXT_INSTR_NINT();
})

OPCODE_GEN(YIELD, "yield", N, 0, {
	CHECK_STACK(1);
	
	_TMP_OBJ1 = STACK_POP();

	RTM_ASSERT(!IVM_CORO_GET(_CORO, HAS_NATIVE),
			   IVM_ERROR_MSG_YIELD_ATOM_CORO);

	INC_INSTR();
	SAVE_RUNTIME(_INSTR);
	
	YIELD();
})

/*
	top
	------------------
	|  coro  |  arg  |
	------------------
 */
OPCODE_GEN(RESUME, "resume", N, -1, {
	CHECK_STACK(2);

	_TMP_OBJ1 = STACK_POP();
	_TMP_OBJ2 = STACK_TOP(); // leave the coro object on the stack

	// IVM_TRACE("what?\n");

	RTM_ASSERT(IVM_IS_BTTYPE(_TMP_OBJ2, _STATE, IVM_CORO_OBJECT_T),
			   IVM_ERROR_MSG_RESUME_NON_CORO(IVM_OBJECT_GET(_TMP_OBJ2, TYPE_NAME)));

	_TMP_CORO = ivm_coro_object_getCoro(_TMP_OBJ2);

	RTM_ASSERT(ivm_coro_canResume(_TMP_CORO), IVM_ERROR_MSG_CORO_UNABLE_RESUME(_TMP_CORO));

	SAVE_RUNTIME(_INSTR);
	
	// _TMP_OBJ1 = ivm_vmstate_schedule_g(_STATE, _TMP_OBJ1, _TMP_CGID);
	_TMP_OBJ1 = ivm_coro_resume(_TMP_CORO, _STATE, _TMP_OBJ1);

	UPDATE_RUNTIME();
	STACK_POP();
	STACK_PUSH(_TMP_OBJ1 ? _TMP_OBJ1 : IVM_NONE(_STATE));

	NEXT_INSTR();
})

#if 0

OPCODE_GEN(FORK, "fork", N, -1, {
	CHECK_STACK(1);

	_TMP_OBJ1 = STACK_POP();

	RTM_ASSERT(IVM_IS_BTTYPE(_TMP_OBJ1, _STATE, IVM_FUNCTION_OBJECT_T),
			   IVM_ERROR_MSG_NOT_TYPE("function", IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME)));

	ivm_vmstate_addCoroToCurCGroup(
		_STATE,
		IVM_AS(_TMP_OBJ1, ivm_function_object_t)
	);

	NEXT_INSTR_NINT();
})

OPCODE_GEN(GROUP, "group", N, 0, {
	CHECK_STACK(1);

	_TMP_OBJ1 = STACK_POP();

	RTM_ASSERT(IVM_IS_BTTYPE(_TMP_OBJ1, _STATE, IVM_FUNCTION_OBJECT_T),
			   IVM_ERROR_MSG_NOT_TYPE("function", IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME)));

	STACK_PUSH(
		ivm_numeric_new(_STATE,
			ivm_vmstate_addCGroup(
				_STATE,
				IVM_AS(_TMP_OBJ1, ivm_function_object_t)
			)
		)
	);

	NEXT_INSTR_NINT();
})

OPCODE_GEN(GROUP_TO, "group_to", N, -1, {
	CHECK_STACK(2);

	_TMP_OBJ1 = STACK_POP();
	_TMP_OBJ2 = STACK_POP();

	RTM_ASSERT(IVM_IS_BTTYPE(_TMP_OBJ1, _STATE, IVM_FUNCTION_OBJECT_T),
			   IVM_ERROR_MSG_NOT_TYPE("function", IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME)));

	RTM_ASSERT(IVM_IS_BTTYPE(_TMP_OBJ2, _STATE, IVM_NUMERIC_T),
			   IVM_ERROR_MSG_ILLEGAL_GID_TYPE(IVM_OBJECT_GET(_TMP_OBJ2, TYPE_NAME)));

	_TMP_CGID = ivm_numeric_getValue(_TMP_OBJ2);
	CHECK_CGID();

	STACK_PUSH(
		ivm_numeric_new(_STATE,
			ivm_vmstate_addCoro(
				_STATE,
				IVM_AS(_TMP_OBJ1, ivm_function_object_t),
				_TMP_CGID
			)
		)
	);

	NEXT_INSTR();
})

OPCODE_GEN(YIELD, "yield", N, 0, {
	CHECK_STACK(1);
	
	_TMP_OBJ1 = STACK_POP();

	RTM_ASSERT(!IVM_CORO_GET(_CORO, HAS_NATIVE),
			   IVM_ERROR_MSG_YIELD_ATOM_CORO);

	INC_INSTR();
	SAVE_RUNTIME(_INSTR);
	
	YIELD();
})

OPCODE_GEN(RESUME, "resume", N, -1, {
	CHECK_STACK(2);

	_TMP_OBJ2 = STACK_POP();
	_TMP_OBJ1 = STACK_POP();

	RTM_ASSERT(IVM_IS_BTTYPE(_TMP_OBJ2, _STATE, IVM_NUMERIC_T),
			   IVM_ERROR_MSG_ILLEGAL_GID_TYPE(IVM_OBJECT_GET(_TMP_OBJ2, TYPE_NAME)));

	_TMP_CGID = ivm_numeric_getValue(_TMP_OBJ2);

	CHECK_CGID();

	RTM_ASSERT(!ivm_vmstate_isCGroupLocked(_STATE, _TMP_CGID),
			   IVM_ERROR_MSG_CORO_GROUP_SUSPENDED(_TMP_CGID));

	SAVE_RUNTIME(_INSTR);
	
	_TMP_OBJ1 = ivm_vmstate_schedule_g(_STATE, _TMP_OBJ1, _TMP_CGID);

	UPDATE_RUNTIME();
	STACK_PUSH(_TMP_OBJ1 ? _TMP_OBJ1 : IVM_NONE(_STATE));
	NEXT_INSTR();
})

#endif

OPCODE_GEN(ITER_NEXT, "iter_next", A, 2, {
	CHECK_STACK(1);
	STACK_SET(1); // pop all elem but one

	_TMP_OBJ1 = STACK_TOP(); // iter
	_TMP_OBJ2 = ivm_object_getSlot_cc(_TMP_OBJ1, _STATE, IVM_VMSTATE_CONST(_STATE, C_NEXT), _INSTR);
	RTM_ASSERT(_TMP_OBJ2, IVM_ERROR_MSG_NON_ITERABLE);

	if (IVM_IS_BTTYPE(_TMP_OBJ1, _STATE, IVM_LIST_OBJECT_ITER_T) &&
		IVM_IS_BTTYPE(_TMP_OBJ2, _STATE, IVM_FUNCTION_OBJECT_T) &&
		ivm_function_object_checkNative(_TMP_OBJ2, _list_iter_next)) {

		_TMP_OBJ2 = ivm_list_object_iter_next(
			IVM_AS(_TMP_OBJ1, ivm_list_object_iter_t), _STATE
		);

		if (_TMP_OBJ2) {
			STACK_PUSH(_TMP_OBJ2);
			NEXT_N_INSTR(3);
		} else {
			GOTO_SET_INSTR(ADDR_ARG());
		}
	} else if (IVM_IS_BTTYPE(_TMP_OBJ1, _STATE, IVM_RANGE_ITER_T) &&
			   IVM_IS_BTTYPE(_TMP_OBJ2, _STATE, IVM_FUNCTION_OBJECT_T) &&
			   ivm_function_object_checkNative(_TMP_OBJ2, _range_iter_next)) {

		_TMP_OBJ2 = ivm_range_iter_next(IVM_AS(_TMP_OBJ1, ivm_range_iter_t), _STATE);

		if (_TMP_OBJ2) {
			STACK_PUSH(_TMP_OBJ2);
			NEXT_N_INSTR(3);
		} else {
			GOTO_SET_INSTR(ADDR_ARG());
		}
	} else {
		ivm_coro_setCurCatch(_BLOCK_STACK, _RUNTIME, ADDR_ARG());

		// IVM_TRACE("%ld %p %p\n", AVAIL_STACK, tmp_sp, tmp_bp);

		STACK_PUSH(_TMP_OBJ1);
		STACK_PUSH(_TMP_OBJ2); // get iter.next

		NEXT_INSTR_NINT();
	}
})

/*
	top
	--------------
	| list | obj |
	--------------  
 */
OPCODE_GEN(PUSH_LIST, "push_list", N, -2, {
	CHECK_STACK(2);

	_TMP_OBJ1 = STACK_POP();
	_TMP_OBJ2 = STACK_POP();

	RTM_ASSERT(IVM_IS_BTTYPE(_TMP_OBJ1, _STATE, IVM_LIST_OBJECT_T),
			   IVM_ERROR_MSG_NOT_TYPE("list", IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME)));

	if (!ivm_list_object_push(IVM_AS(_TMP_OBJ1, ivm_list_object_t), _STATE, _TMP_OBJ2)) {
		EXCEPTION();
	}

	NEXT_INSTR();
})

/*
	raise protection set/cancel
 */
OPCODE_GEN(RPROT_SET, "rprot_set", A, 0, {
	ivm_coro_setCurCatch(_BLOCK_STACK, _RUNTIME, ADDR_ARG());
	NEXT_INSTR_NINT();
})

OPCODE_GEN(RPROT_CAC, "rprot_cac", N, 0, {
	ivm_coro_setCurCatch(_BLOCK_STACK, _RUNTIME, IVM_NULL);
	NEXT_INSTR_NINT();
})

OPCODE_GEN(RAISE, "raise", N, -1, {
	CHECK_STACK(1);
	_TMP_OBJ1 = STACK_POP();
	RAISE(_TMP_OBJ1);
})

OPCODE_GEN(POP_EXC, "pop_exc", N, -1, {
	if (AVAIL_STACK) {
		_TMP_OBJ1 = STACK_POP();
		RAISE(_TMP_OBJ1);
		NEXT_INSTR_NINT();
	}
})

OPCODE_GEN(PUSH_BLOCK, "push_block", N, 0, {
	SAVE_STACK();
	// SET_BP(ivm_runtime_pushBlock(_RUNTIME, _STATE, AVAIL_STACK));
	SET_BP(ivm_coro_pushBlock(_BLOCK_STACK, _RUNTIME, AVAIL_STACK));
	NEXT_INSTR_NINT();
})

OPCODE_GEN(PUSH_BLOCK_S1, "push_block_s1", N, 0, {
	CHECK_STACK(1);

	SAVE_STACK();
	SET_BP(ivm_coro_pushBlock_c(_BLOCK_STACK, _RUNTIME, AVAIL_STACK - 1));
	
	NEXT_INSTR_NINT();
})

OPCODE_GEN(PUSH_BLOCK_AT, "push_block_at", I, 0, {
	SAVE_STACK();
	SET_BP(ivm_coro_pushBlock_c(_BLOCK_STACK, _RUNTIME, IARG()));

	NEXT_INSTR_NINT();
})

OPCODE_GEN(POP_BLOCK, "pop_block", N, 0, {
	SAVE_STACK();
	// ivm_runtime_popBlock(_RUNTIME);
	ivm_coro_popBlock(_BLOCK_STACK, _RUNTIME);
	UPDATE_STACK_C();
	NEXT_INSTR_NINT();
})

// save original stack
OPCODE_GEN(POP_BLOCK_S, "pop_block_s", I, 0, {
	_TMP_ARGC = IARG();
	CHECK_STACK(_TMP_ARGC);

	SAVE_STACK();
	ivm_coro_popBlock(_BLOCK_STACK, _RUNTIME);
	UPDATE_STACK_C();
	STACK_INC_C(_TMP_ARGC);

	NEXT_INSTR_NINT();
})

// save 1 original stack
OPCODE_GEN(POP_BLOCK_S1, "pop_block_s1", N, 0, {
	CHECK_STACK(1);

	SAVE_STACK();
	ivm_coro_popBlock(_BLOCK_STACK, _RUNTIME);
	UPDATE_STACK_C();
	STACK_INC_C(1);

	NEXT_INSTR_NINT();
})

OPCODE_GEN(RETURN, "return", N, -1, {
	if (AVAIL_STACK) {
		_TMP_OBJ1 = STACK_POP();
	} else {
		_TMP_OBJ1 = IVM_NONE(_STATE);
	}

	RETURN();
})

OPCODE_GEN(JUMP, "jump", A, 0, {
	GOTO_SET_INSTR(ADDR_ARG());
})

#if 0

OPCODE_GEN(INT_LOOP, "int_loop", A, 0, {
	SAVE_STACK();
	ivm_coro_popAllCatch(_BLOCK_STACK, _RUNTIME);
	UPDATE_STACK_C();
	GOTO_SET_INSTR(ADDR_ARG());
})

// pop n normal blocks(not include the blocks with catches)
OPCODE_GEN(INT_N_LOOP, "int_n_loop", I, 0, {
	SAVE_STACK();

	_TMP_ARGC = IARG();

	while (_TMP_ARGC--) {
		ivm_coro_popAllCatch(_BLOCK_STACK, _RUNTIME);
		ivm_coro_popBlock(_BLOCK_STACK, _RUNTIME);
	}

	ivm_coro_popAllCatch(_BLOCK_STACK, _RUNTIME);

	UPDATE_STACK_C();

	NEXT_INSTR();
})

#endif

OPCODE_GEN(INT_LOOP, "int_loop", I, 0, {
	SAVE_STACK();

	_TMP_ARGC = IARG();

	while (_TMP_ARGC--) {
		ivm_coro_popBlock(_BLOCK_STACK, _RUNTIME);
	}

	UPDATE_STACK_C();

	NEXT_INSTR();
})

OPCODE_GEN(JUMP_TRUE, "jump_true", A, -1, {
	CHECK_STACK(1);

	if (ivm_object_toBool(STACK_POP(), _STATE)) {
		GOTO_SET_INSTR(ADDR_ARG());
	} else {
		NEXT_INSTR_NINT();
	}
})

OPCODE_GEN(JUMP_FALSE, "jump_false", A, -1, {
	CHECK_STACK(1);

	if (!ivm_object_toBool(STACK_POP(), _STATE)) {
		GOTO_SET_INSTR(ADDR_ARG());
	} else {
		NEXT_INSTR_NINT();
	}
})

#if 1

OPCODE_GEN(JUMP_TRUE_R, "jump_true_r", A, 0, {
	if (IVM_LIKELY(_USE_REG)) {
		_USE_REG = IVM_FALSE;
		if (_TMP_CMP_REG) {
			GOTO_SET_INSTR(ADDR_ARG());
		} else {
			NEXT_INSTR_NINT();
		}
	} else {
		GOTO_INSTR(JUMP_TRUE);
	}
})

OPCODE_GEN(JUMP_FALSE_R, "jump_false_r", A, 0, {
	if (IVM_LIKELY(_USE_REG)) {
		_USE_REG = IVM_FALSE;
		if (!_TMP_CMP_REG) {
			GOTO_SET_INSTR(ADDR_ARG());
		} else {
			NEXT_INSTR_NINT();
		}
	} else {
		GOTO_INSTR(JUMP_FALSE);
	}
})

#endif

/* no pop */
OPCODE_GEN(JUMP_TRUE_N, "jump_true_n", A, -1, {
	CHECK_STACK(1);

	if (ivm_object_toBool(STACK_TOP(), _STATE)) {
		GOTO_SET_INSTR(ADDR_ARG());
	} else {
		NEXT_INSTR_NINT();
	}
})

OPCODE_GEN(JUMP_FALSE_N, "jump_false_n", A, -1, {
	CHECK_STACK(1);

	if (!ivm_object_toBool(STACK_TOP(), _STATE)) {
		GOTO_SET_INSTR(ADDR_ARG());
	} else {
		NEXT_INSTR_NINT();
	}
})

/* stack empty => goto the addr */
OPCODE_GEN(CHECK, "check", A, 0, {
	if (AVAIL_STACK) {
		GOTO_SET_INSTR(ADDR_ARG());
	} else {
		NEXT_INSTR_NINT();
	}
})

/* stack size < 2 => goto the addr */
OPCODE_GEN(CHECK2, "check2", A, 0, {
	if (AVAIL_STACK < 2) {
		GOTO_SET_INSTR(ADDR_ARG());
	} else {
		NEXT_INSTR_NINT();
	}
})

/* if the number of stack is less than what has expected, an exception will be thrown */
OPCODE_GEN(CHECK_E, "check_e", I, 0, {
	RTM_ASSERT(AVAIL_STACK >= IARG(), IVM_ERROR_MSG_TOO_LESS_ARG(AVAIL_STACK, IARG()));
	NEXT_INSTR_NINT();
})

OPCODE_GEN(CHECK_LIST_ITER, "check_list_iter", A, 0, {
	CHECK_STACK(1);

	_TMP_OBJ1 = STACK_TOP();
	_TMP_OBJ2 = ivm_object_getSlot_cc(_TMP_OBJ1, _STATE, IVM_VMSTATE_CONST(_STATE, C_NEXT), _INSTR);
	RTM_ASSERT(_TMP_OBJ2, IVM_ERROR_MSG_NON_ITERABLE);

	if (IVM_IS_BTTYPE(_TMP_OBJ1, _STATE, IVM_LIST_OBJECT_ITER_T) &&
		IVM_IS_BTTYPE(_TMP_OBJ2, _STATE, IVM_FUNCTION_OBJECT_T) &&
		ivm_function_object_checkNative(_TMP_OBJ2, _list_iter_next)) {
		NEXT_INSTR_NINT();
	} else {
		GOTO_SET_INSTR(ADDR_ARG());
	}
})

OPCODE_GEN(INTR, "intr", N, 0, {
	if (IVM_GC_DBG) {
		ivm_vmstate_doGC(_STATE);
		INT_RET();
	}

	_TMP_INT = ivm_vmstate_popInt(_STATE);

	if (_TMP_INT == IVM_CORO_INT_GC) {
		ivm_vmstate_doGC(_STATE);
	} else if (!_ivm_coro_otherInt(_STATE, _TMP_INT)) {
		RTM_FATAL(IVM_ERROR_MSG_BAD_INT_FLAG(_TMP_INT));
	}

	INT_RET();
})
