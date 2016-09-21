#include "pub/type.h"
#include "pub/const.h"
#include "pub/inlines.h"
#include "pub/vm.h"

#include "vm/range.h"
#include "vm/num.h"

#include "priv.h"
#include "nrange.h"

IVM_NATIVE_FUNC(_range_iter)
{
	ivm_range_t *range;

	CHECK_BASE(IVM_RANGE_T);

	range = IVM_AS(NAT_BASE(), ivm_range_t);

	return ivm_range_iter_new(NAT_STATE(), range->from, range->to, range->step);
}

IVM_NATIVE_FUNC(_range_iter_next)
{
	ivm_range_iter_t *iter;
	ivm_object_t *ret;

	CHECK_BASE(IVM_RANGE_ITER_T);

	iter = IVM_AS(NAT_BASE(), ivm_range_iter_t);

	RTM_ASSERT(iter->cur != iter->end, IVM_ERROR_MSG_ITER_END);

	ret = ivm_numeric_new(NAT_STATE(), iter->cur);
	iter->cur += iter->step;

	return ret;
}
