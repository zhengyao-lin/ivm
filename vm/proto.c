#include "pub/vm.h"
#include "pub/inlines.h"

#include "proto.h"
#include "obj.h"
#include "strobj.h"
#include "num.h"

#define PROTO_INIT_NAME(tname) _proto_init_##tname
#define SLOTS_INIT_NAME(tname) _slots_init_##tname

#include "type.req.h"

#define TYPE_GEN(tag, name, size, cons, proto_init, slots_init, ...) \
	IVM_PRIVATE void                                                           \
	PROTO_INIT_NAME(name)(ivm_type_t *_TYPE, ivm_vmstate_t *_STATE) proto_init \
	IVM_PRIVATE void                                                           \
	SLOTS_INIT_NAME(name)(ivm_type_t *_TYPE, ivm_vmstate_t *_STATE) slots_init \

#include "type.def.h"

#undef TYPE_GEN

IVM_PRIVATE
ivm_type_init_proc_t
proto_init_proc[] = {
#define TYPE_GEN(tag, name, size, cons, proto_init, slots_init, ...) PROTO_INIT_NAME(name),
	#include "type.def.h"
#undef TYPE_GEN
};

IVM_PRIVATE
ivm_type_init_proc_t
slots_init_proc[] = {
#define TYPE_GEN(tag, name, size, cons, proto_init, slots_init, ...) SLOTS_INIT_NAME(name),
	#include "type.def.h"
#undef TYPE_GEN
};

void
ivm_proto_initProto(ivm_type_t *type,
					ivm_vmstate_t *state)
{
	ivm_type_init_proc_t tmp = proto_init_proc[type->tag];

	// if (tmp) {
	tmp(type, state);
	// }

	return;
}

void
ivm_proto_initSlots(ivm_type_t *type,
					ivm_vmstate_t *state)
{
	ivm_type_init_proc_t tmp = slots_init_proc[type->tag];

	// if (tmp) {
	tmp(type, state);
	// }

	return;
}
