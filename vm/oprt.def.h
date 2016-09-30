/*
UNIOP_GEN(NOT, IVM_NUMERIC_T, {
	return ivm_numeric_new(_STATE, !ivm_numeric_getValue(_OP1));
})
*/

UNIOP_GEN(NEG, IVM_NUMERIC_T, {
	return ivm_numeric_new(_STATE, -ivm_numeric_getValue(_OP1));
})

UNIOP_GEN(POS, IVM_NUMERIC_T, {
	return ivm_numeric_new(_STATE, ivm_numeric_getValue(_OP1));
})

BINOP_GEN(IVM_NUMERIC_T, ADD, IVM_NUMERIC_T, {
	return ivm_numeric_new(_STATE, ivm_numeric_getValue(_OP1) + ivm_numeric_getValue(_OP2));
})

BINOP_GEN(IVM_NUMERIC_T, SUB, IVM_NUMERIC_T, {
	return ivm_numeric_new(_STATE, ivm_numeric_getValue(_OP1) - ivm_numeric_getValue(_OP2));
})

BINOP_GEN(IVM_NUMERIC_T, MUL, IVM_NUMERIC_T, {
	return ivm_numeric_new(_STATE, ivm_numeric_getValue(_OP1) * ivm_numeric_getValue(_OP2));
})

BINOP_GEN(IVM_NUMERIC_T, DIV, IVM_NUMERIC_T, {
	return ivm_numeric_new(_STATE, ivm_numeric_getValue(_OP1) / ivm_numeric_getValue(_OP2));
})

BINOP_GEN(IVM_NUMERIC_T, MOD, IVM_NUMERIC_T, {
	return ivm_numeric_new(_STATE, fmod(ivm_numeric_getValue(_OP1), ivm_numeric_getValue(_OP2)));
})

/*
BINOP_GEN(IVM_NUMERIC_T, CMP, IVM_NUMERIC_T, {
	ivm_double_t a = ivm_numeric_getValue(_OP1);
	ivm_double_t b = ivm_numeric_getValue(_OP2);

	return (ivm_object_t *)(ivm_ptr_t)(a > b ? 1 : (a < b ? -1: 0));
})
*/

BINOP_GEN(IVM_NUMERIC_T, NE, IVM_NUMERIC_T, {
	return (ivm_object_t *)(ivm_ptr_t)(ivm_numeric_getValue(_OP1) != ivm_numeric_getValue(_OP2));
})

BINOP_GEN(IVM_NUMERIC_T, EQ, IVM_NUMERIC_T, {
	return (ivm_object_t *)(ivm_ptr_t)(ivm_numeric_getValue(_OP1) == ivm_numeric_getValue(_OP2));
})

BINOP_GEN(IVM_NUMERIC_T, GT, IVM_NUMERIC_T, {
	return (ivm_object_t *)(ivm_ptr_t)(ivm_numeric_getValue(_OP1) > ivm_numeric_getValue(_OP2));
})

BINOP_GEN(IVM_NUMERIC_T, GE, IVM_NUMERIC_T, {
	return (ivm_object_t *)(ivm_ptr_t)(ivm_numeric_getValue(_OP1) >= ivm_numeric_getValue(_OP2));
})

BINOP_GEN(IVM_NUMERIC_T, LT, IVM_NUMERIC_T, {
	return (ivm_object_t *)(ivm_ptr_t)(ivm_numeric_getValue(_OP1) < ivm_numeric_getValue(_OP2));
})

BINOP_GEN(IVM_NUMERIC_T, LE, IVM_NUMERIC_T, {
	return (ivm_object_t *)(ivm_ptr_t)(ivm_numeric_getValue(_OP1) <= ivm_numeric_getValue(_OP2));
})

BINOP_GEN_C(IVM_NONE_T, EQ, IVM_NONE_T, ivm_binop_constTrue)
BINOP_GEN_C(IVM_NONE_T, NE, IVM_NONE_T, ivm_binop_constFalse)

BINOP_GEN_C(IVM_OBJECT_T, EQ, IVM_OBJECT_T, ivm_binop_eq)
BINOP_GEN_C(IVM_OBJECT_T, NE, IVM_OBJECT_T, ivm_binop_ne)

BINOP_GEN(IVM_TYPE_OBJECT_T, EQ, IVM_TYPE_OBJECT_T, {
	return (ivm_object_t *)(ivm_ptr_t)
		   (ivm_type_object_getValue(_OP1) == ivm_type_object_getValue(_OP2));
})

BINOP_GEN(IVM_TYPE_OBJECT_T, NE, IVM_TYPE_OBJECT_T, {
	return (ivm_object_t *)(ivm_ptr_t)
		   (ivm_type_object_getValue(_OP1) != ivm_type_object_getValue(_OP2));
})

BINOP_GEN_C(IVM_FUNCTION_OBJECT_T, EQ, IVM_FUNCTION_OBJECT_T, ivm_binop_eq)
BINOP_GEN_C(IVM_FUNCTION_OBJECT_T, NE, IVM_FUNCTION_OBJECT_T, ivm_binop_ne)

BINOP_GEN(IVM_STRING_OBJECT_T, EQ, IVM_STRING_OBJECT_T, {
	return (ivm_object_t *)(ivm_ptr_t)(!ivm_string_compare_c(ivm_string_object_getValue(_OP1), ivm_string_object_getValue(_OP2)));
})

BINOP_GEN(IVM_STRING_OBJECT_T, NE, IVM_STRING_OBJECT_T, {
	return (ivm_object_t *)(ivm_ptr_t)(ivm_string_compare_c(ivm_string_object_getValue(_OP1), ivm_string_object_getValue(_OP2)));
})

BINOP_GEN(IVM_NUMERIC_T, AND, IVM_NUMERIC_T, {
	return ivm_numeric_new(_STATE, (ivm_long_t)ivm_numeric_getValue(_OP1) & (ivm_long_t)ivm_numeric_getValue(_OP2));
})

BINOP_GEN(IVM_NUMERIC_T, EOR, IVM_NUMERIC_T, {
	return ivm_numeric_new(_STATE, (ivm_long_t)ivm_numeric_getValue(_OP1) ^ (ivm_long_t)ivm_numeric_getValue(_OP2));
})

BINOP_GEN(IVM_NUMERIC_T, IOR, IVM_NUMERIC_T, {
	return ivm_numeric_new(_STATE, (ivm_long_t)ivm_numeric_getValue(_OP1) | (ivm_long_t)ivm_numeric_getValue(_OP2));
})

BINOP_GEN(IVM_STRING_OBJECT_T, ADD, IVM_STRING_OBJECT_T, {
	const ivm_string_t *str1 = ivm_string_object_getValue(_OP1);
	const ivm_string_t *str2 = ivm_string_object_getValue(_OP2);
	ivm_size_t len1 = ivm_string_length(str1);
	ivm_size_t len2 = ivm_string_length(str2);
	ivm_size_t size = len1 + len2;
	ivm_string_t *ret = ivm_vmstate_alloc(_STATE, IVM_STRING_GET_SIZE(size));

	ivm_char_t *data = ivm_string_trimHead(ret);

	STD_MEMCPY(data, ivm_string_trimHead(str1), len1 * sizeof(ivm_char_t));
	STD_MEMCPY(data + len1, ivm_string_trimHead(str2), len2 * sizeof(ivm_char_t));
	data[size] = '\0';

	ivm_string_initHead(ret, IVM_FALSE, size);

	return ivm_string_object_new_c(_STATE, ret);
})

BINOP_GEN_C(IVM_STRING_OBJECT_T, ADD, IVM_NUMERIC_T, ivm_binop_linkStringNum)
BINOP_GEN_C(IVM_NUMERIC_T, ADD, IVM_STRING_OBJECT_T, ivm_binop_linkStringNum_r)

BINOP_GEN_C(IVM_OBJECT_T, IDX, IVM_STRING_OBJECT_T, ivm_binop_getStringIndex)
BINOP_GEN_C(IVM_NUMERIC_T, IDX, IVM_STRING_OBJECT_T, ivm_binop_getStringIndex)
BINOP_GEN_C(IVM_STRING_OBJECT_T, IDX, IVM_STRING_OBJECT_T, ivm_binop_getStringIndex)
BINOP_GEN_C(IVM_FUNCTION_OBJECT_T, IDX, IVM_STRING_OBJECT_T, ivm_binop_getStringIndex)
BINOP_GEN_C(IVM_LIST_OBJECT_T, IDX, IVM_STRING_OBJECT_T, ivm_binop_getStringIndex)

TRIOP_GEN_C(IVM_OBJECT_T, IDXA, IVM_STRING_OBJECT_T, ivm_binop_setStringIndex)
TRIOP_GEN_C(IVM_NUMERIC_T, IDXA, IVM_STRING_OBJECT_T, ivm_binop_setStringIndex)
TRIOP_GEN_C(IVM_STRING_OBJECT_T, IDXA, IVM_STRING_OBJECT_T, ivm_binop_setStringIndex)
TRIOP_GEN_C(IVM_FUNCTION_OBJECT_T, IDXA, IVM_STRING_OBJECT_T, ivm_binop_setStringIndex)
TRIOP_GEN_C(IVM_LIST_OBJECT_T, IDXA, IVM_STRING_OBJECT_T, ivm_binop_setStringIndex)

BINOP_GEN(IVM_STRING_OBJECT_T, IDX, IVM_NUMERIC_T, {
	const ivm_string_t *str1;
	ivm_long_t idx, len;
	ivm_string_t *ret;
	ivm_char_t *data;

	// RTM_ASSERT(!_ASSIGN, IVM_ERROR_MSG_ASSIGN_TO_STRING_INDEX);

	str1 = ivm_string_object_getValue(_OP1);
	len = ivm_string_length(str1);
	idx = ivm_list_realIndex(len, ivm_numeric_getValue(_OP2));

	if (idx > len) return IVM_NONE(_STATE);

	// RTM_ASSERT(idx < len, IVM_ERROR_MSG_STRING_IDX_EXCEED(idx, len));

	ret = ivm_vmstate_alloc(_STATE, IVM_STRING_GET_SIZE(1));
	data = ivm_string_trimHead(ret);

	STD_MEMCPY(data, ivm_string_trimHead(str1) + idx, sizeof(ivm_char_t));
	data[1] = '\0';

	return ivm_string_object_new_c(_STATE, ret);
})

BINOP_GEN(IVM_LIST_OBJECT_T, IDX, IVM_NUMERIC_T, {
	ivm_object_t *tmp;
	tmp = ivm_list_object_get(IVM_AS(_OP1, ivm_list_object_t), ivm_numeric_getValue(_OP2));
	return tmp ? tmp : IVM_NONE(_STATE);
})

TRIOP_GEN(IVM_LIST_OBJECT_T, IDXA, IVM_NUMERIC_T, {
	ivm_list_object_set(
		IVM_AS(_OP1, ivm_list_object_t),
		_STATE, ivm_numeric_getValue(_OP2),
		_OP3
	);

	return _OP3 ? _OP3 : IVM_NONE(_STATE);
})

BINOP_GEN(IVM_LIST_OBJECT_T, ADD, IVM_LIST_OBJECT_T, {
	return ivm_list_object_link(IVM_AS(_OP1, ivm_list_object_t),
								IVM_AS(_OP2, ivm_list_object_t),
								_STATE);
})

BINOP_GEN(IVM_LIST_OBJECT_T, MUL, IVM_NUMERIC_T, {
	ivm_number_t times = ivm_numeric_getValue(_OP2);
	ivm_list_object_t *ret;

	if (times <= 0) {
		return ivm_list_object_new(_STATE, 0);
	}

	ret = IVM_AS(ivm_object_clone(_OP1, _STATE), ivm_list_object_t);
	ivm_list_object_multiply(ret, _STATE, times);

	return IVM_AS_OBJ(ret);
})

BINOP_GEN(IVM_NUMERIC_T, SHL, IVM_NUMERIC_T, {
	return ivm_numeric_new(_STATE, (ivm_sint64_t)ivm_numeric_getValue(_OP1) << (ivm_long_t)ivm_numeric_getValue(_OP2));
})

BINOP_GEN(IVM_NUMERIC_T, SHAR, IVM_NUMERIC_T, {
	return ivm_numeric_new(_STATE, (ivm_sint64_t)ivm_numeric_getValue(_OP1) >> (ivm_long_t)ivm_numeric_getValue(_OP2));
})

BINOP_GEN(IVM_NUMERIC_T, SHLR, IVM_NUMERIC_T, {
	return ivm_numeric_new(_STATE, (ivm_uint64_t)ivm_numeric_getValue(_OP1) >> (ivm_long_t)ivm_numeric_getValue(_OP2));
})
