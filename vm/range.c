#include "pub/type.h"
#include "pub/vm.h"
#include "pub/inlines.h"

#include "range.h"

ivm_object_t *
ivm_range_new(ivm_vmstate_t *state,
			  ivm_long_t from, ivm_long_t to,
			  ivm_long_t step)
{
	ivm_range_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));
	ivm_long_t delta, count;

	ivm_object_init(IVM_AS_OBJ(ret), state, IVM_RANGE_T);

	delta = to - from;
	count = delta / step;

	if (count < 0) {
		ret->from = ret->to = ret->step = 0;
	} else {
		ret->from = from;
		
		if (delta % step) { // align
			ret->to = from + ((count + 1) * step);
		} else {
			ret->to = to;
		}

		ret->step = step;
	}

	return IVM_AS_OBJ(ret);
}

ivm_object_t *
ivm_range_iter_new(struct ivm_vmstate_t_tag *state,
				   ivm_long_t cur, ivm_long_t end,
				   ivm_long_t step)
{
	ivm_range_iter_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), state, IVM_RANGE_ITER_T);

	ret->cur = cur;
	ret->end = end;
	ret->step = step;

	return IVM_AS_OBJ(ret);
}
