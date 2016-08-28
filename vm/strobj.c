#include "pub/type.h"
#include "pub/vm.h"
#include "pub/inlines.h"

#include "std/string.h"

#include "strobj.h"

void
ivm_string_object_destructor(ivm_object_t *obj,
							 ivm_vmstate_t *state)
{
	MEM_FREE((ivm_string_t *)IVM_AS(obj, ivm_string_object_t)->val);

	return;
}

void
ivm_string_object_traverser(ivm_object_t *obj,
							ivm_traverser_arg_t *arg)
{
	ivm_string_object_t *str = IVM_AS(obj, ivm_string_object_t);

	if (!str->is_wild) {
		str->val = ivm_string_copyIfNotConst_heap(str->val, arg->heap);
	}

	return;
}
