#include <stdio.h>
#include <stdlib.h>

#include "pub/type.h"
#include "pub/vm.h"
#include "pub/inlines.h"

#include "vm/mod/mod.h"
#include "vm/native/native.h"
#include "vm/native/priv.h"

ivm_object_t *
ivm_mod_main(ivm_vmstate_t *state,
			 ivm_coro_t *coro,
			 ivm_context_t *context)
{
	ivm_object_t *mod = ivm_object_new(state);

	ivm_vmstate_enableThread(state);

	return mod;
}
