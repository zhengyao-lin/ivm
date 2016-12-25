#include <stdio.h>
#include <stdlib.h>

#include "pub/type.h"
#include "pub/vm.h"
#include "pub/inlines.h"

#include "vm/mod/mod.h"
#include "vm/native/native.h"
#include "vm/native/priv.h"

IVM_NATIVE_FUNC(_mthread_spawn)
{
	ivm_object_t *arg = IVM_NULL;
	ivm_coro_t *coro;

	MATCH_ARG("c*.", &coro, &arg);

	RTM_ASSERT(ivm_coro_canResume(coro),
			   IVM_ERROR_MSG_CORO_UNABLE_RESUME(coro));

	return ivm_vmstate_spawnThread(NAT_STATE(), coro, arg);
}

IVM_NATIVE_FUNC(_mthread_join)
{
	ivm_vmstate_joinAllThread(NAT_STATE(), IVM_FALSE);
	return IVM_NONE(NAT_STATE());
}

ivm_object_t *
ivm_mod_main(ivm_vmstate_t *state,
			 ivm_coro_t *coro,
			 ivm_context_t *context)
{
	ivm_object_t *mod = ivm_object_new(state);

	ivm_vmstate_enableThread(state);

	ivm_object_setSlot_r(mod, state, "spawn", IVM_NATIVE_WRAP(state, _mthread_spawn));
	ivm_object_setSlot_r(mod, state, "join", IVM_NATIVE_WRAP(state, _mthread_join));

	return mod;
}
