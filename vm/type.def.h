TYPE_GEN(IVM_NONE_T, none, sizeof(ivm_object_t), IVM_NULL, {
}, .const_bool = IVM_FALSE)

TYPE_GEN(IVM_OBJECT_T, object, sizeof(ivm_object_t),
	IVM_GET_NATIVE_FUNC(_object_cons), {

	ivm_object_t *tmp = ivm_object_new_c(_STATE, IVM_DEFAULT_SLOT_TABLE_TO_HASH_THRESHOLD);
	ivm_type_setProto(_TYPE, tmp);

	ivm_object_setSlot_r(tmp, _STATE, "merge", IVM_NATIVE_WRAP(_STATE, _object_merge));
	ivm_object_setSlot_r(tmp, _STATE, "clone", IVM_NATIVE_WRAP(_STATE, _object_clone));
	ivm_object_setSlot_r(tmp, _STATE, "call", IVM_NATIVE_WRAP(_STATE, _object_call));
	// ivm_object_setSlot_r(tmp, _STATE, "type", IVM_NATIVE_WRAP(_STATE, _object_type));

}, .const_bool = IVM_TRUE)

TYPE_GEN(IVM_TYPE_OBJECT_T, type, sizeof(ivm_type_object_t),
	IVM_GET_NATIVE_FUNC(_type_cons), {

	ivm_object_t *tmp = ivm_type_object_new(_STATE, _TYPE);
	ivm_type_setProto(_TYPE, tmp);
	ivm_object_setProto(tmp, _STATE, ivm_vmstate_getTypeProto(_STATE, IVM_OBJECT_T));

	ivm_object_setSlot(tmp, _STATE, IVM_VMSTATE_CONST(_STATE, C_TO_S), IVM_NATIVE_WRAP(_STATE, _type_to_s));

}, .des = ivm_type_object_destructor,
   .trav = ivm_type_object_traverser,
   .const_bool = IVM_TRUE)

TYPE_GEN(IVM_NUMERIC_T, numeric, sizeof(ivm_numeric_t),
	IVM_GET_NATIVE_FUNC(_numeric_cons), {
	
	ivm_object_t *tmp = ivm_numeric_new(_STATE, IVM_NUM(0));
	ivm_type_setProto(_TYPE, tmp);
	ivm_object_setProto(tmp, _STATE, ivm_vmstate_getTypeProto(_STATE, IVM_OBJECT_T));
	ivm_object_initSlots(tmp, _STATE, 8);

	ivm_object_setSlot_r(tmp, _STATE, "ceil", IVM_NATIVE_WRAP(_STATE, _numeric_ceil));
	ivm_object_setSlot_r(tmp, _STATE, "floor", IVM_NATIVE_WRAP(_STATE, _numeric_floor));
	ivm_object_setSlot_r(tmp, _STATE, "round", IVM_NATIVE_WRAP(_STATE, _numeric_round));
	ivm_object_setSlot_r(tmp, _STATE, "trunc", IVM_NATIVE_WRAP(_STATE, _numeric_trunc));
	ivm_object_setSlot(tmp, _STATE, IVM_VMSTATE_CONST(_STATE, C_TO_S), IVM_NATIVE_WRAP(_STATE, _numeric_to_s));

	ivm_object_setSlot_r(tmp, _STATE, "isnan", IVM_NATIVE_WRAP(_STATE, _numeric_isnan));
	ivm_object_setSlot_r(tmp, _STATE, "isinf", IVM_NATIVE_WRAP(_STATE, _numeric_isinf));
	ivm_object_setSlot_r(tmp, _STATE, "isposinf", IVM_NATIVE_WRAP(_STATE, _numeric_isposinf));
	ivm_object_setSlot_r(tmp, _STATE, "isneginf", IVM_NATIVE_WRAP(_STATE, _numeric_isneginf));

	ivm_object_setSlot_r(tmp, _STATE, "char", IVM_NATIVE_WRAP(_STATE, _numeric_char));

}, .to_bool = ivm_numeric_isTrue)

TYPE_GEN(IVM_STRING_OBJECT_T, string, sizeof(ivm_string_object_t),
	IVM_GET_NATIVE_FUNC(_string_cons), {
	
	ivm_object_t *tmp = ivm_string_object_new(_STATE, IVM_VMSTATE_CONST(_STATE, C_EMPTY));
	ivm_type_setProto(_TYPE, tmp);
	ivm_object_setProto(tmp, _STATE, ivm_vmstate_getTypeProto(_STATE, IVM_OBJECT_T));
	// ivm_object_initSlots(tmp, _STATE, IVM_DEFAULT_SLOT_TABLE_TO_HASH_THRESHOLD);

	ivm_object_setSlot_r(tmp, _STATE, "len", IVM_NATIVE_WRAP(_STATE, _string_len));
	ivm_object_setSlot_r(tmp, _STATE, "ord", IVM_NATIVE_WRAP(_STATE, _string_ord));
	ivm_object_setSlot_r(tmp, _STATE, "chars", IVM_NATIVE_WRAP(_STATE, _string_chars));
	ivm_object_setSlot_r(tmp, _STATE, "ords", IVM_NATIVE_WRAP(_STATE, _string_ords));
	ivm_object_setSlot(tmp, _STATE, IVM_VMSTATE_CONST(_STATE, C_TO_S), IVM_NATIVE_WRAP(_STATE, _string_to_s));

}, .trav = ivm_string_object_traverser,
   .const_bool = IVM_TRUE)

TYPE_GEN(IVM_LIST_OBJECT_T, list, sizeof(ivm_list_object_t),
	IVM_GET_NATIVE_FUNC(_list_cons), {
	
	ivm_object_t *tmp = ivm_list_object_new(_STATE, 0);
	ivm_type_setProto(_TYPE, tmp);
	ivm_object_setProto(tmp, _STATE, ivm_vmstate_getTypeProto(_STATE, IVM_OBJECT_T));
	// ivm_object_initSlots(tmp, _STATE, IVM_DEFAULT_SLOT_TABLE_TO_HASH_THRESHOLD);

	ivm_object_setSlot(tmp, _STATE, IVM_VMSTATE_CONST(_STATE, C_SIZE), IVM_NATIVE_WRAP(_STATE, _list_size));
	ivm_object_setSlot_r(tmp, _STATE, "push", IVM_NATIVE_WRAP(_STATE, _list_push));
	ivm_object_setSlot_r(tmp, _STATE, "pop", IVM_NATIVE_WRAP(_STATE, _list_pop));
	ivm_object_setSlot_r(tmp, _STATE, "top", IVM_NATIVE_WRAP(_STATE, _list_top));
	ivm_object_setSlot_r(tmp, _STATE, "slice", IVM_NATIVE_WRAP(_STATE, _list_slice));
	ivm_object_setSlot(tmp, _STATE, IVM_VMSTATE_CONST(_STATE, C_ITER), IVM_NATIVE_WRAP(_STATE, _list_iter));

}, .des = ivm_list_object_destructor,
   .clone = ivm_list_object_cloner,
   .trav = ivm_list_object_traverser,
   .const_bool = IVM_TRUE)

TYPE_GEN(IVM_BUFFER_OBJECT_T, buffer, sizeof(ivm_buffer_object_t),
	IVM_GET_NATIVE_FUNC(_buffer_cons), {

	ivm_object_t *tmp = ivm_buffer_object_new(_STATE, 1);
	ivm_type_setProto(_TYPE, tmp);
	ivm_object_setProto(tmp, _STATE, ivm_vmstate_getTypeProto(_STATE, IVM_OBJECT_T));

	ivm_object_setSlot(tmp, _STATE, IVM_VMSTATE_CONST(_STATE, C_SIZE), IVM_NATIVE_WRAP(_STATE, _buffer_size));
	ivm_object_setSlot(tmp, _STATE, IVM_VMSTATE_CONST(_STATE, C_TO_S), IVM_NATIVE_WRAP(_STATE, _buffer_to_s));
	ivm_object_setSlot_r(tmp, _STATE, "init", IVM_NATIVE_WRAP(_STATE, _buffer_init));

}, .des = ivm_buffer_object_destructor,
   .clone = ivm_buffer_object_cloner,
   .const_bool = IVM_TRUE)

TYPE_GEN(IVM_FUNCTION_OBJECT_T, function, sizeof(ivm_function_object_t),
	IVM_GET_NATIVE_FUNC(_function_cons), {
	
	ivm_object_t *tmp = ivm_function_object_new(_STATE, IVM_NULL, IVM_NULL);
	ivm_type_setProto(_TYPE, tmp);
	ivm_object_setProto(tmp, _STATE, ivm_vmstate_getTypeProto(_STATE, IVM_OBJECT_T));

}, .des = ivm_function_object_destructor,
   .clone = ivm_function_object_cloner,
   .trav = ivm_function_object_traverser,
   .const_bool = IVM_TRUE)

TYPE_GEN(IVM_CORO_OBJECT_T, coro, sizeof(ivm_coro_object_t),
	IVM_GET_NATIVE_FUNC(_coro_cons), {
	
	ivm_object_t *tmp = ivm_coro_object_new(_STATE, IVM_NULL);
	ivm_type_setProto(_TYPE, tmp);
	ivm_object_setProto(tmp, _STATE, ivm_vmstate_getTypeProto(_STATE, IVM_OBJECT_T));

}, .des = ivm_coro_object_destructor,
   .clone = ivm_coro_object_cloner,
   .trav = ivm_coro_object_traverser,
   .const_bool = IVM_TRUE)

TYPE_GEN(IVM_RANGE_T, range, sizeof(ivm_range_t),
	IVM_GET_NATIVE_FUNC(_range_cons), {
	
	ivm_object_t *tmp = ivm_range_new(_STATE, 0, 0, 1);
	ivm_type_setProto(_TYPE, tmp);
	ivm_object_setProto(tmp, _STATE, ivm_vmstate_getTypeProto(_STATE, IVM_OBJECT_T));

	ivm_object_setSlot(tmp, _STATE, IVM_VMSTATE_CONST(_STATE, C_ITER), IVM_NATIVE_WRAP(_STATE, _range_iter));
	// ivm_object_setOop(tmp, _STATE, IVM_OOP_ID(CALL), IVM_NATIVE_WRAP(_STATE, _range_call));

}, .const_bool = IVM_TRUE)

TYPE_GEN(IVM_RANGE_ITER_T, range_iter, sizeof(ivm_range_iter_t),
	IVM_GET_NATIVE_FUNC(_range_iter_cons), {
	
	ivm_object_t *tmp = ivm_range_iter_new(_STATE, 0, 0, 1);
	ivm_type_setProto(_TYPE, tmp);
	ivm_object_setProto(tmp, _STATE, ivm_vmstate_getTypeProto(_STATE, IVM_OBJECT_T));

	ivm_object_setSlot(tmp, _STATE, IVM_VMSTATE_CONST(_STATE, C_NEXT), IVM_NATIVE_WRAP(_STATE, _range_iter_next));

}, .const_bool = IVM_TRUE)

TYPE_GEN(IVM_LIST_OBJECT_ITER_T, list_iter, sizeof(ivm_list_object_iter_t),
	IVM_GET_NATIVE_FUNC(_list_iter_cons), {
	
	ivm_object_t *tmp = ivm_list_object_iter_new(_STATE, 0);
	ivm_type_setProto(_TYPE, tmp);
	ivm_object_setProto(tmp, _STATE, ivm_vmstate_getTypeProto(_STATE, IVM_OBJECT_T));
	// ivm_object_initSlots(tmp, _STATE, IVM_DEFAULT_SLOT_TABLE_TO_HASH_THRESHOLD);

	ivm_object_setSlot(tmp, _STATE, IVM_VMSTATE_CONST(_STATE, C_NEXT), IVM_NATIVE_WRAP(_STATE, _list_iter_next));

}, .trav = ivm_list_object_iter_traverser,
   .const_bool = IVM_TRUE)
