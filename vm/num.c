#include "num.h"
#include "obj.h"
#include "err.h"
#include "vm.h"

ivm_object_t *ivm_new_numeric(ivm_vmstate_t *state, ivm_numeric_t val)
{
	ivm_object_t *ret = ivm_new_object(state);

	IVM_ASSERT(ret, "Failed to allocate new room for new object");

	ret->type = IVM_NUMERIC_T;
	ret->slots = IVM_NULL;

	ret->u.num = val;

	return ret;
}
