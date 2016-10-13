#ifndef _IVM_VM_INLINE_CORO_H_
#define _IVM_VM_INLINE_CORO_H_

#include "pub/com.h"
#include "pub/err.h"
#include "pub/vm.h"

#include "std/mem.h"

#include "vm/coro.h"
#include "vm/obj.h"

IVM_COM_HEADER

IVM_INLINE
void
ivm_coro_pushFrame_c(ivm_block_stack_t *bstack,
					 ivm_frame_stack_t *frame_st,
					 ivm_runtime_t *runtime)
{
	ivm_frame_stack_push(frame_st, runtime);
	IVM_RUNTIME_SET(runtime, BCUR, ivm_block_stack_getCur(bstack));

	return;
}

IVM_INLINE
ivm_object_t **
ivm_coro_popFrame_c(ivm_block_stack_t *bstack,
					ivm_frame_stack_t *frame_st,
					ivm_runtime_t *runtime)
{
	ivm_block_stack_setCur(bstack, IVM_RUNTIME_GET(runtime, BCUR));
	return ivm_frame_stack_pop(frame_st, runtime);
}

IVM_INLINE
void
ivm_coro_pushFrame(ivm_coro_t *coro)
{
	// IVM_FRAME_SET(ivm_frame_stack_push(&coro->frame_st, &coro->runtime), BCUR, bcur);
	ivm_frame_stack_push(&coro->frame_st, &coro->runtime);
	IVM_RUNTIME_SET(&coro->runtime, BCUR, ivm_block_stack_getCur(&coro->bstack));

	// IVM_TRACE("next bcur: %d\n", IVM_RUNTIME_GET(&coro->runtime, BCUR));

	return;
}

IVM_INLINE
ivm_object_t **
ivm_coro_popFrame(ivm_coro_t *coro)
{
	ivm_object_t **bp;

	// IVM_TRACE("prev bcur: %d\n", IVM_RUNTIME_GET(&coro->runtime, BCUR));

	ivm_block_stack_setCur(&coro->bstack, IVM_RUNTIME_GET(&coro->runtime, BCUR));
	bp = ivm_frame_stack_pop(&coro->frame_st, &coro->runtime);

	return bp;
}

IVM_COM_END

#endif
