#include "pub/type.h"
#include "pub/vm.h"
#include "pub/inlines.h"

#include "std/heap.h"

#include "typeobj.h"

void
ivm_type_object_traverser(ivm_object_t *obj,
						  ivm_traverser_arg_t *arg)
{
	ivm_type_t *type = IVM_AS(obj, ivm_type_object_t)->val;
	ivm_object_t *proto;

	if (!ivm_type_isBuiltin(type)) {
		// avoid duplicated copy(prototypes of built-in types is in the root)

		proto = ivm_type_getProto(type);
		if (proto && !ivm_heap_isIn(arg->heap, proto)) {
			ivm_type_setProto(type, ivm_collector_copyObject(proto, arg));
		}
	}

	return;
}
