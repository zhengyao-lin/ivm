#include "pub/mem.h"
#include "call.h"
#include "context.h"
#include "runtime.h"
#include "vm.h"
#include "err.h"

ivm_frame_t *
ivm_frame_new(ivm_vmstate_t *state,
			  ivm_runtime_t *runtime,
			  ivm_size_t st_top)
{
	ivm_frame_t *ret = ivm_vmstate_allocFrame(state);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("frame"));

	MEM_COPY(ret, runtime, sizeof(*runtime));
	ret->st_top = st_top;

	return ret;
}

void
ivm_frame_free(ivm_frame_t *frame, ivm_vmstate_t *state)
{
	ivm_vmstate_dumpFrame(state, frame);
	return;
}
