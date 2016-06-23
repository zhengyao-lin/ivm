#include "pub/mem.h"
#include "pub/err.h"
#include "pub/type.h"
#include "pub/vm.h"
#include "pub/inlines.h"

#include "std/string.h"

#include "strobj.h"

void
ivm_string_object_traverser(ivm_object_t *obj,
							ivm_traverser_arg_t *arg)
{
	ivm_string_object_t *str = IVM_AS(obj, ivm_string_object_t);

	str->val = ivm_string_copyIfNotConst_heap(str->val, arg->heap);

	return;
}
