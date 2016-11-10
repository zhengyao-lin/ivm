#include "pub/type.h"
#include "pub/const.h"
#include "pub/inlines.h"
#include "pub/vm.h"

#include "vm/coro.h"

#include "priv.h"
#include "ncoro.h"

IVM_NATIVE_FUNC(_coro_cons)
{
	CHECK_ARG_1(IVM_CORO_OBJECT_T);
	return ivm_object_clone(NAT_ARG_AT(1), NAT_STATE());
}
