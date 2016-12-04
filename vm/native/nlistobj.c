#include "pub/type.h"
#include "pub/const.h"
#include "pub/inlines.h"
#include "pub/vm.h"

#include "vm/listobj.h"
#include "vm/num.h"

#include "priv.h"
#include "nlistobj.h"

#define ERR_MSG_ILLEGAL_STEP "the absolute value of the step must be greater or equal than 1"

/* IVM_NATIVE_FUNC(_list_cons)
{
	CHECK_ARG_1(IVM_LIST_OBJECT_T);
	return ivm_object_clone(NAT_ARG_AT(1), NAT_STATE());
} */

BUILTIN_FUNC(_list_cons, {
	I(NEW_LIST, 0)
	I(CHECK2, 12)

	I(PUSH_BLOCK)
	I(DUP_PREV_BLOCK_N, 1)
	I(GET_SLOT_N, "iter")
	I(INVOKE_BASE, 0)
	
	I(ITER_NEXT, 6)
	I(INVOKE_BASE, 0)
	I(RPROT_CAC)

	I(DUP_PREV_BLOCK_N, 0)
    I(PUSH_LIST)

    I(JUMP, -5)
    I(POP_BLOCK)
    I(RETURN)
})

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

IVM_NATIVE_FUNC(_list_pop)
{
	ivm_list_object_t *list;

	CHECK_BASE(IVM_LIST_OBJECT_T);

	list = IVM_AS(NAT_BASE(), ivm_list_object_t);
	RTM_ASSERT(ivm_list_object_getSize(list), IVM_ERROR_MSG_TOO_SMALL_LIST);
	
	return ivm_list_object_pop(list);
}

IVM_NATIVE_FUNC(_list_top)
{
	ivm_size_t size;
	ivm_list_object_t *list;

	CHECK_BASE(IVM_LIST_OBJECT_T);

	list = IVM_AS(NAT_BASE(), ivm_list_object_t);
	size = ivm_list_object_getSize(list);
	RTM_ASSERT(size, IVM_ERROR_MSG_TOO_SMALL_LIST);
	
	return _ivm_list_object_get_c(list, size - 1);
}

IVM_NATIVE_FUNC(_list_slice)
{
	ivm_number_t start = 0, end, step = 1;
	ivm_long_t startl, endl;
	ivm_long_t size;
	ivm_object_t **lst;
	ivm_list_object_t *list;
	ivm_object_t *ret;

	CHECK_BASE(IVM_LIST_OBJECT_T);

	list = NAT_BASE_C(ivm_list_object_t);
	end = ivm_list_object_getSize(list);

	MATCH_ARG("*nnn", &start, &end, &step);

	RTM_ASSERT(step <= -1 || step >= 1, ERR_MSG_ILLEGAL_STEP);
	
	CHECK_OVERFLOW(start);
	CHECK_OVERFLOW(end);

	startl = ivm_list_object_realIndex(list, start);
	endl = ivm_list_object_realIndex(list, end);

	step = (ivm_long_t)step;
	size = ivm_list_object_getSize(list);
	lst = ivm_list_object_core(list);

	if (startl > size) startl = size;
	if (endl > size) endl = size;

	if (startl == endl) ret = ivm_list_object_new(NAT_STATE(), 0);
	else if (startl < endl) {
		if (step < 0) {
			ret = ivm_list_object_new(NAT_STATE(), 0);
		} else {
			ret = ivm_list_object_new_c(NAT_STATE(), lst + (ivm_size_t)startl, endl - startl);
			ivm_list_object_step(IVM_AS(ret, ivm_list_object_t), step);
		}
	} else {
		if (step > 0) {
			ret = ivm_list_object_new(NAT_STATE(), 0);
		} else {
			ret = ivm_list_object_new_c(NAT_STATE(), lst + (ivm_size_t)endl + 1, startl - endl);
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
