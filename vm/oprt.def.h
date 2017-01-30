/******************* uniop *******************/

UNIOP_GEN(NOT, IVM_NONE_T, {
	return ivm_numeric_new(_STATE, IVM_TRUE);
})

UNIOP_GEN(NOT, IVM_NUMERIC_T, {
	return ivm_numeric_new(_STATE, !ivm_numeric_getValue(_OP1));
})

UNIOP_GEN(NEG, IVM_NUMERIC_T, {
	return ivm_numeric_new(_STATE, -ivm_numeric_getValue(_OP1));
})

UNIOP_GEN(POS, IVM_NUMERIC_T, {
	return ivm_numeric_new(_STATE, ivm_numeric_getValue(_OP1));
})

UNIOP_GEN(BNOT, IVM_NUMERIC_T, {
	return ivm_numeric_new(_STATE, ~(ivm_sint32_t)ivm_numeric_getValue(_OP1));
})

/******************* binop *******************/

/******************* numeric *******************/
BINOP_GEN(IVM_NUMERIC_T, ADD, IVM_FALSE, {
	_TYPE_SWITCH(
		_IS_TYPE(IVM_NUMERIC_T, {
			return ivm_numeric_new(_STATE, ivm_numeric_getValue(_OP1) + ivm_numeric_getValue(_OP2));
		})

		_IS_TYPE(IVM_STRING_OBJECT_T, {
			_FALL_TO_BINOP(ivm_binop_linkStringNum_r);
		})

		_NO_OTHER("+")
	);
})

BINOP_GEN(IVM_NUMERIC_T, SUB, IVM_FALSE, {
	_ONLY_TYPE(IVM_NUMERIC_T, "-", {
		return ivm_numeric_new(_STATE, ivm_numeric_getValue(_OP1) - ivm_numeric_getValue(_OP2));
	});
})

BINOP_GEN(IVM_NUMERIC_T, MUL, IVM_FALSE, {
	_ONLY_TYPE(IVM_NUMERIC_T, "*", {
		return ivm_numeric_new(_STATE, ivm_numeric_getValue(_OP1) * ivm_numeric_getValue(_OP2));
	});
})

BINOP_GEN(IVM_NUMERIC_T, DIV, IVM_FALSE, {
	_ONLY_TYPE(IVM_NUMERIC_T, "/", {
		ivm_number_t denom = ivm_numeric_getValue(_OP2);

		OPRT_ASSERT(denom != 0, IVM_ERROR_MSG_DIV_MOD_ZERO);

		return ivm_numeric_new(_STATE, ivm_numeric_getValue(_OP1) / denom);
	});
})

BINOP_GEN(IVM_NUMERIC_T, MOD, IVM_FALSE, {
	_ONLY_TYPE(IVM_NUMERIC_T, "%", {
		ivm_number_t denom = ivm_numeric_getValue(_OP2);

		OPRT_ASSERT(denom != 0, IVM_ERROR_MSG_DIV_MOD_ZERO);

		return ivm_numeric_new(_STATE, fmod(ivm_numeric_getValue(_OP1), denom));
	});
})

BINOP_GEN(IVM_NUMERIC_T, EQ, IVM_TRUE, {
	_STD_EQ(IVM_NUMERIC_T, {
		return (ivm_object_t *)(ivm_ptr_t)(ivm_numeric_getValue(_OP1) == ivm_numeric_getValue(_OP2));
	});
})

BINOP_GEN(IVM_NUMERIC_T, NE, IVM_TRUE, {
	_STD_NE(IVM_NUMERIC_T, {
		return (ivm_object_t *)(ivm_ptr_t)(ivm_numeric_getValue(_OP1) != ivm_numeric_getValue(_OP2));
	});
})

BINOP_GEN(IVM_NUMERIC_T, GT, IVM_TRUE, {
	_ONLY_TYPE(IVM_NUMERIC_T, ">", {
		return (ivm_object_t *)(ivm_ptr_t)(ivm_numeric_getValue(_OP1) > ivm_numeric_getValue(_OP2));
	});
})

BINOP_GEN(IVM_NUMERIC_T, GE, IVM_TRUE, {
	_ONLY_TYPE(IVM_NUMERIC_T, ">=", {
		return (ivm_object_t *)(ivm_ptr_t)(ivm_numeric_getValue(_OP1) >= ivm_numeric_getValue(_OP2));
	});
})

BINOP_GEN(IVM_NUMERIC_T, LT, IVM_TRUE, {
	_ONLY_TYPE(IVM_NUMERIC_T, "<", {
		return (ivm_object_t *)(ivm_ptr_t)(ivm_numeric_getValue(_OP1) < ivm_numeric_getValue(_OP2));
	});
})

BINOP_GEN(IVM_NUMERIC_T, LE, IVM_TRUE, {
	_ONLY_TYPE(IVM_NUMERIC_T, "<=", {
		return (ivm_object_t *)(ivm_ptr_t)(ivm_numeric_getValue(_OP1) <= ivm_numeric_getValue(_OP2));
	});
})

BINOP_GEN(IVM_NUMERIC_T, AND, IVM_FALSE, {
	_ONLY_TYPE(IVM_NUMERIC_T, "&", {
		return ivm_numeric_new(_STATE, (ivm_sint32_t)ivm_numeric_getValue(_OP1) & (ivm_sint32_t)ivm_numeric_getValue(_OP2));
	});
})

BINOP_GEN(IVM_NUMERIC_T, EOR, IVM_FALSE, {
	_ONLY_TYPE(IVM_NUMERIC_T, "^", {
		return ivm_numeric_new(_STATE, (ivm_sint32_t)ivm_numeric_getValue(_OP1) ^ (ivm_sint32_t)ivm_numeric_getValue(_OP2));
	});
})

BINOP_GEN(IVM_NUMERIC_T, IOR, IVM_FALSE, {
	_ONLY_TYPE(IVM_NUMERIC_T, "|", {
		return ivm_numeric_new(_STATE, (ivm_sint32_t)ivm_numeric_getValue(_OP1) | (ivm_sint32_t)ivm_numeric_getValue(_OP2));
	});
})

BINOP_GEN(IVM_NUMERIC_T, SHL, IVM_FALSE, {
	_ONLY_TYPE(IVM_NUMERIC_T, "<<", {
		return ivm_numeric_new(_STATE, (ivm_sint64_t)ivm_numeric_getValue(_OP1) << (ivm_sint32_t)ivm_numeric_getValue(_OP2));
	});
})

BINOP_GEN(IVM_NUMERIC_T, SHAR, IVM_FALSE, {
	_ONLY_TYPE(IVM_NUMERIC_T, ">>", {
		return ivm_numeric_new(_STATE, (ivm_sint64_t)ivm_numeric_getValue(_OP1) >> (ivm_sint32_t)ivm_numeric_getValue(_OP2));
	});
})

BINOP_GEN(IVM_NUMERIC_T, SHLR, IVM_FALSE, {
	_ONLY_TYPE(IVM_NUMERIC_T, ">>>", {
		return ivm_numeric_new(_STATE, (ivm_uint64_t)ivm_numeric_getValue(_OP1) >> (ivm_sint32_t)ivm_numeric_getValue(_OP2));
	});
})

BINOP_GEN(IVM_NUMERIC_T, INADD, IVM_FALSE, {
	_TYPE_SWITCH(
		_IS_TYPE(IVM_NUMERIC_T, {
			return ivm_numeric_new(_STATE, ivm_numeric_getValue(_OP1) + ivm_numeric_getValue(_OP2));
		})

		_IS_TYPE(IVM_STRING_OBJECT_T, {
			_FALL_TO_BINOP(ivm_binop_linkStringNum_r);
		})

		_NO_OTHER("+=")
	);
})

BINOP_GEN(IVM_NUMERIC_T, INSUB, IVM_FALSE, {
	_ONLY_TYPE(IVM_NUMERIC_T, "-=", {
		return ivm_numeric_new(_STATE, ivm_numeric_getValue(_OP1) - ivm_numeric_getValue(_OP2));
	});
})

BINOP_GEN(IVM_NUMERIC_T, INMUL, IVM_FALSE, {
	_ONLY_TYPE(IVM_NUMERIC_T, "*=", {
		return ivm_numeric_new(_STATE, ivm_numeric_getValue(_OP1) * ivm_numeric_getValue(_OP2));
	});
})

BINOP_GEN(IVM_NUMERIC_T, INDIV, IVM_FALSE, {
	_ONLY_TYPE(IVM_NUMERIC_T, "/=", {
		ivm_number_t denom = ivm_numeric_getValue(_OP2);

		OPRT_ASSERT(denom != 0, IVM_ERROR_MSG_DIV_MOD_ZERO);

		return ivm_numeric_new(_STATE, ivm_numeric_getValue(_OP1) / denom);
	});
})

BINOP_GEN(IVM_NUMERIC_T, INMOD, IVM_FALSE, {
	_ONLY_TYPE(IVM_NUMERIC_T, "*=", {
		ivm_number_t denom = ivm_numeric_getValue(_OP2);

		OPRT_ASSERT(denom != 0, IVM_ERROR_MSG_DIV_MOD_ZERO);

		return ivm_numeric_new(_STATE, fmod(ivm_numeric_getValue(_OP1), denom));
	});
})

BINOP_GEN(IVM_NUMERIC_T, INAND, IVM_FALSE, {
	_ONLY_TYPE(IVM_NUMERIC_T, "&=", {
		return ivm_numeric_new(_STATE, (ivm_sint32_t)ivm_numeric_getValue(_OP1) & (ivm_sint32_t)ivm_numeric_getValue(_OP2));
	});
})

BINOP_GEN(IVM_NUMERIC_T, INEOR, IVM_FALSE, {
	_ONLY_TYPE(IVM_NUMERIC_T, "^=", {
		return ivm_numeric_new(_STATE, (ivm_sint32_t)ivm_numeric_getValue(_OP1) ^ (ivm_sint32_t)ivm_numeric_getValue(_OP2));
	});
})

BINOP_GEN(IVM_NUMERIC_T, INIOR, IVM_FALSE, {
	_ONLY_TYPE(IVM_NUMERIC_T, "|=", {
		return ivm_numeric_new(_STATE, (ivm_sint32_t)ivm_numeric_getValue(_OP1) | (ivm_sint32_t)ivm_numeric_getValue(_OP2));
	});
})

BINOP_GEN(IVM_NUMERIC_T, INSHL, IVM_FALSE, {
	_ONLY_TYPE(IVM_NUMERIC_T, "<<=", {
		return ivm_numeric_new(_STATE, (ivm_sint64_t)ivm_numeric_getValue(_OP1) << (ivm_sint32_t)ivm_numeric_getValue(_OP2));
	});
})

BINOP_GEN(IVM_NUMERIC_T, INSHAR, IVM_FALSE, {
	_ONLY_TYPE(IVM_NUMERIC_T, ">>=", {
		return ivm_numeric_new(_STATE, (ivm_sint64_t)ivm_numeric_getValue(_OP1) >> (ivm_sint32_t)ivm_numeric_getValue(_OP2));
	});
})

BINOP_GEN(IVM_NUMERIC_T, INSHLR,  IVM_FALSE, {
	_ONLY_TYPE(IVM_NUMERIC_T, ">>>=", {
		return ivm_numeric_new(_STATE, (ivm_uint64_t)ivm_numeric_getValue(_OP1) >> (ivm_sint32_t)ivm_numeric_getValue(_OP2));
	});
})

/******************* none/object *******************/
BINOP_GEN_C(IVM_NONE_T, EQ, IVM_TRUE, ivm_binop_eq)
BINOP_GEN_C(IVM_NONE_T, NE, IVM_TRUE, ivm_binop_ne)

BINOP_GEN_C(IVM_OBJECT_T, EQ, IVM_TRUE, ivm_binop_eq)
BINOP_GEN_C(IVM_OBJECT_T, NE, IVM_TRUE, ivm_binop_ne)

BINOP_GEN_C(IVM_OBJECT_T, IDX, IVM_FALSE, ivm_binop_getStringIndex)
TRIOP_GEN_C(IVM_OBJECT_T, IDXA, ivm_binop_setStringIndex)

/******************* type *******************/
BINOP_GEN(IVM_TYPE_OBJECT_T, EQ, IVM_TRUE, {
	_STD_EQ(IVM_TYPE_OBJECT_T, {
		return (ivm_object_t *)(ivm_ptr_t)
			   (ivm_type_object_getValue(_OP1) == ivm_type_object_getValue(_OP2));
	});
})

BINOP_GEN(IVM_TYPE_OBJECT_T, NE, IVM_TRUE, {
	_STD_NE(IVM_TYPE_OBJECT_T, {
		return (ivm_object_t *)(ivm_ptr_t)
			   (ivm_type_object_getValue(_OP1) != ivm_type_object_getValue(_OP2));
	});
})

/******************* string *******************/
BINOP_GEN(IVM_STRING_OBJECT_T, EQ, IVM_TRUE, {
	_STD_EQ(IVM_STRING_OBJECT_T, {
		return (ivm_object_t *)(ivm_ptr_t)(!ivm_string_compare_c(ivm_string_object_getValue(_OP1), ivm_string_object_getValue(_OP2)));
	});
})

BINOP_GEN(IVM_STRING_OBJECT_T, NE, IVM_TRUE, {
	_STD_NE(IVM_STRING_OBJECT_T, {
		return (ivm_object_t *)(ivm_ptr_t)(ivm_string_compare_c(ivm_string_object_getValue(_OP1), ivm_string_object_getValue(_OP2)));
	});
})

BINOP_GEN(IVM_STRING_OBJECT_T, ADD, IVM_FALSE, {
	_TYPE_SWITCH(
		_IS_TYPE(IVM_STRING_OBJECT_T, {
			_FALL_TO_BINOP(ivm_binop_linkStringString);
		})

		_IS_TYPE(IVM_NUMERIC_T, {
			_FALL_TO_BINOP(ivm_binop_linkStringNum);
		})

		_NO_OTHER("+")
	);
})

BINOP_GEN(IVM_STRING_OBJECT_T, IDX, IVM_FALSE, {
	_TYPE_SWITCH(
		_IS_TYPE(IVM_NUMERIC_T, {
			const ivm_string_t *op1;
			ivm_long_t idx, len;
			ivm_number_t n;

			// OPRT_ASSERT(!_ASSIGN, IVM_ERROR_MSG_ASSIGN_TO_STRING_INDEX);

			op1 = ivm_string_object_getValue(_OP1);
			n = ivm_numeric_getValue(_OP2);

			CHECK_OVERFLOW(n);

			len = ivm_string_length(op1);
			idx = ivm_list_realIndex(len, n);

			OPRT_ASSERT(idx < len, IVM_ERROR_MSG_STRING_IDX_EXCEED(idx, len));

			return ivm_string_object_newChar(_STATE, ivm_string_trimHead(op1)[idx]);
		})

		_IS_TYPE(IVM_STRING_OBJECT_T, {
			_FALL_TO_BINOP(ivm_binop_getStringIndex);
		})

		_NO_OTHER("[]")
	);
})

BINOP_GEN_C(IVM_STRING_OBJECT_T, MUL, IVM_FALSE, ivm_binop_mulString)
BINOP_GEN_C(IVM_STRING_OBJECT_T, INMUL, IVM_FALSE, ivm_binop_mulString)

BINOP_GEN(IVM_STRING_OBJECT_T, INADD, IVM_FALSE, {
	_TYPE_SWITCH(
		_IS_TYPE(IVM_STRING_OBJECT_T, {
			_FALL_TO_BINOP(ivm_binop_linkStringString);
		})

		_IS_TYPE(IVM_NUMERIC_T, {
			_FALL_TO_BINOP(ivm_binop_linkStringNum);
		})

		_NO_OTHER("+=")
	);
})

/******************* coro *******************/
BINOP_GEN(IVM_CORO_OBJECT_T, EQ, IVM_TRUE, {
	_STD_EQ(IVM_CORO_OBJECT_T, {
		return (ivm_object_t *)(ivm_ptr_t)(ivm_coro_object_getCoro(_OP1) == ivm_coro_object_getCoro(_OP2));
	});
})

BINOP_GEN(IVM_CORO_OBJECT_T, NE, IVM_TRUE, {
	_STD_NE(IVM_CORO_OBJECT_T, {
		return (ivm_object_t *)(ivm_ptr_t)(ivm_coro_object_getCoro(_OP1) != ivm_coro_object_getCoro(_OP2));
	});
})

/******************* list *******************/
BINOP_GEN(IVM_LIST_OBJECT_T, IDX, IVM_FALSE, {
	_TYPE_SWITCH(
		_IS_TYPE(IVM_NUMERIC_T, {
			ivm_object_t *tmp;
			tmp = ivm_list_object_get(IVM_AS(_OP1, ivm_list_object_t), ivm_numeric_getValue(_OP2));
			return tmp ? tmp : IVM_NONE(_STATE);
		})

		_IS_TYPE(IVM_STRING_OBJECT_T, {
			_FALL_TO_BINOP(ivm_binop_getStringIndex);
		})

		_NO_OTHER("[]")
	);
})

TRIOP_GEN(IVM_LIST_OBJECT_T, IDXA, {
	_TYPE_SWITCH(
		_IS_TYPE(IVM_NUMERIC_T, {
			ivm_number_t idx = ivm_numeric_getValue(_OP2);
			ivm_list_object_t *lobj = IVM_AS(_OP1, ivm_list_object_t);
			ivm_size_t real_idx;

			CHECK_OVERFLOW(idx);

			real_idx = ivm_list_realIndex(ivm_list_object_getSize(lobj), idx);

			if (!ivm_list_object_set(lobj, _STATE, real_idx, _OP3))
				return IVM_NULL;

			return _OP3 ? _OP3 : IVM_NONE(_STATE);
		})

		_IS_TYPE(IVM_STRING_OBJECT_T, {
			_FALL_TO_TRIOP(ivm_binop_setStringIndex);
		})

		_NO_OTHER("[=]")
	);
})

BINOP_GEN(IVM_LIST_OBJECT_T, ADD, IVM_FALSE, {
	_ONLY_TYPE(IVM_LIST_OBJECT_T, "+", {
		return ivm_list_object_link(IVM_AS(_OP1, ivm_list_object_t),
									IVM_AS(_OP2, ivm_list_object_t),
									_STATE);
	});
})

BINOP_GEN(IVM_LIST_OBJECT_T, INADD, IVM_FALSE, {
	_ONLY_TYPE(IVM_LIST_OBJECT_T, "+=", {
		return ivm_list_object_link(IVM_AS(_OP1, ivm_list_object_t),
									IVM_AS(_OP2, ivm_list_object_t),
									_STATE);
	});
})

BINOP_GEN_C(IVM_LIST_OBJECT_T, MUL, IVM_FALSE, ivm_binop_mulList)
BINOP_GEN_C(IVM_LIST_OBJECT_T, INMUL, IVM_FALSE, ivm_binop_mulList)

/******************* function *******************/
BINOP_GEN(IVM_FUNCTION_OBJECT_T, EQ, IVM_TRUE, {
	_STD_EQ(IVM_FUNCTION_OBJECT_T, {
		return (ivm_object_t *)(ivm_ptr_t)(ivm_function_object_getFunc(_OP1) == ivm_function_object_getFunc(_OP2));
	});
})

BINOP_GEN(IVM_FUNCTION_OBJECT_T, NE, IVM_TRUE, {
	_STD_NE(IVM_FUNCTION_OBJECT_T, {
		return (ivm_object_t *)(ivm_ptr_t)(ivm_function_object_getFunc(_OP1) != ivm_function_object_getFunc(_OP2));
	});
})
