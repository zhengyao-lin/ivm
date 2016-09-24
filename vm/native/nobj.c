#include "pub/type.h"
#include "pub/const.h"
#include "pub/inlines.h"
#include "pub/vm.h"

#include "std/string.h"
#include "std/enc.h"

#include "vm/obj.h"
#include "vm/typeobj.h"

#include "priv.h"
#include "nobj.h"

IVM_NATIVE_FUNC(_object_merge)
{
	ivm_object_t *base;
	ivm_object_t *mergee;
	ivm_number_t overw = 0.0;

	CHECK_BASE_EXIST();
	MATCH_ARG(".*n", &mergee, &overw);

	base = NAT_BASE();
	ivm_object_merge(base, NAT_STATE(), mergee, overw);

	return base;
}

IVM_NATIVE_FUNC(_object_clone)
{
	CHECK_BASE_EXIST();
	return ivm_object_clone(NAT_BASE(), NAT_STATE());
}

IVM_NATIVE_FUNC(_object_type)
{
	CHECK_BASE_EXIST();
	return ivm_type_object_new(NAT_STATE(), IVM_TYPE_OF(NAT_BASE()));
}
