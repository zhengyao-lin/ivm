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

#define CHECK_BASE_CONS(cons) \
	RTM_ASSERT(NAT_BASE() && ivm_type_checkCons(IVM_TYPE_OF(NAT_BASE()), (cons)), \
			   IVM_NATIVE_ERROR_MSG_WRONG_BASE(NAT_BASE() ? IVM_OBJECT_GET(NAT_BASE(), TYPE_NAME) : "(nil)"))

#define CHECK_BASE_TP(cons) CHECK_BASE_CONS(cons)

#define CHECK_BASE_EXIST() \
	RTM_ASSERT(NAT_BASE(), IVM_NATIVE_ERROR_MSG_WRONG_BASE("(nil)"))

#define CHECK_ARG_1(type) \
	RTM_ASSERT(NAT_ARGC() && IVM_IS_BTTYPE(NAT_ARG_AT(1), NAT_STATE(), (type)), IVM_NATIVE_ERROR_MSG_WRONG_ARG)

#define CHECK_ARG_1_TP(cons) \
	RTM_ASSERT(NAT_ARGC() && ivm_type_checkCons(IVM_TYPE_OF(NAT_ARG_AT(1)), (cons)), IVM_NATIVE_ERROR_MSG_WRONG_ARG)

#define CHECK_ARG_2(t1, t2) \
	RTM_ASSERT(NAT_ARGC() >= 2 &&                                      \
			   IVM_IS_BTTYPE(NAT_ARG_AT(1), NAT_STATE(), (t1)) &&      \
			   IVM_IS_BTTYPE(NAT_ARG_AT(2), NAT_STATE(), (t2)),        \
			   IVM_NATIVE_ERROR_MSG_WRONG_ARG)

#define CHECK_ARG_3(t1, t2, t3) \
	RTM_ASSERT(NAT_ARGC() >= 3 &&                                      \
			   IVM_IS_BTTYPE(NAT_ARG_AT(1), NAT_STATE(), (t1)) &&      \
			   IVM_IS_BTTYPE(NAT_ARG_AT(2), NAT_STATE(), (t2)) &&      \
			   IVM_IS_BTTYPE(NAT_ARG_AT(3), NAT_STATE(), (t3),         \
			   IVM_NATIVE_ERROR_MSG_WRONG_ARG)

#define CHECK_OVERFLOW(n) \
	RTM_ASSERT(ivm_number_canbeLong(n), IVM_ERROR_MSG_TO_LONG_OVERFLOW(n));

#define HAS_ARG(n) \
	(NAT_ARGC() >= (n) && !IVM_IS_NONE(NAT_STATE(), NAT_ARG_AT(n)))

#define MATCH_ARG(rule, ...) \
	{                                                                                   \
		ivm_int_t __match_ret__                                                         \
		= ivm_native_matchArgument(NAT_ARG(), NAT_STATE(), (rule), __VA_ARGS__);        \
		RTM_ASSERT(!__match_ret__, IVM_NATIVE_ERROR_MSG_WRONG_ARG_AT(__match_ret__));   \
	}

#define GET_BASE_AS(type) IVM_AS(NAT_BASE(), type)

#define CHECK_ARG_COUNT(count) \
	RTM_ASSERT(NAT_ARGC() >= (count),                                       \
			   IVM_NATIVE_ERROR_MSG_WRONG_ARG_COUNT((count), NAT_ARGC()))

#define WRONG_ARG_COUNT(expect) \
	RTM_FATAL(IVM_NATIVE_ERROR_MSG_WRONG_ARG_COUNT((expect), NAT_ARGC()))

#define WRONG_ARG() \
	RTM_FATAL(IVM_NATIVE_ERROR_MSG_WRONG_ARG)

#define IVM_NATIVE_ERROR_MSG_WRONG_BASE(tn)								"wrong base type <%s>", (tn)
#define IVM_NATIVE_ERROR_MSG_WRONG_ARG									("wrong argument")
#define IVM_NATIVE_ERROR_MSG_WRONG_ARG_COUNT(expect, given)				"wrong argument count(expect %d, %d given)", (ivm_int_t)(expect), (ivm_int_t)(given)
#define IVM_NATIVE_ERROR_MSG_WRONG_ARG_AT(i)							"wrong %dth argument", (i)

#define BUILTIN_FUNC(name, instrs) IVM_BUILTIN_FUNC(name) {                   \
		ivm_exec_t *__exec__;                                                 \
		ivm_function_t *__ret__;                                              \
	                                                                          \
		__exec__ = ivm_exec_new(ivm_vmstate_getConstPool(NAT_STATE()));       \
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
