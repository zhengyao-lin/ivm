#include "pub/inlines.h"
#include "pub/vm.h"

ivm_object_t *
ivm_mod_main(ivm_vmstate_t *state,
			 ivm_coro_t *coro,
			 ivm_context_t *context)
{
	IVM_TRACE("test module\n");

	return IVM_NULL;
}
