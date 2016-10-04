#include "pub/type.h"
#include "pub/const.h"
#include "pub/inlines.h"
#include "pub/vm.h"

#include "std/string.h"
#include "std/enc.h"

#include "vm/func.h"
#include "vm/num.h"

#include "priv.h"
#include "nfunc.h"

IVM_NATIVE_FUNC(_function_cons)
{
	CHECK_ARG_1(IVM_FUNCTION_OBJECT_T);
	return ivm_object_clone(NAT_ARG_AT(1), NAT_STATE());
}
