#include "pub/mem.h"
#include "vm.h"
#include "err.h"
#include "gc/heap.h"

ivm_vmstate_t *
ivm_new_state()
{
	ivm_vmstate_t *ret = MEM_ALLOC_INIT(sizeof(*ret));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("vm state"));
	
	ret->heap = ivm_new_heap(DEFAULT_INIT_HEAP_SIZE);
	
	return ret;
}

void
ivm_free_state(ivm_vmstate_t *state)
{
	if (state) {
		ivm_free_heap(state->heap);
		MEM_FREE(state);
	}

	return;
}
