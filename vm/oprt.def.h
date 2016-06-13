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
