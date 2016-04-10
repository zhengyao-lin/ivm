#include "num.h"
#include "obj.h"
#include "err.h"
#include "vm.h"

ivm_object_t *ivm_numeric_new(ivm_vmstate_t *state, ivm_numeric_t val)
{
	ivm_object_t *ret = ivm_object_new(state);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("numeric object"));

	ret->type = IVM_NUMERIC_T;
	ret->slots = IVM_NULL;

	ret->u.num = val;

	return ret;
}
