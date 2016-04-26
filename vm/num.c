#include "pub/mem.h"
#include "num.h"
#include "obj.h"
#include "vm.h"
#include "err.h"

ivm_object_t *ivm_numeric_new(ivm_vmstate_t *state, ivm_number_t val)
{
	ivm_numeric_t *ret = MEM_ALLOC(sizeof(*ret));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("numeric"));

	ivm_object_init(IVM_AS_OBJ(ret), state);

	ret->type = ivm_vmstate_getType(state, IVM_NUMERIC_T);
	ret->val = val;

	return IVM_AS_OBJ(ret);
}
