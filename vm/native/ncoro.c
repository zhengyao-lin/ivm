#include "pub/type.h"
#include "pub/const.h"
#include "pub/inlines.h"
#include "pub/vm.h"

#include "vm/coro.h"

#include "priv.h"
#include "ncoro.h"

#define CHECK_CORO_INIT(cobj, out) \
	RTM_ASSERT(ivm_coro_object_getCoro(cobj), IVM_ERROR_MSG_UNINIT_CORO); \
	(out) = ivm_coro_object_getCoro(cobj);

IVM_NATIVE_FUNC(_coro_cons)
{
	CHECK_ARG_1(IVM_CORO_OBJECT_T);
	return ivm_object_clone(NAT_ARG_AT(1), NAT_STATE());
}

IVM_NATIVE_FUNC(_coro_alive)
{
	ivm_coro_object_t *cobj;
	ivm_coro_t *coro;

	CHECK_BASE(IVM_CORO_OBJECT_T);

	cobj = IVM_AS(NAT_BASE(), ivm_coro_object_t);
	CHECK_CORO_INIT(cobj, coro);

	return ivm_bool_new(NAT_STATE(), ivm_coro_isAlive(coro));
}

IVM_NATIVE_FUNC(_coro_exitv)
{
	ivm_coro_t *coro;
	ivm_object_t *val;

	CHECK_BASE(IVM_CORO_OBJECT_T);

	coro = ivm_coro_object_getCoro(NAT_BASE());
	val = ivm_coro_getExitValue(coro);

	return val ? val : IVM_NONE(NAT_STATE());
}
