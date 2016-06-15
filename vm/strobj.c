#include "pub/mem.h"
#include "pub/err.h"
#include "pub/type.h"
#include "pub/vm.h"

#include "std/string.h"
#include "inline/obj.h"
#include "strobj.h"

ivm_object_t *ivm_string_object_new(ivm_vmstate_t *state,
									const ivm_string_t *val)
{
	ivm_string_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), state, IVM_STRING_OBJECT_T);

	ret->val = ivm_string_copyIfNotConst_pool(val, state);

	return IVM_AS_OBJ(ret);
}

void
ivm_string_object_traverser(ivm_object_t *obj,
							ivm_traverser_arg_t *arg)
{
	ivm_string_object_t *str = IVM_AS(obj, ivm_string_object_t);

	str->val = ivm_string_copyIfNotConst_heap(str->val, arg->heap);

	return;
}
