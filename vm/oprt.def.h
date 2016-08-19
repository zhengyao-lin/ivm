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

BINOP_GEN(IVM_UNDEFINED_T, EQ, IVM_UNDEFINED_T, {
	return (ivm_object_t *)(ivm_ptr_t)IVM_TRUE;
})

BINOP_GEN(IVM_UNDEFINED_T, NE, IVM_UNDEFINED_T, {
	return (ivm_object_t *)(ivm_ptr_t)IVM_FALSE;
})

BINOP_GEN(IVM_NULL_T, EQ, IVM_NULL_T, {
	return (ivm_object_t *)(ivm_ptr_t)IVM_TRUE;
})

BINOP_GEN(IVM_NULL_T, NE, IVM_NULL_T, {
	return (ivm_object_t *)(ivm_ptr_t)IVM_FALSE;
})

BINOP_GEN(IVM_OBJECT_T, EQ, IVM_OBJECT_T, {
	return (ivm_object_t *)(ivm_ptr_t)(_OP1 == _OP2);
})

BINOP_GEN(IVM_OBJECT_T, NE, IVM_OBJECT_T, {
	return (ivm_object_t *)(ivm_ptr_t)(_OP1 != _OP2);
})

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

	MEM_COPY(data, ivm_string_trimHead(str1), len1 * sizeof(ivm_char_t));
	MEM_COPY(data + len1, ivm_string_trimHead(str2), len2 * sizeof(ivm_char_t));
	data[size] = '\0';

	ivm_string_initHead(ret, IVM_FALSE, size);

	return ivm_string_object_new_c(_STATE, ret);
})

BINOP_GEN(IVM_STRING_OBJECT_T, ADD, IVM_NUMERIC_T, LINK_STRING_NUM(_OP1, _OP2))
BINOP_GEN(IVM_NUMERIC_T, ADD, IVM_STRING_OBJECT_T, LINK_STRING_NUM_R(_OP2, _OP1))

BINOP_GEN(IVM_OBJECT_T, IDX, IVM_STRING_OBJECT_T, GET_STRING_INDEX())
BINOP_GEN(IVM_NUMERIC_T, IDX, IVM_STRING_OBJECT_T, GET_STRING_INDEX())
BINOP_GEN(IVM_STRING_OBJECT_T, IDX, IVM_STRING_OBJECT_T, GET_STRING_INDEX())
BINOP_GEN(IVM_FUNCTION_OBJECT_T, IDX, IVM_STRING_OBJECT_T, GET_STRING_INDEX())
BINOP_GEN(IVM_LIST_OBJECT_T, IDX, IVM_STRING_OBJECT_T, GET_STRING_INDEX())

BINOP_GEN(IVM_STRING_OBJECT_T, IDX, IVM_NUMERIC_T, {
	const ivm_string_t *str1;
	ivm_long_t idx, len;
	ivm_string_t *ret;
	ivm_char_t *data;

	str1 = ivm_string_object_getValue(_OP1);
	len = ivm_string_length(str1);
	idx = ivm_list_realIndex(len, ivm_numeric_getValue(_OP2));

	RTM_ASSERT(idx < len, IVM_ERROR_MSG_STRING_IDX_EXCEED(idx, len));

	ret = ivm_vmstate_alloc(_STATE, IVM_STRING_GET_SIZE(1));
	data = ivm_string_trimHead(ret);

	MEM_COPY(data, ivm_string_trimHead(str1) + idx, sizeof(ivm_char_t));
	data[1] = '\0';

	return ivm_string_object_new_c(_STATE, ret);
})

BINOP_GEN(IVM_LIST_OBJECT_T, IDX, IVM_NUMERIC_T, {
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

BINOP_GEN(IVM_LIST_OBJECT_T, ADD, IVM_LIST_OBJECT_T, {
	return ivm_list_object_link(IVM_AS(_OP1, ivm_list_object_t),
								IVM_AS(_OP2, ivm_list_object_t),
								_STATE);
})

BINOP_GEN(IVM_NUMERIC_T, SHL, IVM_NUMERIC_T, {
	return ivm_numeric_new(_STATE, (ivm_long_t)ivm_numeric_getValue(_OP1) << (ivm_long_t)ivm_numeric_getValue(_OP2));
})

BINOP_GEN(IVM_NUMERIC_T, SHAR, IVM_NUMERIC_T, {
	return ivm_numeric_new(_STATE, (ivm_long_t)ivm_numeric_getValue(_OP1) >> (ivm_long_t)ivm_numeric_getValue(_OP2));
})

BINOP_GEN(IVM_NUMERIC_T, SHLR, IVM_NUMERIC_T, {
	return ivm_numeric_new(_STATE, (ivm_ulong_t)ivm_numeric_getValue(_OP1) >> (ivm_long_t)ivm_numeric_getValue(_OP2));
})
