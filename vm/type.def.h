TYPE_GEN(IVM_UNDEFINED_T, undefined, sizeof(ivm_object_t), {
}, .const_bool = IVM_FALSE)

TYPE_GEN(IVM_NULL_T, null, sizeof(ivm_object_t), {
}, .const_bool = IVM_FALSE)

TYPE_GEN(IVM_OBJECT_T, object, sizeof(ivm_object_t), {
	ivm_type_setProto(_TYPE, ivm_object_new(_STATE));
}, .const_bool = IVM_TRUE)

TYPE_GEN(IVM_NUMERIC_T, numeric, sizeof(ivm_numeric_t), {
	
	ivm_object_t *tmp = ivm_numeric_new(_STATE, IVM_NUM(0));
	ivm_type_setProto(_TYPE, tmp);
	ivm_object_setProto(tmp, _STATE, ivm_vmstate_getTypeProto(_STATE, IVM_OBJECT_T));

}, .to_bool = ivm_numeric_isTrue)

TYPE_GEN(IVM_STRING_OBJECT_T, string, sizeof(ivm_string_object_t), {
	
	ivm_object_t *tmp = ivm_string_object_new(_STATE, IVM_NULL);
	ivm_type_setProto(_TYPE, tmp);
	ivm_object_setProto(tmp, _STATE, ivm_vmstate_getTypeProto(_STATE, IVM_OBJECT_T));

}, .trav = ivm_string_object_traverser, .const_bool = IVM_TRUE)

TYPE_GEN(IVM_LIST_OBJECT_T, list, sizeof(ivm_list_object_t), {
	
	ivm_object_t *tmp = ivm_list_object_new(_STATE, 0);
	ivm_type_setProto(_TYPE, tmp);
	ivm_object_setProto(tmp, _STATE, ivm_vmstate_getTypeProto(_STATE, IVM_OBJECT_T));

	ivm_object_setSlot(tmp, _STATE, IVM_VMSTATE_CONST(_STATE, C_SIZE), IVM_NATIVE_WRAP(_STATE, _list_size));
	ivm_object_setSlot_r(tmp, _STATE, "push", IVM_NATIVE_WRAP(_STATE, _list_push));
	ivm_object_setSlot_r(tmp, _STATE, "slice", IVM_NATIVE_WRAP(_STATE, _list_slice));

}, .des = ivm_list_object_destructor,
   .trav = ivm_list_object_traverser,
   .const_bool = IVM_TRUE)

TYPE_GEN(IVM_FUNCTION_OBJECT_T, function, sizeof(ivm_function_object_t), {
	
	ivm_object_t *tmp = ivm_function_object_new(_STATE, IVM_NULL, IVM_NULL);
	ivm_type_setProto(_TYPE, tmp);
	ivm_object_setProto(tmp, _STATE, ivm_vmstate_getTypeProto(_STATE, IVM_OBJECT_T));

}, .des = ivm_function_object_destructor,
   .trav = ivm_function_object_traverser,
   .const_bool = IVM_TRUE)
