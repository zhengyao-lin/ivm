UNIOP_GEN(NOT, NUMERIC, {
	return ivm_numeric_new(_STATE, !ivm_numeric_getValue(_OP1));
})

UNIOP_GEN(NEG, NUMERIC, {
	return ivm_numeric_new(_STATE, -ivm_numeric_getValue(_OP1));
})

UNIOP_GEN(POS, NUMERIC, {
	return ivm_numeric_new(_STATE, ivm_numeric_getValue(_OP1));
})

BINOP_GEN(NUMERIC, ADD, NUMERIC, {
	return ivm_numeric_new(_STATE, ivm_numeric_getValue(_OP1) + ivm_numeric_getValue(_OP2));
})

BINOP_GEN(NUMERIC, SUB, NUMERIC, {
	return ivm_numeric_new(_STATE, ivm_numeric_getValue(_OP1) - ivm_numeric_getValue(_OP2));
})

BINOP_GEN(NUMERIC, MUL, NUMERIC, {
	return ivm_numeric_new(_STATE, ivm_numeric_getValue(_OP1) * ivm_numeric_getValue(_OP2));
})

BINOP_GEN(NUMERIC, DIV, NUMERIC, {
	return ivm_numeric_new(_STATE, ivm_numeric_getValue(_OP1) / ivm_numeric_getValue(_OP2));
})

BINOP_GEN(NUMERIC, MOD, NUMERIC, {
	return ivm_numeric_new(_STATE, fmod(ivm_numeric_getValue(_OP1), ivm_numeric_getValue(_OP2)));
})

BINOP_GEN(NUMERIC, CMP, NUMERIC, {
	return (ivm_object_t *)(ivm_ptr_t)(ivm_numeric_getValue(_OP1) - ivm_numeric_getValue(_OP2));
})

BINOP_GEN(NUMERIC, AND, NUMERIC, {
	return ivm_numeric_new(_STATE, (ivm_long_t)ivm_numeric_getValue(_OP1) & (ivm_long_t)ivm_numeric_getValue(_OP2));
})

BINOP_GEN(NUMERIC, EOR, NUMERIC, {
	return ivm_numeric_new(_STATE, (ivm_long_t)ivm_numeric_getValue(_OP1) ^ (ivm_long_t)ivm_numeric_getValue(_OP2));
})

BINOP_GEN(NUMERIC, IOR, NUMERIC, {
	return ivm_numeric_new(_STATE, (ivm_long_t)ivm_numeric_getValue(_OP1) | (ivm_long_t)ivm_numeric_getValue(_OP2));
})

BINOP_GEN(STRING_OBJECT, ADD, STRING_OBJECT, {
	const ivm_string_t *str1 = ivm_string_object_getValue(_OP1);
	const ivm_string_t *str2 = ivm_string_object_getValue(_OP2);
	ivm_size_t len1 = ivm_string_length(str1);
	ivm_size_t len2 = ivm_string_length(str2);
	ivm_size_t size = len1 + len2;
	ivm_string_t *ret = ivm_vmstate_alloc(_STATE, IVM_STRING_GET_SIZE(size));

	ivm_char_t *data = ivm_string_trimHead(ret);

	MEM_COPY(data, ivm_string_trimHead(str1), len1 * sizeof(ivm_char_t));
	MEM_COPY(data + len1, ivm_string_trimHead(str2), len2 * sizeof(ivm_char_t));
	data[size] = '\0';

	ivm_string_initHead(ret, IVM_FALSE, size);

	return ivm_string_object_new(_STATE, ret);
})

BINOP_GEN(OBJECT, IDX, STRING_OBJECT, GET_STRING_INDEX())
BINOP_GEN(NUMERIC, IDX, STRING_OBJECT, GET_STRING_INDEX())
BINOP_GEN(STRING_OBJECT, IDX, STRING_OBJECT, GET_STRING_INDEX())
BINOP_GEN(FUNCTION_OBJECT, IDX, STRING_OBJECT, GET_STRING_INDEX())
BINOP_GEN(LIST_OBJECT, IDX, STRING_OBJECT, GET_STRING_INDEX())

BINOP_GEN(LIST_OBJECT, IDX, NUMERIC, {
	ivm_object_t *tmp;

	if (_ASSIGN) {
		return ivm_list_object_set(
			IVM_AS(_OP1, ivm_list_object_t),
			_STATE, ivm_numeric_getValue(_OP2),
			_ASSIGN
		);
	}

	tmp = ivm_list_object_get(IVM_AS(_OP1, ivm_list_object_t), ivm_numeric_getValue(_OP2));

	return tmp ? tmp : IVM_UNDEFINED(_STATE);
})

BINOP_GEN(LIST_OBJECT, ADD, LIST_OBJECT, {
	return ivm_list_object_link(IVM_AS(_OP1, ivm_list_object_t),
								IVM_AS(_OP2, ivm_list_object_t),
								_STATE);
})
