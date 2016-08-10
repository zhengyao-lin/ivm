#ifndef _IVM_VM_NATIVE_PRIV_H_
#define _IVM_VM_NATIVE_PRIV_H_

#include "pub/err.h"

#define RTM_ASSERT(cond, ...) \
	IVM_CORO_NATIVE_ASSERT(NAT_CORO(), NAT_STATE(), (cond), __VA_ARGS__)

#define RTM_FATAL(...) \
	IVM_CORO_NATIVE_ASSERT(NAT_CORO(), NAT_STATE(), 0, __VA_ARGS__)

#define CHECK_BASE(expect) \
	RTM_ASSERT(NAT_BASE() && IVM_IS_TYPE(NAT_BASE(), (expect)), \
			   IVM_NATIVE_ERROR_MSG_WRONG_BASE(NAT_BASE() ? IVM_OBJECT_GET(NAT_BASE(), TYPE_NAME) : "(nil)"))

#define CHECK_ARG_1(type) \
	RTM_ASSERT(NAT_ARGC() && IVM_IS_TYPE(NAT_ARG_AT(1), (type)), IVM_NATIVE_ERROR_MSG_WRONG_ARG)

#define CHECK_ARG_2(t1, t2) \
	RTM_ASSERT(NAT_ARGC() >= 2 &&                       \
			   IVM_IS_TYPE(NAT_ARG_AT(1), (t1)) &&      \
			   IVM_IS_TYPE(NAT_ARG_AT(2), (t2)),        \
			   IVM_NATIVE_ERROR_MSG_WRONG_ARG)

#define CHECK_ARG_3(t1, t2, t3) \
	RTM_ASSERT(NAT_ARGC() >= 3 &&                       \
			   IVM_IS_TYPE(NAT_ARG_AT(1), (t1)) &&      \
			   IVM_IS_TYPE(NAT_ARG_AT(2), (t2)) &&      \
			   IVM_IS_TYPE(NAT_ARG_AT(3), (t3),         \
			   IVM_NATIVE_ERROR_MSG_WRONG_ARG)

#define CHECK_ARG_COUNT(name, count) \
	RTM_ASSERT(NAT_ARGC() >= (count),                                       \
			   IVM_NATIVE_ERROR_MSG_WRONG_ARG_COUNT((name), (count), NAT_ARGC()))

#define WRONG_ARG() \
	RTM_FATAL(IVM_NATIVE_ERROR_MSG_WRONG_ARG)

#define IVM_NATIVE_ERROR_MSG_WRONG_BASE(tn)								"wrong base type <%s>", (tn)
#define IVM_NATIVE_ERROR_MSG_WRONG_ARG									("wrong argument")
#define IVM_NATIVE_ERROR_MSG_WRONG_ARG_COUNT(name, expect, given) \
	"wrong argument count for %s(expect %d, %d given)", (name), (expect), (given)

#endif
