#ifndef _IVM_VM_RUNTIME_H_
#define _IVM_VM_RUNTIME_H_

#include "pub/com.h"
#include "std/mem.h"
#include "pub/type.h"

#include "exec.h"
#include "func.h"
#include "context.h"
#include "call.h"
#include "instr.h"

IVM_COM_HEADER

struct ivm_coro_t_tag;
struct ivm_frame_t_tag;
struct ivm_vmstate_t_tag;
struct ivm_object_t_tag;

typedef struct ivm_runtime_t_tag {
	IVM_FRAME_HEADER
	struct ivm_object_t_tag **sp;
} ivm_runtime_t;

#define IVM_RUNTIME_GET_IS_NATIVE(runtime) (!(runtime)->ip)
#define IVM_RUNTIME_GET_CONTEXT(runtime) ((runtime)->ctx)
#define IVM_RUNTIME_GET_IP(runtime) ((runtime)->ip)
#define IVM_RUNTIME_GET_BP(runtime) ((runtime)->bp)
#define IVM_RUNTIME_GET_SP(runtime) ((runtime)->sp)
#define IVM_RUNTIME_GET_OFFSET(runtime) ((runtime)->offset)
#define IVM_RUNTIME_GET_NO_REG(runtime) ((runtime)->no_reg)

#define IVM_RUNTIME_GET_SP_INC(runtime) ((runtime)->sp++)
#define IVM_RUNTIME_GET_DEC_SP(runtime) (--(runtime)->sp)

#define IVM_RUNTIME_SET_IP(runtime, val) ((runtime)->ip = (val))
#define IVM_RUNTIME_SET_BP(runtime, val) ((runtime)->bp = (val))
#define IVM_RUNTIME_SET_SP(runtime, val) ((runtime)->sp = (val))
#define IVM_RUNTIME_SET_NO_REG(runtime, val) ((runtime)->no_reg = (val))

#define IVM_RUNTIME_GET(obj, member) IVM_GET((obj), IVM_RUNTIME, member)
#define IVM_RUNTIME_SET(obj, member, val) IVM_SET((obj), IVM_RUNTIME, member, (val))

ivm_context_t *
ivm_runtime_appendContextNode(ivm_runtime_t *runtime,
							  struct ivm_vmstate_t_tag *state);

ivm_context_t *
ivm_runtime_removeContextNode(ivm_runtime_t *runtime,
							  struct ivm_vmstate_t_tag *state);

ivm_runtime_t *
ivm_runtime_new(struct ivm_vmstate_t_tag *state);

#define ivm_runtime_free(runtime, state) \
	(STD_FREE(runtime))

/* rewrite the environment */
void
ivm_runtime_invokeNative(ivm_runtime_t *runtime,
						 struct ivm_vmstate_t_tag *state,
						 ivm_context_t *ctx);
void
ivm_runtime_invoke(ivm_runtime_t *runtime,
				   struct ivm_vmstate_t_tag *state,
				   const ivm_exec_t *exec,
				   ivm_context_t *ctx);

IVM_INLINE
void
ivm_runtime_dump(ivm_runtime_t *runtime,
				 struct ivm_vmstate_t_tag *state)
{
	ivm_context_free(runtime->ctx, state);
	if (runtime->nested_cat) {
		ivm_instr_stack_free(runtime->cat.astack);
	}

	return;
}

IVM_INLINE
ivm_bool_t
ivm_runtime_hasCatch(ivm_runtime_t *runtime)
{
	return ivm_frame_hasCatch(IVM_AS(runtime, ivm_frame_t));
}

IVM_INLINE
ivm_instr_t *
ivm_runtime_popCatch(ivm_runtime_t *runtime)
{
	return ivm_frame_popCatch(IVM_AS(runtime, ivm_frame_t));
}

IVM_INLINE
void
ivm_runtime_pushCatch(ivm_runtime_t *runtime, ivm_instr_t *cat)
{
	ivm_frame_pushCatch(IVM_AS(runtime, ivm_frame_t), cat);
	return;
}

typedef ivm_ptpool_t ivm_runtime_pool_t;

#define ivm_runtime_pool_new(count) (ivm_ptpool_new((count), sizeof(ivm_runtime_t)))
#define ivm_runtime_pool_free ivm_ptpool_free
#define ivm_runtime_pool_alloc(pool) ((ivm_runtime_t *)ivm_ptpool_alloc(pool))
#define ivm_runtime_pool_dump ivm_ptpool_dump

IVM_COM_END

#endif
