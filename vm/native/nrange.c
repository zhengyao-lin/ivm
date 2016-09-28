#include "pub/type.h"
#include "pub/const.h"
#include "pub/inlines.h"
#include "pub/vm.h"

#include "vm/range.h"
#include "vm/num.h"

#include "priv.h"
#include "nrange.h"

IVM_NATIVE_FUNC(_range_cons)
{
	ivm_number_t from = 0, to, step = 1;

	if (NAT_ARGC() == 1) {
		MATCH_ARG("n", &to);
	} else {
		MATCH_ARG("nn*n", &from, &to, &step);
	} 

	return ivm_range_new(NAT_STATE(), from, to, step);
}

IVM_NATIVE_FUNC(_range_iter)
{
	ivm_range_t *range;

	CHECK_BASE(IVM_RANGE_T);

	range = IVM_AS(NAT_BASE(), ivm_range_t);

	return ivm_range_iter_new(NAT_STATE(), range->from, range->to, range->step);
}

IVM_NATIVE_FUNC(_range_iter_cons)
{
	CHECK_ARG_1(IVM_RANGE_ITER_T);
	return ivm_object_clone(NAT_ARG_AT(1), NAT_STATE());
}

IVM_NATIVE_FUNC(_range_iter_next)
{
	ivm_object_t *ret;

	CHECK_BASE(IVM_RANGE_ITER_T);

	ret = ivm_range_iter_next(IVM_AS(NAT_BASE(), ivm_range_iter_t), NAT_STATE());
	RTM_ASSERT(ret, IVM_ERROR_MSG_ITER_END);

	return ret;
}
