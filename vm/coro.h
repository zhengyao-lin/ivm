#ifndef _IVM_VM_CORO_H_
#define _IVM_VM_CORO_H_

#include "type.h"
#include "obj.h"
#include "vmstack.h"
#include "func.h"
#include "call.h"
#include "runtime.h"

struct ivm_vmstate_t_tag;

typedef struct ivm_coro_t_tag {
	ivm_vmstack_t *stack;
	ivm_call_stack_t *call_st;
	ivm_runtime_t *runtime;
} ivm_coro_t;

#define IVM_CORO_STACK(coro) ((coro)->stack)
#define IVM_CORO_CALL_STACK(coro) ((coro)->call_st)
#define IVM_CORO_RUNTIME(coro) ((coro)->runtime)

ivm_coro_t *
ivm_coro_new();
void
ivm_coro_free(ivm_coro_t *coro);

#define ivm_coro_stackTop(coro) (ivm_vmstack_size(coro->stack))
ivm_object_t *
ivm_coro_start(ivm_coro_t *coro, struct ivm_vmstate_t_tag *state, ivm_function_t *root);

typedef ivm_ptlist_t ivm_coro_list_t;

#define ivm_coro_list_new ivm_ptlist_new
#define ivm_coro_list_free ivm_ptlist_free
#define ivm_coro_list_add ivm_ptlist_push
#define ivm_coro_list_size ivm_ptlist_size
#define ivm_coro_list_at ivm_ptlist_at
#define ivm_coro_list_foreach ivm_ptlist_foreach
#define ivm_coro_list_foreach_arg ivm_ptlist_foreach_arg

#endif
