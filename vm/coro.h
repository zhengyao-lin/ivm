#ifndef _IVM_VM_CORO_H_
#define _IVM_VM_CORO_H_

#include "pub/com.h"
#include "pub/const.h"
#include "pub/type.h"

#include "obj.h"
#include "vmstack.h"
#include "func.h"
#include "call.h"
#include "runtime.h"

IVM_COM_HEADER

struct ivm_vmstate_t_tag;

typedef struct ivm_coro_t_tag {
	ivm_vmstack_t stack;
	ivm_frame_stack_t frame_st;
	ivm_runtime_t *runtime;
} ivm_coro_t;

typedef enum {
	IVM_CORO_ACTION_NONE = 0,
	IVM_CORO_ACTION_INVOKE,
	IVM_CORO_ACTION_RETURN,
	IVM_CORO_ACTION_YIELD
} ivm_coro_action_t;

#define IVM_CORO_GET_STACK(coro) (&(coro)->stack)
#define IVM_CORO_GET_FRAME_STACK(coro) (&(coro)->frame_st)
#define IVM_CORO_GET_RUNTIME(coro) ((coro)->runtime)

#define IVM_CORO_GET(obj, member) IVM_GET((obj), IVM_CORO, member)
#define IVM_CORO_SET(obj, member, val) IVM_SET((obj), IVM_CORO, member, (val))

ivm_coro_t *
ivm_coro_new(struct ivm_vmstate_t_tag *state);
void
ivm_coro_free(ivm_coro_t *coro,
			  struct ivm_vmstate_t_tag *state);

#define ivm_coro_stackTop(coro) (ivm_vmstack_size(&coro->stack))

ivm_object_t *
ivm_coro_start_c(ivm_coro_t *coro,
				 struct ivm_vmstate_t_tag *state,
				 ivm_function_object_t *root,
				 ivm_bool_t get_opcode_entry);

#define ivm_coro_start(coro, state, root) \
	(ivm_coro_start_c((coro), (state), (root), IVM_FALSE))

#if IVM_DISPATCH_METHOD_DIRECT_THREAD
	#define ivm_coro_getOpcodeEntry() \
		((void **)ivm_coro_start_c(IVM_NULL, IVM_NULL, IVM_NULL, IVM_TRUE))
#endif

void
ivm_coro_setRoot(ivm_coro_t *coro,
				 struct ivm_vmstate_t_tag *state,
				 ivm_function_object_t *root);

#define ivm_coro_isAsleep(coro) ((coro)->runtime != IVM_NULL)

IVM_INLINE
ivm_object_t *
ivm_coro_resume(ivm_coro_t *coro,
				struct ivm_vmstate_t_tag *state,
				ivm_object_t *init)
{
	if (init) {
		ivm_vmstack_push(coro, init);
	}

	return ivm_coro_start(coro, state, IVM_NULL);
}

IVM_INLINE
ivm_object_t *
ivm_coro_getRuntimeGlobal(ivm_coro_t *coro)
{
	ivm_ctchain_t *chain;

	if (coro->runtime &&
		(chain = IVM_RUNTIME_GET(coro->runtime, CONTEXT))) {
		return ivm_context_toObject(ivm_ctchain_getGlobal(chain));
	}

	return IVM_NULL;
}

typedef ivm_ptlist_t ivm_coro_list_t;
typedef IVM_PTLIST_ITER_TYPE(ivm_coro_t *) ivm_coro_list_iterator_t;

#define ivm_coro_list_init(list) (ivm_ptlist_init_c((list), IVM_DEFAULT_CORO_LIST_BUFFER_SIZE))
#define ivm_coro_list_dump ivm_ptlist_dump
#define ivm_coro_list_add ivm_ptlist_push
#define ivm_coro_list_setSize ivm_ptlist_setCur
#define ivm_coro_list_size ivm_ptlist_size
#define ivm_coro_list_empty ivm_ptlist_empty
#define ivm_coro_list_at(list, i) ((ivm_coro_t *)ivm_ptlist_at((list), (i)))

#define IVM_CORO_LIST_ITER_BEGIN(list) ((ivm_coro_list_iterator_t)IVM_PTLIST_ITER_BEGIN(list))
#define IVM_CORO_LIST_ITER_END(list) ((ivm_coro_list_iterator_t)IVM_PTLIST_ITER_END(list))
#define IVM_CORO_LIST_ITER_AT(list, i) ((ivm_coro_list_iterator_t)IVM_PTLIST_ITER_AT((list), (i)))
#define IVM_CORO_LIST_ITER_INDEX IVM_PTLIST_ITER_INDEX
#define IVM_CORO_LIST_ITER_SET(iter, val) (IVM_PTLIST_ITER_SET((iter), (val)))
#define IVM_CORO_LIST_ITER_GET(iter) ((ivm_coro_t *)IVM_PTLIST_ITER_GET(iter))
#define IVM_CORO_LIST_EACHPTR(list, iter) IVM_PTLIST_EACHPTR((list), iter, ivm_coro_t *)

typedef ivm_ptpool_t ivm_coro_pool_t;

#define ivm_coro_pool_init(pool, count) (ivm_ptpool_init((pool), (count), sizeof(ivm_coro_t)))
#define ivm_coro_pool_destruct ivm_ptpool_destruct
#define ivm_coro_pool_alloc(pool) ((ivm_coro_t *)ivm_ptpool_alloc(pool))
#define ivm_coro_pool_dump ivm_ptpool_dump
#define ivm_coro_pool_dumpAll ivm_ptpool_dumpAll

IVM_COM_END

#endif
