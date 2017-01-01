#include <stdlib.h>

#include "pub/type.h"
#include "pub/vm.h"
#include "pub/inlines.h"

#include "vm/env.h"

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
	ivm_list_object_t *argv_obj;
	const ivm_char_t **argv = ivm_env_getArgv();
	ivm_int_t argc = ivm_env_getArgc(), i;

	ivm_object_setSlot_r(mod, state, "exit", IVM_NATIVE_WRAP(state, _sys_exit));
	ivm_object_setSlot_r(mod, state, "abort", IVM_NATIVE_WRAP(state, _sys_abort));

	argv_obj = IVM_AS(ivm_list_object_new(state, 0), ivm_list_object_t);
	ivm_object_setSlot_r(mod, state, "argv", IVM_AS_OBJ(argv_obj));

	// IVM_TRACE("arg: %d %s\n", argc, argc ? argv[0] : "nop");

	for (i = 0; i < argc; i++) {
		ivm_list_object_push(argv_obj, state, ivm_string_object_new_r(state, argv[i]));
	}

	return mod;
}
