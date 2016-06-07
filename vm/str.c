#include "pub/mem.h"
#include "inline/obj.h"
#include "str.h"
#include "type.h"
#include "vm.h"
#include "bit.h"
#include "gc/heap.h"
#include "gc/gc.h"
#include "err.h"

ivm_object_t *ivm_string_object_new(ivm_vmstate_t *state,
									const ivm_string_t *val)
{
	ivm_string_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), state, IVM_STRING_OBJECT_T);

	ret->val = ivm_string_copyIfNotConst_state(val, state);

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
