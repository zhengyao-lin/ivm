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

IVM_NATIVE_FUNC(_object_cons)
{
	CHECK_ARG_1(IVM_OBJECT_T);
	return ivm_object_clone(NAT_ARG_AT(1), NAT_STATE());
}

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

IVM_NATIVE_FUNC(_object_call)
{
	ivm_function_object_t *func;

	CHECK_ARG_COUNT(1);

	func = IVM_AS(NAT_ARG_AT(1), ivm_function_object_t);

	RTM_ASSERT(IVM_IS_BTTYPE(func, NAT_STATE(), IVM_FUNCTION_OBJECT_T),
			   IVM_ERROR_MSG_NOT_TYPE("function", IVM_OBJECT_GET(func, TYPE_NAME)));

	return ivm_coro_callBase_0(NAT_CORO(), NAT_STATE(), func, NAT_BASE());
}

IVM_NATIVE_FUNC(_object_to_s)
{
	ivm_object_t *obj;
	ivm_char_t buf[128];

	CHECK_BASE_EXIST();

	obj = NAT_BASE();

	if (IVM_IS_BTTYPE(obj, NAT_STATE(), IVM_OBJECT_T)) {
		IVM_SNPRINTF(buf, IVM_ARRLEN(buf), "<object at %p>", (void *)obj);
	} else {
		IVM_SNPRINTF(buf, IVM_ARRLEN(buf), "<%s object at %p>", IVM_OBJECT_GET(obj, TYPE_NAME), (void *)obj);
	}

	return ivm_string_object_new_r(NAT_STATE(), buf);
}

IVM_NATIVE_FUNC(_object_slots)
{
	ivm_object_t *obj;
	ivm_slot_table_t *slots;
	ivm_slot_table_iterator_t siter;
	ivm_object_t *buf[2];
	ivm_list_object_t *ret;

	CHECK_BASE_EXIST();

	obj = NAT_BASE();
	slots = IVM_OBJECT_GET(obj, SLOTS);

	ret = IVM_AS(ivm_list_object_new(NAT_STATE()), ivm_list_object_t);

	if (!slots) return IVM_AS_OBJ(ret);

	IVM_SLOT_TABLE_EACHPTR(slots, siter) {
		buf[0] = ivm_string_object_new(NAT_STATE(), IVM_SLOT_TABLE_ITER_GET_KEY(siter));
		buf[1] = IVM_SLOT_TABLE_ITER_GET_VAL(siter);

		ivm_list_object_push(ret, NAT_STATE(), ivm_list_object_new_c(NAT_STATE(), buf, 2));
	}

	return IVM_AS_OBJ(ret);
}

/*
IVM_NATIVE_FUNC(_object_type)
{
	CHECK_BASE_EXIST();
	return ivm_type_object_new(NAT_STATE(), IVM_TYPE_OF(NAT_BASE()));
}
*/
