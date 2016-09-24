#include "pub/type.h"
#include "pub/const.h"
#include "pub/inlines.h"
#include "pub/vm.h"
#include "pub/obj.h"

#include "vm/mod/mod.h"

#include "priv.h"
#include "glob.h"

IVM_NATIVE_FUNC(_global_import)
{
	const ivm_string_t *mod;

	MATCH_ARG("s", &mod);

	return ivm_mod_load(mod, NAT_STATE(), NAT_CORO(), NAT_CONTEXT());
}

IVM_NATIVE_FUNC(_global_typeof)
{
	ivm_object_t *obj;

	MATCH_ARG(".", &obj);

	return ivm_string_object_new_r(NAT_STATE(), IVM_OBJECT_GET(obj, TYPE_NAME));
}

IVM_NATIVE_FUNC(_global_is)
{
	ivm_object_t *op1, *op2;

	MATCH_ARG("..", &op1, &op2);

	// 1. op1 == op2 == none
	// 2. op1.type == op2
	// 3. op1.type == op2.type
	
	if (IVM_IS_NONE(NAT_STATE(), op2)) {
		return ivm_numeric_new(NAT_STATE(), IVM_IS_NONE(NAT_STATE(), op1));
	} else if (IVM_IS_BTTYPE(op2, NAT_STATE(), IVM_TYPE_OBJECT_T)) {
		return ivm_numeric_new(NAT_STATE(), IVM_TYPE_OF(op1) == ivm_type_object_getValue(op2));
	}

	return ivm_numeric_new(NAT_STATE(), IVM_TYPE_OF(op1) == IVM_TYPE_OF(op2));

	// RTM_FATAL(IVM_ERROR_MSG_UNEXPECT_IS_OPERAND);
	// return IVM_NONE(NAT_STATE());
}

void
ivm_native_global_bind(ivm_vmstate_t *state,
					   ivm_context_t *ctx)
{
	ivm_type_t *type;
	ivm_type_tag_t i;

	for (i = 0; i < IVM_TYPE_COUNT; i++) {
		type = IVM_BTTYPE(state, i);
		ivm_context_setSlot_r(ctx, state, ivm_type_getName(type), ivm_type_getProto(type));
	}

	ivm_context_setSlot_r(ctx, state, "none", IVM_NONE(state));

	ivm_context_setSlot_r(ctx, state, "import", IVM_NATIVE_WRAP(state, _global_import));
	ivm_context_setSlot_r(ctx, state, "typeof", IVM_NATIVE_WRAP(state, _global_typeof));
	// ivm_context_setSlot_r(ctx, state, "range", IVM_NATIVE_WRAP(state, _global_range));
	ivm_context_setSlot_r(ctx, state, "is", IVM_NATIVE_WRAP(state, _global_is));

	return;
}
