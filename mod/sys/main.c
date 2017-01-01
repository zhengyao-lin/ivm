#include <stdlib.h>

#include "pub/type.h"
#include "pub/vm.h"
#include "pub/inlines.h"

#include "vm/native/native.h"
#include "vm/native/priv.h"

IVM_NATIVE_FUNC(_sys_exit)
{
	ivm_number_t num = 0;

	MATCH_ARG("*n", &num);

	IVM_EXIT(num);

	return IVM_NONE(NAT_STATE());
}

IVM_NATIVE_FUNC(_sys_abort)
{
	IVM_ABORT();
	return IVM_NONE(NAT_STATE());
}

ivm_object_t *
ivm_mod_main(ivm_vmstate_t *state,
			 ivm_coro_t *coro,
			 ivm_context_t *context)
{
	ivm_object_t *mod = ivm_object_new_c(state, 2);

	ivm_object_setSlot_r(mod, state, "exit", IVM_NATIVE_WRAP(state, _sys_exit));
	ivm_object_setSlot_r(mod, state, "abort", IVM_NATIVE_WRAP(state, _sys_abort));

	return mod;
}
