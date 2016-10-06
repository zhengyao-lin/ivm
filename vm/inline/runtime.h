#ifndef _IVM_VM_INLINE_RUNTIME_H_
#define _IVM_VM_INLINE_RUNTIME_H_

#include "pub/com.h"
#include "pub/err.h"
#include "pub/vm.h"

#include "std/mem.h"

#include "vm/call.h"
#include "vm/runtime.h"
#include "vm/obj.h"

IVM_COM_HEADER

IVM_INLINE
void
ivm_runtime_invokeNative(ivm_runtime_t *runtime,
						 ivm_vmstate_t *state,
						 ivm_context_t *ctx)
{
	runtime->ctx = ivm_context_addRef(ctx);
	runtime->bp = runtime->sp;
	runtime->ip = IVM_NULL;

	IVM_FRAME_INIT_HEADER(runtime);

	return;
}

IVM_INLINE
ivm_instr_t *
ivm_runtime_invoke(ivm_runtime_t *runtime,
				   ivm_vmstate_t *state,
				   const ivm_exec_t *exec,
				   ivm_context_t *ctx)
{
	runtime->ctx = ivm_context_addRef(ctx);
	runtime->bp = runtime->sp;

	IVM_FRAME_INIT_HEADER(runtime);

	runtime->offset = ivm_exec_offset(exec);

	return runtime->ip = ivm_exec_instrPtrStart(exec);
}

IVM_INLINE
ivm_object_t ** /* new_bp */
ivm_runtime_pushBlock(ivm_runtime_t *runtime,
					  ivm_vmstate_t *state,
					  ivm_size_t sp)
{
	return ivm_frame_pushBlock(IVM_AS(runtime, ivm_frame_t), state, sp);
}

IVM_INLINE
void
ivm_runtime_dump(ivm_runtime_t *runtime,
				 ivm_vmstate_t *state)
{
	ivm_context_free(runtime->ctx, state);
	ivm_vmstate_dumpBlock(state, runtime->blocks, runtime->block_alloc);
	// STD_FREE(runtime->blocks);
	return;
}

IVM_COM_END

#endif
