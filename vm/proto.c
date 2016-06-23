#include "pub/vm.h"
#include "pub/inlines.h"

#include "proto.h"
#include "obj.h"
#include "strobj.h"
#include "num.h"

IVM_PRIVATE
void
init_object(ivm_type_t *type,
			ivm_vmstate_t *state)
{
	ivm_type_setProto(type, ivm_object_new(state));
	return;
}

IVM_PRIVATE
void
init_numeric(ivm_type_t *type,
			 ivm_vmstate_t *state)
{
	ivm_object_t *tmp = ivm_numeric_new(state, IVM_NUM(0));
	ivm_type_setProto(type, tmp);
	IVM_OBJECT_SET(tmp, PROTO, ivm_vmstate_getTypeProto(state, IVM_OBJECT_T));

	return;
}

IVM_PRIVATE
void
init_string(ivm_type_t *type,
			ivm_vmstate_t *state)
{
	ivm_object_t *tmp = ivm_string_object_new(state, IVM_NULL);
	ivm_type_setProto(type, tmp);
	IVM_OBJECT_SET(tmp, PROTO, ivm_vmstate_getTypeProto(state, IVM_OBJECT_T));
	
	return;
}

IVM_PRIVATE
void
init_function(ivm_type_t *type,
			  ivm_vmstate_t *state)
{
	ivm_object_t *tmp = ivm_function_object_new(state, IVM_NULL, IVM_NULL);
	ivm_type_setProto(type, tmp);
	IVM_OBJECT_SET(tmp, PROTO, ivm_vmstate_getTypeProto(state, IVM_OBJECT_T));
	
	return;
}

IVM_PRIVATE
ivm_type_init_proc_t
init_proc[] = {
	IVM_NULL,
	IVM_NULL,
	init_object,
	init_numeric,
	init_string,
	init_function
};

void
ivm_proto_initType(ivm_type_t *type,
				   ivm_vmstate_t *state)
{
	ivm_type_init_proc_t tmp = init_proc[type->tag];

	if (tmp) {
		tmp(type, state);
	}

	return;
}
