#include "pub/type.h"
#include "pub/const.h"
#include "pub/inlines.h"
#include "pub/vm.h"

#include "vm/typeobj.h"

#include "priv.h"
#include "ntypeobj.h"

IVM_NATIVE_FUNC(_type_cons)
{
	CHECK_ARG_1(IVM_TYPE_OBJECT_T);
	return ivm_object_clone(NAT_ARG_AT(1), NAT_STATE());
}

IVM_NATIVE_FUNC(_type_to_s)
{
	CHECK_BASE(IVM_TYPE_OBJECT_T);
	return ivm_string_object_new_r(NAT_STATE(), ivm_type_getName(ivm_type_object_getValue(NAT_BASE())));
}
