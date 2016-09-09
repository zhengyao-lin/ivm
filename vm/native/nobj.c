#include "pub/type.h"
#include "pub/const.h"
#include "pub/inlines.h"
#include "pub/vm.h"

#include "std/string.h"
#include "std/enc.h"

#include "vm/obj.h"

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
