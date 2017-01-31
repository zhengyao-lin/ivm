#ifndef _IVM_VM_CORO_H_
#define _IVM_VM_CORO_H_

#include "pub/com.h"
#include "pub/const.h"
#include "pub/type.h"

#include "std/hash.h"
#include "std/list.h"
#include "std/thread.h"

#include "obj.h"
#include "vmstack.h"
#include "func.h"
#include "call.h"
#include "runtime.h"
#include "block.h"
#include "exc.h"

IVM_COM_HEADER

struct ivm_vmstate_t_tag;
struct ivm_traverser_arg_t_tag;

typedef struct ivm_coro_t_tag {
	ivm_vmstack_t stack;
	ivm_block_stack_t bstack;
	ivm_frame_stack_t frame_st;
	ivm_runtime_t runtime;

	ivm_param_list_t *param;
	ivm_object_t *exitv;

	ivm_long_t ref;
	ivm_int_t cid; // collect id
	ivm_bool_t alive;
	ivm_bool_t has_native;
	ivm_bool_t active;
	ivm_bool_t spawned;
	ivm_bool_t wb;
} ivm_coro_t;

typedef enum {
	IVM_CORO_ACTION_NONE = 0,
	IVM_CORO_ACTION_INVOKE,
	IVM_CORO_ACTION_RETURN,
	IVM_CORO_ACTION_EXCEPTION,
	IVM_CORO_ACTION_YIELD,
	IVM_CORO_ACTION_OP_ALT
} ivm_coro_action_t;

typedef enum {
	IVM_CORO_INT_NONE = 0,
	IVM_CORO_INT_GC,
#if IVM_USE_MULTITHREAD
	IVM_CORO_INT_THREAD_YIELD
#endif
} ivm_coro_int_t;

#define IVM_CORO_GET_STACK(coro) (&(coro)->stack)
#define IVM_CORO_GET_FRAME_STACK(coro) (&(coro)->frame_st)
#define IVM_CORO_GET_RUNTIME(coro) (&(coro)->runtime)
#define IVM_CORO_GET_HAS_NATIVE(coro) ((coro)->has_native)

#define IVM_CORO_SET_HAS_NATIVE(coro, val) ((coro)->has_native = (val))

#define IVM_CORO_GET(obj, member) IVM_GET((obj), IVM_CORO, member)
#define IVM_CORO_SET(obj, member, val) IVM_SET((obj), IVM_CORO, member, (val))

#define ivm_coro_addRef(coro) ((coro)->ref++)
#define ivm_coro_decRef(coro) (--(coro)->ref)

ivm_coro_t *
ivm_coro_new(struct ivm_vmstate_t_tag *state);
void
ivm_coro_free(ivm_coro_t *coro,
			  struct ivm_vmstate_t_tag *state);

#define ivm_coro_stackTop(coro) (ivm_vmstack_size(&coro->stack))

ivm_object_t *
ivm_coro_newStringException(ivm_coro_t *coro,
							struct ivm_vmstate_t_tag *state,
							const ivm_char_t *msg);

void
ivm_coro_printException(ivm_coro_t *coro,
						struct ivm_vmstate_t_tag *state,
						ivm_object_t *except);

// no return
#define IVM_CORO_NATIVE_FATAL_C(coro, state, ...) \
	{                                             \
		ivm_char_t msg_buf[                       \
			IVM_DEFAULT_EXCEPTION_BUFFER_SIZE     \
		];                                        \
		ivm_object_t *exc;                        \
		IVM_SNPRINTF(                             \
			msg_buf, IVM_ARRLEN(msg_buf),         \
			__VA_ARGS__                           \
		);                                        \
		exc = ivm_exception_new(                  \
			(state), msg_buf, __func__, 0         \
		);                                        \
		ivm_vmstate_setException((state), exc);   \
	}

#define IVM_CORO_NATIVE_FATAL(coro, state, ...) \
	IVM_CORO_NATIVE_FATAL_C((coro), (state), __VA_ARGS__);  \
	return IVM_NULL;                                        \

#define IVM_CORO_NATIVE_ASSERT(coro, state, cond, ...) \
	if (IVM_UNLIKELY(!(cond))) {                              \
		IVM_CORO_NATIVE_FATAL((coro), (state), __VA_ARGS__);  \
	}
	
ivm_object_t *
ivm_coro_execute_c(ivm_coro_t *coro,
				   struct ivm_vmstate_t_tag *state,
				   ivm_object_t *arg,
				   ivm_bool_t get_opcode_entry);

#if IVM_DISPATCH_METHOD_DIRECT_THREAD
	#define ivm_coro_getOpcodeEntry() \
		((void **)ivm_coro_execute_c(IVM_NULL, IVM_NULL, IVM_NULL, IVM_TRUE))
#endif

ivm_object_t *
ivm_coro_resume(ivm_coro_t *coro,
				struct ivm_vmstate_t_tag *state,
				ivm_object_t *arg);

// void
// ivm_coro_setInt(ivm_coro_int_t flag);

void
ivm_coro_setRoot(ivm_coro_t *coro,
				 struct ivm_vmstate_t_tag *state,
				 ivm_function_object_t *root);

#define ivm_coro_checkCID(coro, id) ((coro)->cid == (id))
#define ivm_coro_setCID(coro, id) ((coro)->cid = (id))

#define ivm_coro_isAlive(coro) ((coro)->alive)
#define ivm_coro_isActive(coro) ((coro)->active)
#define ivm_coro_isSpawned(coro) ((coro)->spawned)

#define ivm_coro_getWB(coro) ((coro)->wb)
#define ivm_coro_setWB(coro, val) ((coro)->wb = (val))

#define ivm_coro_setSpawned(coro) ((coro)->spawned = IVM_TRUE)
#define ivm_coro_unsetSpawned(coro) ((coro)->spawned = IVM_FALSE)

#define ivm_coro_setParam(coro, p) ((coro)->param = (p))

#define ivm_coro_getExitValue(coro) ((coro)->exitv)
#define _ivm_coro_setExitValue(coro, obj) ((coro)->exitv = (obj))

IVM_INLINE
ivm_bool_t
ivm_coro_canResume(ivm_coro_t *coro)
{
	return
		coro &&
		ivm_coro_isAlive(coro) &&
		!ivm_coro_isActive(coro) &&
		!ivm_coro_isSpawned(coro);
}

ivm_object_t *
ivm_coro_call_0(ivm_coro_t *coro,
				struct ivm_vmstate_t_tag *state,
				ivm_function_object_t *func);

ivm_object_t *
ivm_coro_call_1(ivm_coro_t *coro,
				struct ivm_vmstate_t_tag *state,
				ivm_function_object_t *func,
				ivm_object_t *arg);

ivm_object_t *
ivm_coro_callBase_0(ivm_coro_t *coro,
					struct ivm_vmstate_t_tag *state,
					ivm_function_object_t *func,
					ivm_object_t *base);

ivm_object_t *
ivm_coro_callBase_1(ivm_coro_t *coro,
					struct ivm_vmstate_t_tag *state,
					ivm_function_object_t *func,
					ivm_object_t *base,
					ivm_object_t *arg);

ivm_object_t *
ivm_coro_callBase_2(ivm_coro_t *coro,
					struct ivm_vmstate_t_tag *state,
					ivm_function_object_t *func,
					ivm_object_t *base,
					ivm_object_t *arg1, ivm_object_t *arg2);

IVM_INLINE
ivm_object_t **
ivm_coro_pushBlock(ivm_block_stack_t *bstack,
				   ivm_runtime_t *runtime,
				   ivm_size_t sp)
{
	ivm_block_stack_push(bstack, sp);
	return ivm_runtime_BPToSP(runtime);
}

IVM_INLINE
ivm_object_t **
ivm_coro_pushBlock_c(ivm_block_stack_t *bstack,
					 ivm_runtime_t *runtime,
					 ivm_size_t sp)
{
	ivm_block_stack_push(bstack, sp);
	return ivm_runtime_incBP(runtime, sp);
}

IVM_INLINE
ivm_bool_t
ivm_coro_popBlock(ivm_block_stack_t *bstack,
				  ivm_runtime_t *runtime)
{
	register ivm_size_t sp;

	if (ivm_block_stack_checkCur(bstack, IVM_RUNTIME_GET(runtime, BCUR)))
		return IVM_FALSE;
	
	sp = ivm_block_stack_pop(bstack);
	ivm_runtime_SPToBP(runtime);
	ivm_runtime_decBP(runtime, sp);

	return IVM_TRUE;
}

IVM_INLINE
void
ivm_coro_setCurCatch(ivm_block_stack_t *bstack,
					 ivm_runtime_t *runtime,
					 ivm_instr_t *catc)
{
	if (ivm_block_stack_checkCur(bstack, IVM_RUNTIME_GET(runtime, BCUR))) {
		IVM_FATAL("no block");
		return;
	}

	ivm_block_stack_setCurCatch(bstack, catc);
	return;
}

IVM_INLINE
ivm_instr_t *
ivm_coro_unsetCurCatch(ivm_block_stack_t *bstack,
					   ivm_runtime_t *runtime)
{
	if (ivm_block_stack_checkCur(bstack, IVM_RUNTIME_GET(runtime, BCUR)))
		return IVM_NULL;

	return ivm_block_stack_unsetCurCatch(bstack);
}

IVM_INLINE
ivm_instr_t *
ivm_coro_popToCatch(ivm_block_stack_t *bstack,
					ivm_runtime_t *runtime)
{
	ivm_instr_t *catc;

	do {
		catc = ivm_coro_unsetCurCatch(bstack, runtime);
		// IVM_TRACE("pop: %p\n", catc);
		if (catc) return catc;
	} while (ivm_coro_popBlock(bstack, runtime));

	return IVM_NULL;
}

// pop all blocks with raise protection
IVM_INLINE
void
ivm_coro_popAllCatch(ivm_block_stack_t *bstack,
					 ivm_runtime_t *runtime)
{
	do {
		if (!ivm_coro_unsetCurCatch(bstack, runtime)) return;
	} while (ivm_coro_popBlock(bstack, runtime));

	return;
}

IVM_INLINE
void
ivm_coro_popToFrame(ivm_block_stack_t *bstack,
					ivm_runtime_t *runtime)
{
	ivm_uint_t cur = ivm_block_stack_getCur(bstack);
	ivm_uint_t to = IVM_RUNTIME_GET(runtime, BCUR);

	IVM_ASSERT(cur >= to, "impossible");

	while (cur != to) {
		ivm_coro_popBlock(bstack, runtime);
		cur--;
	}

	return;
}

IVM_INLINE
ivm_object_t **
ivm_coro_getPrevBlockTop(ivm_block_stack_t *bstack,
						 ivm_object_t **cur_bp,
						 ivm_int_t n)
{
	return cur_bp - ivm_block_stack_getTopN(bstack, n - 1) - 1;
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

typedef ivm_ptpool_t ivm_coro_pool_t;

#define ivm_coro_pool_init(pool, count) (ivm_ptpool_init((pool), (count), sizeof(ivm_coro_t)))
#define ivm_coro_pool_destruct ivm_ptpool_destruct
#define ivm_coro_pool_alloc(pool) ((ivm_coro_t *)ivm_ptpool_alloc(pool))
#define ivm_coro_pool_dump ivm_ptpool_dump
#define ivm_coro_pool_dumpAll ivm_ptpool_dumpAll

typedef struct {
	IVM_OBJECT_HEADER
	ivm_coro_t *coro;
} ivm_coro_object_t;

ivm_object_t *
ivm_coro_object_new(struct ivm_vmstate_t_tag *state,
					ivm_coro_t *coro);

#define ivm_coro_object_getCoro(obj) (IVM_AS((obj), ivm_coro_object_t)->coro)

void
ivm_coro_object_destructor(ivm_object_t *obj,
						   struct ivm_vmstate_t_tag *state);

void
ivm_coro_object_cloner(ivm_object_t *obj,
					   struct ivm_vmstate_t_tag *state);

void
ivm_coro_object_traverser(ivm_object_t *obj,
						  struct ivm_traverser_arg_t_tag *arg);

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

typedef ivm_pthash_t ivm_coro_set_t;
typedef ivm_pthash_iterator_t ivm_coro_set_iterator_t;

#define ivm_coro_set_init(set) ivm_pthash_init(set)
#define ivm_coro_set_insert(set, coro) ivm_pthash_insertEmpty((set), (void *)(coro), IVM_NULL)
#define ivm_coro_set_remove(set, coro) ivm_pthash_remove((set), (void *)(coro))
#define ivm_coro_set_size(set) ivm_pthash_count(set)

#define ivm_coro_set_has(set, coro) ivm_pthash_find((set), (void *)(coro))

#define IVM_CORO_SET_ITER_GET(iter) ((ivm_coro_t *)IVM_PTHASH_ITER_GET_KEY(iter))
#define IVM_CORO_SET_EACHPTR(set, iter) IVM_PTHASH_EACHPTR((set), iter)

#define ivm_coro_set_dump(set) ivm_pthash_dump(set)

#if IVM_USE_MULTITHREAD

typedef struct {
	ivm_coro_t *coro;
	ivm_thread_t tid;
} ivm_coro_thread_t;

ivm_coro_thread_t *
ivm_coro_thread_new(struct ivm_vmstate_t_tag *state,
					ivm_coro_t *coro);

void
ivm_coro_thread_free(ivm_coro_thread_t *thread,
					 struct ivm_vmstate_t_tag *state);

#define ivm_coro_thread_getTID(thread) (&(thread)->tid)
#define ivm_coro_thread_getCoro(thread) ((thread)->coro)

typedef ivm_ptpool_t ivm_cthread_pool_t;

#define ivm_cthread_pool_init(pool, count) (ivm_ptpool_init((pool), (count), sizeof(ivm_coro_thread_t)))
#define ivm_cthread_pool_destruct ivm_ptpool_destruct
#define ivm_cthread_pool_alloc(pool) ((ivm_coro_thread_t *)ivm_ptpool_alloc(pool))
#define ivm_cthread_pool_dump ivm_ptpool_dump

typedef ivm_pthash_t ivm_cthread_set_t;
typedef ivm_pthash_iterator_t ivm_cthread_set_iterator_t;

#define ivm_cthread_set_init(set) ivm_pthash_init(set)
#define ivm_cthread_set_insert(set, thread) ivm_pthash_insertEmpty((set), (void *)(thread), IVM_NULL)
#define ivm_cthread_set_remove(set, thread) ivm_pthash_remove((set), (void *)(thread))
#define ivm_cthread_set_size(set) ivm_pthash_count(set)

#define ivm_cthread_set_has(set, thread) ivm_pthash_find((set), (void *)(thread))

#define IVM_CTHREAD_SET_ITER_GET(iter) ((ivm_coro_thread_t *)IVM_PTHASH_ITER_GET_KEY(iter))
#define IVM_CTHREAD_SET_EACHPTR(set, iter) IVM_PTHASH_EACHPTR((set), iter)

#define ivm_cthread_set_dump(set) ivm_pthash_dump(set)

#endif

IVM_COM_END

#endif
