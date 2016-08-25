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
struct ivm_traverser_arg_t_tag;

typedef struct ivm_coro_t_tag {
	ivm_vmstack_t stack;
	ivm_frame_stack_t frame_st;
	ivm_runtime_t runtime;

	ivm_bool_t alive;
	ivm_bool_t has_native;
} ivm_coro_t;

typedef enum {
	IVM_CORO_ACTION_NONE = 0,
	IVM_CORO_ACTION_INVOKE,
	IVM_CORO_ACTION_RETURN,
	IVM_CORO_ACTION_YIELD
} ivm_coro_action_t;

#define IVM_CORO_GET_STACK(coro) (&(coro)->stack)
#define IVM_CORO_GET_FRAME_STACK(coro) (&(coro)->frame_st)
#define IVM_CORO_GET_RUNTIME(coro) (&(coro)->runtime)
#define IVM_CORO_GET_HAS_NATIVE(coro) ((coro)->has_native)

#define IVM_CORO_SET_HAS_NATIVE(coro, val) ((coro)->has_native = (val))

#define IVM_CORO_GET(obj, member) IVM_GET((obj), IVM_CORO, member)
#define IVM_CORO_SET(obj, member, val) IVM_SET((obj), IVM_CORO, member, (val))

ivm_coro_t *
ivm_coro_new(struct ivm_vmstate_t_tag *state);
void
ivm_coro_free(ivm_coro_t *coro,
			  struct ivm_vmstate_t_tag *state);

#define ivm_coro_stackTop(coro) (ivm_vmstack_size(&coro->stack))

ivm_object_t *
ivm_coro_newException_s(ivm_coro_t *coro,
						struct ivm_vmstate_t_tag *state,
						const ivm_char_t *msg);

#define IVM_CORO_NATIVE_ASSERT(coro, state, cond, ...) \
	if (!(cond)) {                                \
		char __rtm_assert_buf__[128];             \
		ivm_object_t *exc;                        \
		IVM_SNPRINTF(                             \
			__rtm_assert_buf__,                   \
			IVM_ARRLEN(__rtm_assert_buf__),       \
			__VA_ARGS__                           \
		);                                        \
		exc = ivm_coro_newException_s(            \
			(coro), (state), __rtm_assert_buf__   \
		);                                        \
		ivm_vmstate_setException((state), exc);   \
		return IVM_NULL;                          \
	}

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

#define ivm_coro_isAlive(coro) ((coro)->alive)

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
ivm_context_t *
ivm_coro_getRuntimeGlobal(ivm_coro_t *coro)
{
	ivm_context_t *ctx;

	if (coro->alive &&
		(ctx = IVM_RUNTIME_GET(&coro->runtime, CONTEXT))) {
		return ivm_context_getGlobal(ctx);
	}

	return IVM_NULL;
}

IVM_INLINE
ivm_context_t *
ivm_coro_getRuntimeLocal(ivm_coro_t *coro)
{
	if (coro->alive) {
		return IVM_RUNTIME_GET(&coro->runtime, CONTEXT);
	}

	return IVM_NULL;
}

typedef ivm_ptlist_t ivm_coro_list_t;
typedef IVM_PTLIST_ITER_TYPE(ivm_coro_t *) ivm_coro_list_iterator_t;

#define ivm_coro_list_init(list) (ivm_ptlist_init_c((list), IVM_DEFAULT_CORO_LIST_BUFFER_SIZE))
#define ivm_coro_list_dump ivm_ptlist_dump
#define ivm_coro_list_push ivm_ptlist_push
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

#if 0

typedef ivm_list_t ivm_cgroup_stack_t;
typedef IVM_LIST_ITER_TYPE(ivm_cgid_t) ivm_cgroup_stack_iterator_t;

#define ivm_cgroup_stack_init(stack) (ivm_list_init_c((stack), sizeof(ivm_cgid_t), IVM_DEFAULT_CGROUP_STACK_BUFFER_SIZE))
#define ivm_cgroup_stack_dump(stack) (ivm_list_dump(stack))
#define ivm_cgroup_stack_isEmpty ivm_list_isEmpty
#define ivm_cgroup_stack_empty ivm_list_empty
#define ivm_cgroup_stack_push(stack, gid) {   \
	ivm_cgid_t tmp = (gid);                   \
	ivm_list_push((stack), &tmp);             \
}

#define ivm_cgroup_stack_pop(stack) (*(ivm_cgid_t *)ivm_list_pop(stack))

#endif

typedef struct {
	ivm_coro_list_t coros;
	ivm_size_t cur;
	ivm_bool_t lock;
} ivm_cgroup_t;

IVM_INLINE
void
ivm_cgroup_init(ivm_cgroup_t *group)
{
	ivm_coro_list_init(&group->coros);
	group->cur = 0;
	group->lock = IVM_FALSE;

	return;
}

IVM_INLINE
void
ivm_cgroup_dump(ivm_cgroup_t *group,
				struct ivm_vmstate_t_tag *state)
{
	ivm_coro_list_iterator_t citer;

	if (group) {
		IVM_CORO_LIST_EACHPTR(&group->coros, citer) {
			ivm_coro_free(IVM_CORO_LIST_ITER_GET(citer), state);
		}
		ivm_coro_list_dump(&group->coros);
	}

	return;
}

#define ivm_cgroup_getCoroList(group) (&(group)->coros)
#define ivm_cgroup_isAlive(group) (ivm_coro_list_size(&(group)->coros) != 0)
#define ivm_cgroup_addCoro(group, coro) (ivm_coro_list_push(&(group)->coros, (coro)))

IVM_INLINE
ivm_coro_t *
ivm_cgroup_curCoro(ivm_cgroup_t *group)
{
	return ivm_coro_list_at(&group->coros, group->cur);
}

#define ivm_cgroup_isLocked(group) ((group)->lock)
#define ivm_cgroup_lock(group) ((group)->lock = IVM_TRUE)
#define ivm_cgroup_unlock(group) ((group)->lock = IVM_FALSE)

ivm_bool_t
ivm_cgroup_switchCoro(ivm_cgroup_t *group);

IVM_INLINE
void
ivm_cgroup_empty(ivm_cgroup_t *group,
				 struct ivm_vmstate_t_tag *state)
{
	ivm_coro_list_iterator_t citer;

	IVM_CORO_LIST_EACHPTR(&group->coros, citer) {
		ivm_coro_free(IVM_CORO_LIST_ITER_GET(citer), state);
	}
	ivm_coro_list_empty(&group->coros);

	return;
}

void
ivm_cgroup_travAndCompact(ivm_cgroup_t *group,
						  struct ivm_traverser_arg_t_tag *arg);

typedef ivm_list_t ivm_cgroup_list_t;
typedef IVM_LIST_ITER_TYPE(ivm_cgroup_t) ivm_cgroup_list_iterator_t;

#define ivm_cgroup_list_init(list) (ivm_list_init_c((list), sizeof(ivm_cgroup_t), IVM_DEFAULT_CGROUP_LIST_BUFFER_SIZE))
#define ivm_cgroup_list_dump ivm_list_dump
#define ivm_cgroup_list_size ivm_list_size
#define ivm_cgroup_list_prepush(list, ptr) (ivm_list_prepush((list), (void **)(ptr)))
#define ivm_cgroup_list_at(list, i) ((ivm_cgroup_t *)ivm_list_at((list), (i)))

#define IVM_CGROUP_LIST_ITER_GET_PTR(iter) (IVM_LIST_ITER_GET_PTR((iter), ivm_cgroup_t))
#define IVM_CGROUP_LIST_EACHPTR(list, iter) IVM_LIST_EACHPTR((list), iter, ivm_cgroup_t)

IVM_COM_END

#endif
