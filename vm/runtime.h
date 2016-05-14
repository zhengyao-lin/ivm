#ifndef _IVM_VM_RUNTIME_H_
#define _IVM_VM_RUNTIME_H_

#include "pub/mem.h"
#include "type.h"
#include "exec.h"
#include "func.h"
#include "context.h"
#include "call.h"

struct ivm_coro_t_tag;
struct ivm_caller_info_t_tag;

typedef struct ivm_runtime_t_tag {
	IVM_EXEC_INFO_HEAD
} ivm_runtime_t;

#define IVM_RUNTIME_GET_IS_NATIVE(runtime) ((runtime)->exec == IVM_NULL)
#define IVM_RUNTIME_GET_PC(runtime) ((runtime)->pc)
#define IVM_RUNTIME_GET_EXEC(runtime) ((runtime)->exec)
#define IVM_RUNTIME_GET_CONTEXT(runtime) ((runtime)->context)

#define IVM_RUNTIME_GET(obj, member) IVM_GET((obj), IVM_RUNTIME, member)
#define IVM_RUNTIME_SET(obj, member, val) IVM_SET((obj), IVM_RUNTIME, member, (val))

ivm_runtime_t *
ivm_runtime_new();

void
ivm_runtime_free(ivm_runtime_t *runtime);

/* just rewrite new environment */
/* NOTICE: this function will copy the context chain */
void
ivm_runtime_invoke(ivm_runtime_t *runtime,
				   ivm_exec_t *exec,
				   ivm_ctchain_t *context);

/* pack current state to caller info */
ivm_caller_info_t *
ivm_runtime_getCallerInfo(ivm_runtime_t *runtime,
						  struct ivm_coro_t_tag *coro);

/* restore caller info */
void
ivm_runtime_restore(ivm_runtime_t *runtime, struct ivm_coro_t_tag *coro, struct ivm_caller_info_t_tag *info);

#endif
