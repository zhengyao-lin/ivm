#ifndef _IVM_VM_RUNTIME_H_
#define _IVM_VM_RUNTIME_H_

#include "pub/com.h"
#include "pub/mem.h"
#include "pub/type.h"

#include "exec.h"
#include "func.h"
#include "context.h"
#include "call.h"

IVM_COM_HEADER

struct ivm_coro_t_tag;
struct ivm_frame_t_tag;
struct ivm_vmstate_t_tag;
struct ivm_object_t_tag;

typedef struct ivm_runtime_t_tag {
	IVM_FRAME_HEADER
	struct ivm_object_t_tag **sp;
} IVM_NOALIGN ivm_runtime_t;

#define IVM_RUNTIME_GET_IS_NATIVE(runtime) (!(runtime)->ip)
#define IVM_RUNTIME_GET_CONTEXT(runtime) ((runtime)->context)
#define IVM_RUNTIME_GET_IP(runtime) ((runtime)->ip)
#define IVM_RUNTIME_GET_BP(runtime) ((runtime)->bp)
#define IVM_RUNTIME_GET_SP(runtime) ((runtime)->sp)

#define IVM_RUNTIME_GET_SP_INC(runtime) ((runtime)->sp++)
#define IVM_RUNTIME_GET_DEC_SP(runtime) (--(runtime)->sp)

#define IVM_RUNTIME_GET_CONTEXT_PTR(runtime) (&(runtime)->context)
#define IVM_RUNTIME_GET_IP_PTR(runtime) (&(runtime)->IP)

#define IVM_RUNTIME_SET_IP(runtime, val) ((runtime)->ip = (val))
#define IVM_RUNTIME_SET_BP(runtime, val) ((runtime)->bp = (val))
#define IVM_RUNTIME_SET_SP(runtime, val) ((runtime)->sp = (val))

#define IVM_RUNTIME_GET(obj, member) IVM_GET((obj), IVM_RUNTIME, member)
#define IVM_RUNTIME_SET(obj, member, val) IVM_SET((obj), IVM_RUNTIME, member, (val))

ivm_runtime_t *
ivm_runtime_new(struct ivm_vmstate_t_tag *state);

#define ivm_runtime_free(runtime, state) \
	(MEM_FREE(runtime))

/* just rewrite new environment */
/* NOTICE: this function will NOT copy the context chain,
 *		   clone the context chain by yourself
 */
void
ivm_runtime_invoke(ivm_runtime_t *runtime,
				   struct ivm_vmstate_t_tag *state,
				   const ivm_exec_t *exec,
				   ivm_ctchain_t *context);

#define ivm_runtime_dump(runtime, state) \
	(ivm_ctchain_free((runtime)->context, (state)))

typedef ivm_ptpool_t ivm_runtime_pool_t;

#define ivm_runtime_pool_new(count) (ivm_ptpool_new((count), sizeof(ivm_runtime_t)))
#define ivm_runtime_pool_free ivm_ptpool_free
#define ivm_runtime_pool_alloc(pool) ((ivm_runtime_t *)ivm_ptpool_alloc(pool))
#define ivm_runtime_pool_dump ivm_ptpool_dump

IVM_COM_END

#endif
