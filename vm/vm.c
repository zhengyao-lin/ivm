#include "pub/mem.h"
#include "vm.h"

ivm_vmstate_t *
ivm_new_state()
{
	ivm_vmstate_t *ret = MEM_ALLOC_INIT(sizeof(*ret));
	return ret;
}

void
ivm_free_state(ivm_vmstate_t *state)
{
	MEM_FREE(state);
	return;
}
