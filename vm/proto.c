#include "pub/vm.h"
#include "pub/inlines.h"

#include "proto.h"
#include "obj.h"
#include "strobj.h"
#include "num.h"

#define PROTO_INIT_NAME(tname) _proto_init_##tname

#include "type.req.h"

#define TYPE_GEN(tag, name, size, cons, proto_init, ...) \
	IVM_PRIVATE                                                      \
	void                                                             \
	PROTO_INIT_NAME(name)(ivm_type_t *_TYPE, ivm_vmstate_t *_STATE)  \
	proto_init

#include "type.def.h"

#undef TYPE_GEN

IVM_PRIVATE
ivm_type_init_proc_t
init_proc[] = {
#define TYPE_GEN(tag, name, size, cons, proto_init, ...) PROTO_INIT_NAME(name),
	#include "type.def.h"
#undef TYPE_GEN
};

void
ivm_proto_initType(ivm_type_t *type,
				   ivm_vmstate_t *state)
{
	ivm_type_init_proc_t tmp = init_proc[type->tag];

	// if (tmp) {
	tmp(type, state);
	// }

	return;
}
