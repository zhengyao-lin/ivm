OPCODE_GEN(NOP, "nop", N, {
	NEXT_INSTR();
})

OPCODE_GEN(NEW_NULL, "new_null", N, {
	STACK_PUSH(IVM_NULL_OBJ(_STATE));
	NEXT_INSTR();
})

OPCODE_GEN(NEW_OBJ, "new_obj", N, {
	STACK_PUSH(ivm_object_new(_STATE));
	NEXT_INSTR();
})

OPCODE_GEN(NEW_NUM_I, "new_num_i", I, {
	STACK_PUSH(ivm_numeric_new(_STATE, _ARG));
	NEXT_INSTR();
})

OPCODE_GEN(NEW_NUM_F, "new_num_f", F, {
	STACK_PUSH(ivm_numeric_new(_STATE, _FARG));
	NEXT_INSTR();
})

/*
OPCODE_GEN(NEW_NUM_S, "new_num_s", S, {
	NEXT_INSTR();
})
*/

OPCODE_GEN(NEW_STR, "new_str", S, {
	STACK_PUSH(ivm_string_object_new(_STATE, ivm_string_pool_get(_STRING_POOL, _ARG)));
	NEXT_INSTR();
})

OPCODE_GEN(NEW_FUNC, "new_func", I, {
	STACK_PUSH(ivm_function_object_new(_STATE, _CONTEXT,
									   ivm_vmstate_getFunc(_STATE, _ARG)));
	NEXT_INSTR();
})

/* unary operations */
OPCODE_GEN(NOT, "not", N, DEFAULT_UNIOP_HANDLER(NOT, "!"))

/* binary operations */
OPCODE_GEN(ADD, "add", N, DEFAULT_BINOP_HANDLER(ADD, "+"))
OPCODE_GEN(SUB, "sub", N, DEFAULT_BINOP_HANDLER(SUB, "-"))
OPCODE_GEN(MUL, "mul", N, DEFAULT_BINOP_HANDLER(MUL, "*"))
OPCODE_GEN(DIV, "div", N, DEFAULT_BINOP_HANDLER(DIV, "/"))
OPCODE_GEN(MOD, "mod", N, DEFAULT_BINOP_HANDLER(MOD, "%"))

OPCODE_GEN(LT, "lt", N, CMP_BINOP_HANDLER(
	STACK_PUSH(ivm_numeric_new(_STATE, _RETVAL < 0));
	NEXT_INSTR();
))

OPCODE_GEN(JUMP_LT, "jump_lt", I, CMP_BINOP_HANDLER(
	if (_RETVAL < 0) {
		NEXT_N_INSTR(_ARG);
	} else {
		NEXT_INSTR();
	}
))

OPCODE_GEN(GET_SLOT, "get_slot", S, {
	ivm_object_t *obj;

	CHECK_STACK(1);

	obj = ivm_object_getSlotValue(STACK_POP(), _STATE,
								  ivm_string_pool_get(_STRING_POOL, _ARG));
	
	STACK_PUSH(obj ? obj : IVM_UNDEFINED(_STATE));

	NEXT_INSTR();
})

OPCODE_GEN(SET_SLOT, "set_slot", S, {
	ivm_object_t *obj;

	CHECK_STACK(2);

	obj = STACK_POP();
	
	ivm_object_setSlot(obj, _STATE,
					   ivm_string_pool_get(_STRING_POOL, _ARG),
					   STACK_POP());
	
	STACK_PUSH(obj);

	NEXT_INSTR();
})

OPCODE_GEN(GET_CONTEXT_SLOT, "get_context_slot", S, {
	ivm_object_t *found =
				  ivm_ctchain_search(_CONTEXT, _STATE,
									 ivm_string_pool_get(_STRING_POOL, _ARG));

	STACK_PUSH(found ? found : IVM_UNDEFINED(_STATE));
	NEXT_INSTR();
})

OPCODE_GEN(SET_CONTEXT_SLOT, "set_context_slot", S, {
	const ivm_string_t *key = ivm_string_pool_get(_STRING_POOL, _ARG);
	ivm_object_t *val;

	CHECK_STACK(1);

	val = STACK_POP();

	if (!ivm_ctchain_setSlotIfExist(_CONTEXT, _STATE, key, val)) {
		ivm_ctchain_setLocalSlot(_CONTEXT, _STATE, key, val);
	}

	NEXT_INSTR();
})

OPCODE_GEN(SET_ARG, "set_arg", S, {
	ivm_ctchain_setLocalSlot(_CONTEXT, _STATE,
							 ivm_string_pool_get(_STRING_POOL, _ARG),
							 AVAIL_STACK >= 1
							 ? STACK_POP()
							 : IVM_UNDEFINED(_STATE));

	NEXT_INSTR();
})

OPCODE_GEN(POP, "pop", N, {
	CHECK_STACK(1);
	STACK_POP();

	NEXT_INSTR();
})

OPCODE_GEN(DUP, "dup", I, {
	ivm_size_t i = _ARG;
	CHECK_STACK(i + 1);
	STACK_PUSH(STACK_BEFORE(i));

	NEXT_INSTR();
})

OPCODE_GEN(PRINT_OBJ, "print_obj", N, {
	ivm_object_t *obj;

	CHECK_STACK(1);

	obj = STACK_POP();
	IVM_OUT("print: %p\n", (void *)obj);

	NEXT_INSTR();
})

OPCODE_GEN(PRINT_NUM, "print_num", N, {
	ivm_object_t *obj;

	CHECK_STACK(1);

	obj = STACK_POP();
	if (IVM_OBJECT_GET(obj, TYPE_TAG) == IVM_NUMERIC_T)
		IVM_TRACE("print num: %f\n", IVM_AS(obj, ivm_numeric_t)->val);
	else
		IVM_TRACE("cannot print number of object of type <%s>\n", IVM_OBJECT_GET(obj, TYPE_NAME));

	NEXT_INSTR();
})

OPCODE_GEN(PRINT_TYPE, "print_type", N, {
	ivm_object_t *obj;

	CHECK_STACK(1);

	obj = STACK_POP();
	IVM_TRACE("type: %s\n", obj ? IVM_OBJECT_GET(obj, TYPE_NAME) : "empty pointer");
	
	NEXT_INSTR();
})

OPCODE_GEN(PRINT_STR, "print_str", N, {
	ivm_string_object_t *str;

	CHECK_STACK(1);

	str = IVM_AS(STACK_POP(), ivm_string_object_t);

	IVM_ASSERT(IVM_IS_TYPE(str, IVM_STRING_OBJECT_T),
			   IVM_ERROR_MSG_NOT_TYPE("string", IVM_OBJECT_GET(str, TYPE_NAME)));

	IVM_OUT("%s\n", ivm_string_trimHead(str->val));

	NEXT_INSTR();
})

OPCODE_GEN(PRINT_STACK, "print_stack", N, {
	ivm_dbg_stackState(_CORO, stderr);
	NEXT_INSTR();
})

OPCODE_GEN(OUT, "out", S, {
	IVM_TRACE("%s\n", ivm_string_trimHead(ivm_string_pool_get(_STRING_POOL, _ARG)));
	NEXT_INSTR();
})

OPCODE_GEN(OUT_NUM, "out_num", N, {
	ivm_object_t *obj;

	CHECK_STACK(1);

	obj = STACK_POP();
	if (IVM_OBJECT_GET(obj, TYPE_TAG) == IVM_NUMERIC_T)
		IVM_TRACE("print num: %f\n", IVM_AS(obj, ivm_numeric_t)->val);
	else
		IVM_TRACE("cannot print number of object of type <%s>\n", IVM_OBJECT_GET(obj, TYPE_NAME));

	NEXT_INSTR();
})

OPCODE_GEN(INVOKE, "invoke", I, {
	ivm_function_object_t *obj;
	ivm_function_t *func;
	ivm_sint32_t arg_count = _ARG;
	ivm_object_t **args;
	ivm_object_t *ret;

	CHECK_STACK(arg_count + 1);

	obj = IVM_AS(STACK_POP(), ivm_function_object_t);
	// IVM_TRACE("%p\n", obj);

	IVM_ASSERT(IVM_IS_TYPE(obj, IVM_FUNCTION_OBJECT_T),
			   IVM_ERROR_MSG_NOT_TYPE("function", IVM_OBJECT_GET(obj, TYPE_NAME)));
	
	func = ivm_function_object_getFunc(obj);
	args = STACK_CUT(arg_count);

	/* IVM_RUNTIME_SET(_RUNTIME, IP, _INSTR + 1); */
	SAVE_RUNTIME(_INSTR + 1);

	ivm_function_invoke(func, _STATE,
						ivm_function_object_getClosure(obj), _CORO);
	
	UPDATE_STACK();

	if (ivm_function_isNative(func)) {
		IVM_PER_INSTR_DBG(DBG_RUNTIME_ACTION(INVOKE, 1 /* native invoke */));

		ret = ivm_function_callNative(func, _STATE, _CONTEXT,
									  IVM_FUNCTION_SET_ARG_2(arg_count, args));
		STACK_PUSH(ret ? ret : IVM_NULL_OBJ(_STATE));
	} else {
		IVM_PER_INSTR_DBG(DBG_RUNTIME_ACTION(INVOKE, IVM_NULL));

		STACK_INC(arg_count);
	}

	INVOKE();
})

OPCODE_GEN(YIELD, "yield", N, {
	YIELD();
})

OPCODE_GEN(RETURN, "return", N, {
	RETURN();
})

OPCODE_GEN(JUMP, "jump", I, {
	NEXT_N_INSTR(_ARG);
})

OPCODE_GEN(JUMP_TRUE, "jump_true", I, {
	if (ivm_object_toBool(STACK_POP(), _STATE)) {
		NEXT_N_INSTR(_ARG);
	} else {
		NEXT_INSTR();
	}
})

OPCODE_GEN(JUMP_FALSE, "jump_false", I, {
	if (!ivm_object_toBool(STACK_POP(), _STATE)) {
		NEXT_N_INSTR(_ARG);
	} else {
		NEXT_INSTR();
	}
})

OPCODE_GEN(TEST1, "test1", N, {
	IVM_OUT("test1\n");
	NEXT_INSTR();
})

OPCODE_GEN(TEST2, "test2", I, { 
	NEXT_INSTR();
})

OPCODE_GEN(TEST3, "test3", S, {
	IVM_OUT("morning! this is test3\n");
	IVM_OUT("string argument: %s\n", ivm_string_trimHead(ivm_string_pool_get(_STRING_POOL, _ARG)));
	NEXT_INSTR();
})
