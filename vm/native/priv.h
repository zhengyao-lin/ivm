#ifndef _IVM_VM_NATIVE_PRIV_H_
#define _IVM_VM_NATIVE_PRIV_H_

#include "pub/err.h"

#define CHECK_BASE(expect) \
	IVM_ASSERT(IVM_IS_TYPE(arg.base, (expect)), IVM_NATIVE_ERROR_MSG_WRONG_BASE)

#define CHECK_ARG_1(type) \
	IVM_ASSERT(arg.argc && IVM_IS_TYPE(arg.argv[0], (type)), IVM_NATIVE_ERROR_MSG_WRONG_ARG)

#define CHECK_ARG_2(t1, t2) \
	IVM_ASSERT(arg.argc >= 2 &&                       \
			   IVM_IS_TYPE(arg.argv[0], (t1)) &&      \
			   IVM_IS_TYPE(arg.argv[1], (t2)),        \
			   IVM_NATIVE_ERROR_MSG_WRONG_ARG)

#define CHECK_ARG_3(t1, t2, t3) \
	IVM_ASSERT(arg.argc >= 3 &&                       \
			   IVM_IS_TYPE(arg.argv[0], (t1)) &&      \
			   IVM_IS_TYPE(arg.argv[1], (t2)) &&      \
			   IVM_IS_TYPE(arg.argv[2], (t3),         \
			   IVM_NATIVE_ERROR_MSG_WRONG_ARG)

#define WRONG_ARG() \
	IVM_FATAL(IVM_NATIVE_ERROR_MSG_WRONG_ARG)

#define IVM_NATIVE_ERROR_MSG_WRONG_BASE				"wrong base type"
#define IVM_NATIVE_ERROR_MSG_WRONG_ARG				"wrong argument"

#endif
