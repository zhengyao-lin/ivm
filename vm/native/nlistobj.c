#include "pub/type.h"
#include "pub/const.h"
#include "pub/inlines.h"
#include "pub/vm.h"

#include "vm/listobj.h"
#include "vm/num.h"

#include "priv.h"
#include "nlistobj.h"

#define ERR_MSG_ILLEGAL_STEP "the absolute value of the step must be greater or equal than 1"

IVM_NATIVE_FUNC(_list_cons)
{
	ivm_object_t *obj, *iter, *list, *base, *ret, *next;
	ivm_function_object_t *iter_func, *next_func;

	CHECK_ARG_COUNT(1);

	obj = NAT_ARG_AT(1);

	if (IVM_IS_BTTYPE(obj, NAT_STATE(), IVM_LIST_OBJECT_T)) {
		return ivm_object_clone(obj, NAT_STATE());
	}

	iter = ivm_object_getSlot(obj, NAT_STATE(), IVM_VMSTATE_CONST(NAT_STATE(), C_ITER));

	if (!iter || !(iter_func = ivm_object_callable(iter, NAT_STATE(), &base))) {
		RTM_FATAL(IVM_ERROR_MSG_NON_ITERABLE);
	}

	iter = ivm_coro_callBase_n(NAT_CORO(), NAT_STATE(), iter_func, base ? base : obj);

	list = ivm_list_object_new(NAT_STATE(), 0);
	ivm_vmstack_push(NAT_CORO(), list);
	ivm_vmstack_push(NAT_CORO(), iter);

	while (1) {
		iter = ivm_vmstack_prev(NAT_CORO(), 1);
		next = ivm_object_getSlot(iter, NAT_STATE(), IVM_VMSTATE_CONST(NAT_STATE(), C_NEXT));

		if (!next || !(next_func = ivm_object_callable(next, NAT_STATE(), &base))) {
			RTM_FATAL(IVM_ERROR_MSG_NON_ITERABLE);
		}

		ret = ivm_coro_callBase_n(NAT_CORO(), NAT_STATE(), next_func, base ? base : iter);

		list = ivm_vmstack_prev(NAT_CORO(), 2);
		if (!ret)
			break;
		
		ivm_list_object_push(IVM_AS(list, ivm_list_object_t), NAT_STATE(), ret);
	}

	return list;
}

IVM_NATIVE_FUNC(_list_size)
{
	CHECK_BASE(IVM_LIST_OBJECT_T);
	return ivm_numeric_new(NAT_STATE(), ivm_list_object_getSize(IVM_AS(NAT_BASE(), ivm_list_object_t)));
}

IVM_NATIVE_FUNC(_list_push)
{
	ivm_size_t size;
	ivm_object_t *obj;

	CHECK_BASE(IVM_LIST_OBJECT_T);
	CHECK_ARG_COUNT(1);

	obj = NAT_ARG_AT(1);

	size = ivm_list_object_push(IVM_AS(NAT_BASE(), ivm_list_object_t), NAT_STATE(), obj);
	if (!size) {
		return IVM_NULL;
	}
	
	return ivm_numeric_new(NAT_STATE(), size);
}

IVM_NATIVE_FUNC(_list_slice)
{
	ivm_number_t start, end, step = 1;
	ivm_long_t size;
	ivm_object_t **lst;
	ivm_list_object_t *list;
	ivm_object_t *ret;

	CHECK_BASE(IVM_LIST_OBJECT_T);

	list = NAT_BASE_C(ivm_list_object_t);
	end = ivm_list_object_getSize(list);

	MATCH_ARG("n*nn", &start, &end, &step);

	RTM_ASSERT(step <= -1 || step >= 1, ERR_MSG_ILLEGAL_STEP);

	start = ivm_list_object_realIndex(list, start);
	end = ivm_list_object_realIndex(list, end);
	step = (ivm_long_t)step;
	size = ivm_list_object_getSize(list);
	lst = ivm_list_object_core(list);

	if (start > size) start = size;
	if (end > size) end = size;

	if (start == end) ret = ivm_list_object_new(NAT_STATE(), 0);
	else if (start < end) {
		if (step < 0) {
			ret = ivm_list_object_new(NAT_STATE(), 0);
		} else {
			ret = ivm_list_object_new_c(NAT_STATE(), lst + (ivm_size_t)start, end - start);
			ivm_list_object_step(IVM_AS(ret, ivm_list_object_t), step);
		}
	} else {
		if (step > 0) {
			ret = ivm_list_object_new(NAT_STATE(), 0);
		} else {
			ret = ivm_list_object_new_c(NAT_STATE(), lst + (ivm_size_t)end + 1, start - end);
			ivm_list_object_reverse(IVM_AS(ret, ivm_list_object_t));
			ivm_list_object_step(IVM_AS(ret, ivm_list_object_t), -step);
		}
	}

	return ret;
}

IVM_NATIVE_FUNC(_list_iter)
{
	CHECK_BASE(IVM_LIST_OBJECT_T);
	return ivm_list_object_iter_new(NAT_STATE(), IVM_AS(NAT_BASE(), ivm_list_object_t));
}

IVM_NATIVE_FUNC(_list_iter_cons)
{
	CHECK_ARG_1(IVM_LIST_OBJECT_ITER_T);
	return ivm_object_clone(NAT_ARG_AT(1), NAT_STATE());
}

IVM_NATIVE_FUNC(_list_iter_next)
{
	ivm_list_object_iter_t *iter;
	ivm_object_t *ret;

	CHECK_BASE(IVM_LIST_OBJECT_ITER_T);

	iter = IVM_AS(NAT_BASE(), ivm_list_object_iter_t);
	ret = ivm_list_object_iter_next(iter, NAT_STATE());

	RTM_ASSERT(ret, IVM_ERROR_MSG_ITER_END);

	return ret;
}
