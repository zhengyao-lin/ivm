#include "pub/type.h"
#include "pub/vm.h"
#include "pub/inlines.h"

#include "typeobj.h"

void
ivm_type_object_traverser(ivm_object_t *obj,
						  ivm_traverser_arg_t *arg)
{
	ivm_type_object_t *type = IVM_AS(obj, ivm_type_object_t);

	ivm_type_setProto(type->val, ivm_collector_copyObject(ivm_type_getProto(type->val), arg));

	return;
}
