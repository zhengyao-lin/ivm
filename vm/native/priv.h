#ifndef _IVM_VM_NATIVE_PRIV_H_
#define _IVM_VM_NATIVE_PRIV_H_

#include "pub/err.h"
#include "pub/obj.h"
#include "pub/vm.h"

#include "native.h"

#define RTM_ASSERT(cond, ...) \
	IVM_CORO_NATIVE_ASSERT(NAT_CORO(), NAT_STATE(), (cond), __VA_ARGS__)

#define RTM_ASSERT_C(coro, state, cond, ...) \
	IVM_CORO_NATIVE_ASSERT((coro), (state), (cond), __VA_ARGS__)

#define RTM_FATAL(...) \
	IVM_CORO_NATIVE_ASSERT(NAT_CORO(), NAT_STATE(), 0, __VA_ARGS__)

#define RTM_MEMCHECK(ptr) \
	IVM_CORO_NATIVE_ASSERT(NAT_CORO(), NAT_STATE(), (ptr), IVM_ERROR_MSG_MEMORY_ERROR)

#define CHECK_BASE(expect) \
	RTM_ASSERT(NAT_BASE() && IVM_IS_BTTYPE(NAT_BASE(), NAT_STATE(), (expect)), \
			   IVM_NATIVE_ERROR_MSG_WRONG_BASE(NAT_BASE() ? IVM_OBJECT_GET(NAT_BASE(), TYPE_NAME) : "(nil)"))

#define CHECK_BASE_UID(uid) \
	RTM_ASSERT(NAT_BASE() && ivm_type_checkUID(IVM_TYPE_OF(NAT_BASE()), (uid)), \
			   IVM_NATIVE_ERROR_MSG_WRONG_BASE(NAT_BASE() ? IVM_OBJECT_GET(NAT_BASE(), TYPE_NAME) : "(nil)"))

#define CHECK_BASE_TP(uid) CHECK_BASE_UID(uid)

#define CHECK_BASE_EXIST() \
	RTM_ASSERT(NAT_BASE(), IVM_NATIVE_ERROR_MSG_WRONG_BASE("(nil)"))

#define CHECK_ARG_1(type) \
	RTM_ASSERT(NAT_ARGC(), IVM_ERROR_MSG_MISSING_ARG(1, ivm_vmstate_getTypeName(NAT_STATE(), (type))))  \
	RTM_ASSERT(                                                                                         \
		IVM_IS_BTTYPE(NAT_ARG_AT(1), NAT_STATE(), (type)),                                              \
		IVM_ERROR_MSG_WRONG_ARG(                                                                        \
			1, ivm_vmstate_getTypeName(NAT_STATE(), (type)),                                            \
			IVM_OBJECT_GET(NAT_ARG_AT(1), TYPE_NAME)                                                    \
		)                                                                                               \
	)

#define CHECK_ARG_1_C(type) \
	(NAT_ARGC() >= 1 && IVM_IS_BTTYPE(NAT_ARG_AT(1), NAT_STATE(), (type)))

#define CHECK_ARG_1_TP(uid) \
	RTM_ASSERT(NAT_ARGC(), IVM_ERROR_MSG_MISSING_ARG(1, ivm_type_getName(IVM_TPTYPE(NAT_STATE(), (uid)))))  \
	RTM_ASSERT(                                                                                             \
		ivm_type_checkUID(IVM_TYPE_OF(NAT_ARG_AT(1)), (uid)),                                               \
		IVM_ERROR_MSG_WRONG_ARG(                                                                            \
			1, ivm_type_getName(IVM_TPTYPE(NAT_STATE(), (uid))),                                            \
			IVM_OBJECT_GET(NAT_ARG_AT(1), TYPE_NAME)                                                        \
		)                                                                                                   \
	)

#define CHECK_OVERFLOW(n) \
	RTM_ASSERT(ivm_number_canbeLong(n), IVM_ERROR_MSG_TO_LONG_OVERFLOW(n));

#define HAS_ARG(n) \
	(NAT_ARGC() >= (n) && !IVM_IS_NONE(NAT_STATE(), NAT_ARG_AT(n)))

#define MATCH_ARG(rule, ...) \
	{                                                                                            \
		if (!ivm_native_matchArgument(NAT_ARG(), NAT_STATE(), __func__, (rule), __VA_ARGS__)) {  \
			return IVM_NULL;                                                                     \
		}                                                                                        \
	}

#define GET_BASE_AS(type) IVM_AS(NAT_BASE(), type)

#define CHECK_ARG_COUNT(count) \
	RTM_ASSERT(NAT_ARGC() >= (count),                                       \
			   IVM_NATIVE_ERROR_MSG_WRONG_ARG_COUNT((count), NAT_ARGC()))

#define WRONG_ARG_COUNT(expect) \
	RTM_FATAL(IVM_NATIVE_ERROR_MSG_WRONG_ARG_COUNT((expect), NAT_ARGC()))

#define IVM_NATIVE_ERROR_MSG_WRONG_BASE(tn)								"wrong base type <%s>", (tn)
#define IVM_NATIVE_ERROR_MSG_WRONG_ARG_COUNT(expect, given)				"wrong argument count(expecting %d, %d given)", (ivm_int_t)(expect), (ivm_int_t)(given)

#define BUILTIN_FUNC(name, instrs) IVM_BUILTIN_FUNC(name) {                   \
		ivm_exec_t *__exec__;                                                 \
		ivm_function_t *__ret__;                                              \
	                                                                          \
		__exec__ = ivm_exec_new(ivm_vmstate_getConstPool(NAT_STATE()), -1);   \
		ivm_ref_inc(__exec__);                                                \
	                                                                          \
		instrs;                                                               \
	                                                                          \
		__ret__ = ivm_function_new(NAT_STATE(), __exec__);                    \
		ivm_vmstate_registerFunc(NAT_STATE(), __ret__);                       \
		ivm_exec_free(__exec__);                                              \
                                                                              \
		return __ret__;                                                       \
	}

#define I(...) ivm_exec_addInstr(__exec__, __VA_ARGS__);

#endif
