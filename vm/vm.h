#ifndef _IVM_VM_VM_H_
#define _IVM_VM_VM_H_

#include "pub/com.h"
#include "pub/const.h"
#include "pub/type.h"
#include "pub/err.h"
#include "pub/obj.h"

#include "std/mem.h"
#include "std/pool.h"
#include "std/string.h"
#include "std/heap.h"
#include "std/uid.h"
#include "std/thread.h"
#include "std/time.h"
#include "std/io.h"

#include "gc.h"
#include "coro.h"
#include "context.h"

IVM_COM_HEADER

#define _INT_BUF_SIZE IVM_DEFAULT_CORO_INT_BUFFER_SIZE

#if _INT_BUF_SIZE != 2 && \
	_INT_BUF_SIZE != 4 && \
	_INT_BUF_SIZE != 8 && \
	_INT_BUF_SIZE != 16 && \
	_INT_BUF_SIZE != 32 && \
	_INT_BUF_SIZE != 64 && \
	_INT_BUF_SIZE != 128
	#error unsupported coroutine interrupt buffer size
#endif

typedef struct ivm_vmstate_t_tag {
	ivm_collector_t gc;
	ivm_coro_int_t int_buf[_INT_BUF_SIZE];

	ivm_heap_t heaps[3];						// 120

	ivm_context_pool_t ct_pool;
	ivm_coro_pool_t cr_pool;

#if IVM_USE_BLOCK_POOL
	ivm_block_pool_t block_pool;
#endif

	ivm_func_list_t func_list;					// 24

	ivm_object_t *except;
	ivm_object_t *loaded_mod;
	ivm_object_t *obj_none;
	ivm_coro_set_t coro_set;

	ivm_type_pool_t type_pool;

#if IVM_USE_MULTITHREAD
	
	ivm_cthread_pool_t thread_pool;
	ivm_cthread_set_t thread_set;

	ivm_thread_mutex_t thread_gil;
	// clock sync lock
	ivm_thread_mutex_t thread_csl_prot;
	ivm_bool_t thread_csl;
	ivm_thread_t thread_clock;

#endif

	ivm_coro_t *main_coro;

	ivm_bool_t thread_enabled;

	ivm_uint_t int_next;
	ivm_uint_t int_head;

	ivm_bool_t has_int;

#if IVM_USE_MULTITHREAD
	ivm_thread_mutex_t int_lock;
#endif

	ivm_function_pool_t *func_pool;				// 8
	ivm_string_pool_t *const_pool;				// 8

#define CONST_GEN(name, str) const ivm_string_t *const_str_##name;
	#include "vm.const.h"						// 8
#undef CONST_GEN

	// const ivm_string_t **const_oop_symb;
	ivm_string_pool_t *oop_map;

	const ivm_string_t *cur_path;

	ivm_size_t wild_size;

	// ivm_cgid_t cur_cgroup;
	ivm_int_t gc_flag; /* gc flag:				// 4
						  > 0: open
						  = 0: closed
						  < 0: locked */

	ivm_uid_gen_t st_uid_gen; // slot table uid generator

	ivm_bool_t join_start;

	ivm_type_t type_list[IVM_TYPE_COUNT];
} ivm_vmstate_t;

#define IVM_VMSTATE_GET_TYPE_LIST(state) ((state)->type_list)
#define IVM_VMSTATE_GET_CONST_POOL(state) ((state)->const_pool)
#define IVM_VMSTATE_GET_CUR_HEAP(state) ((state)->heaps)
#define IVM_VMSTATE_GET_THREAD_SET(state) (&(state)->thread_set)

#define IVM_VMSTATE_GET(obj, member) IVM_GET((obj), IVM_VMSTATE, member)
#define IVM_VMSTATE_SET(obj, member, val) IVM_SET((obj), IVM_VMSTATE, member, (val))

#define IVM_VMSTATE_CONST(state, name) ((state)->const_str_##name)

#define IVM_CSTR(state, str) ((const ivm_string_t *)ivm_string_pool_registerRaw((state)->const_pool, (str)))

#define IVM_BTTYPE(state, type) ((state)->type_list + (type))

// is builtin type
#define IVM_IS_BTTYPE(obj, state, type) (IVM_TYPE_OF(obj) == IVM_BTTYPE((state), (type)))

#define IVM_TPTYPE(state, key) ivm_type_pool_get(&(state)->type_pool, (key))

#define IVM_VMSTATE_REGISTER_TPTYPE(state, coro, init, proto, ...) \
	{                                                                                       \
		ivm_type_t *_TYPE =                                                                 \
		ivm_type_pool_register(&(state)->type_pool, ivm_type_getUID(init), (init));         \
		ivm_object_t *_PROTO;                                                               \
                                                                                            \
		RTM_ASSERT_C(                                                                       \
			(coro), (state), _TYPE,                                                         \
			IVM_ERROR_MSG_REDEF_TP_TYPE(ivm_type_getName(init))                             \
		);                                                                                  \
		                                                                                    \
		_PROTO = (proto);                                                                   \
		ivm_type_setProto(_TYPE, _PROTO);                                                   \
		ivm_object_setProto(_PROTO, state, ivm_vmstate_getTypeProto(state, IVM_OBJECT_T));  \
		                                                                                    \
		__VA_ARGS__;                                                                        \
	}

ivm_vmstate_t *
ivm_vmstate_new(ivm_string_pool_t *const_pool);

void
ivm_vmstate_free(ivm_vmstate_t *state);

IVM_INLINE
const ivm_string_t *
ivm_vmstate_constantize(ivm_vmstate_t *state,
						const ivm_string_t *str)
{
	if (IVM_UNLIKELY(!ivm_string_isConst(str))) {
		return ivm_string_pool_register(state->const_pool, str);
	}

	return str;
}

IVM_INLINE
const ivm_string_t *
ivm_vmstate_constantize_r(ivm_vmstate_t *state,
						  const ivm_char_t *str)
{
	return ivm_string_pool_registerRaw(state->const_pool, str);
}

#define ivm_vmstate_getCurPath(state) ((state)->cur_path)
#define ivm_vmstate_getCurPath_r(state) ivm_string_trimHead((state)->cur_path)

IVM_INLINE
void
ivm_vmstate_setCurPath(ivm_vmstate_t *state,
					   const ivm_char_t *path)
{
	ivm_file_setRelativePath(path);
	state->cur_path = ivm_vmstate_constantize_r(state, path);
	return;
}

IVM_INLINE
void
ivm_vmstate_setCurPath_c(ivm_vmstate_t *state,
						 const ivm_string_t *path)
{
	ivm_file_setRelativePath(ivm_string_trimHead(path));
	state->cur_path = path;
	return;
}

#define ivm_vmstate_getOopSymbol(state, oop) ivm_string_pool_get((state)->oop_map, (oop))
#define ivm_vmstate_isOopSymbol(state, str) ivm_string_pool_find((state)->oop_map, (str))

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

#define ivm_vmstate_hasThread(state) ((state)->thread_enabled)

#define _INT_ROUND_MASK (_INT_BUF_SIZE - 1)

#define _INC_ROUND(n) ((n) = ((n) + 1) & _INT_ROUND_MASK)
#define _DEC_ROUND(n) ((n) = ((n) - 1) & _INT_ROUND_MASK)

#if IVM_USE_MULTITHREAD
	#define _INT_LOCK() if (state->thread_enabled) ivm_thread_mutex_lock(&state->int_lock)
	#define _INT_UNLOCK() if (state->thread_enabled) ivm_thread_mutex_unlock(&state->int_lock)
#else
	#define _INT_LOCK()
	#define _INT_UNLOCK()
#endif

IVM_INLINE
void
ivm_vmstate_setInt(ivm_vmstate_t *state,
				   ivm_coro_int_t flag)
{
	_INT_LOCK();

	state->int_buf[state->int_next] = flag;

	_INC_ROUND(state->int_next);

	if (state->int_next == state->int_head) {
		_INC_ROUND(state->int_head);
	}

	state->has_int = IVM_TRUE;

	_INT_UNLOCK();
	
	return;
}

IVM_INLINE
ivm_bool_t
ivm_vmstate_hasInt(ivm_vmstate_t *state)
{
	return state->has_int;
}

IVM_INLINE
ivm_coro_int_t
ivm_vmstate_popInt(ivm_vmstate_t *state)
{
	/* assume _coro_int_next != _coro_int_head */
	register ivm_coro_int_t ret;

	_INT_LOCK();
	
	if (state->int_next == state->int_head) return IVM_CORO_INT_NONE;

	_DEC_ROUND(state->int_next);

	ret = state->int_buf[state->int_next];

	if (state->int_next == state->int_head) {
		/* empty -> reset to zeros */
		state->int_head = state->int_next = 0;
		state->has_int = IVM_FALSE;
	}

	_INT_UNLOCK();

	return ret;
}

void
ivm_vmstate_enableThread(ivm_vmstate_t *state);

// void
// ivm_vmstate_cleanThread(ivm_vmstate_t *state);

IVM_INLINE
void
ivm_vmstate_lockGIL(ivm_vmstate_t *state)
{
#if IVM_USE_MULTITHREAD
	ivm_thread_mutex_lock(&state->thread_gil);
#endif
	// IVM_TRACE("GIL locked!\n");
	return;
}

IVM_INLINE
void
ivm_vmstate_unlockGIL(ivm_vmstate_t *state)
{
	// IVM_TRACE("GIL unlocked!\n");
#if IVM_USE_MULTITHREAD
	ivm_thread_mutex_unlock(&state->thread_gil);
#endif
	return;
}

#if IVM_USE_MULTITHREAD
	#define _CSL_LOCK() ivm_thread_mutex_lock(&state->thread_csl_prot)
	#define _CSL_UNLOCK() ivm_thread_mutex_unlock(&state->thread_csl_prot)
#else
	#define _CSL_LOCK()
	#define _CSL_UNLOCK()
#endif

IVM_INLINE
void
ivm_vmstate_setCSL(ivm_vmstate_t *state)
{
#if IVM_USE_MULTITHREAD
	_CSL_LOCK();
	state->thread_csl = IVM_TRUE;
	_CSL_UNLOCK();
#endif

	// IVM_TRACE("set CSL! %d\n", state->thread_csl);

	return;
}

IVM_INLINE
void
ivm_vmstate_unsetCSL(ivm_vmstate_t *state)
{
#if IVM_USE_MULTITHREAD
	_CSL_LOCK();
	state->thread_csl = IVM_FALSE;
	_CSL_UNLOCK();
#endif

	// IVM_TRACE("unset CSL! %d\n", state->thread_csl);

	return;
}

IVM_INLINE
ivm_bool_t
ivm_vmstate_getCSL(ivm_vmstate_t *state)
{
#if IVM_USE_MULTITHREAD
	return state->thread_csl;
#else
	return IVM_FALSE;
#endif
}

#undef _CSL_LOCK
#undef _CSL_UNLOCK

#undef _INT_BUF_SIZE
#undef _INT_ROUND_MASK
#undef _INC_ROUND
#undef _DEC_ROUND

#undef _INT_LOCK
#undef _INT_UNLOCK

IVM_INLINE
void
ivm_vmstate_threadStart(ivm_vmstate_t *state)
{
	ivm_vmstate_lockGIL(state);
	return;
}

IVM_INLINE
void
ivm_vmstate_threadEnd(ivm_vmstate_t *state)
{
	ivm_vmstate_unlockGIL(state);

	// IVM_TRACE("unlock!\n");
	// ivm_vmstate_setCSL(state);

	return;
}

#if IVM_USE_MULTITHREAD

#define ivm_vmstate_allocCThread(state) ivm_cthread_pool_alloc(&(state)->thread_pool)
#define ivm_vmstate_dumpCThread(state, thread) ivm_cthread_pool_dump(&(state)->thread_pool, (thread))

void
ivm_vmstate_threadYield(ivm_vmstate_t *state);

#endif

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

#define ivm_vmstate_isGCFlagOpen(state) ((state)->gc_flag == 1)

IVM_INLINE
void
ivm_vmstate_openGCFlag(ivm_vmstate_t *state)
{
	if (state->gc_flag >= 0) {
		state->gc_flag = 1;
		// ivm_vmstate_interrupt(state);
		ivm_vmstate_setInt(state, IVM_CORO_INT_GC);
	}

	return;
}

IVM_INLINE
void
ivm_vmstate_closeGCFlag(ivm_vmstate_t *state)
{
	if (state->gc_flag >= 0)
		state->gc_flag = 0;

	return;
}

#define ivm_vmstate_lockGCFlag(state) ((state)->gc_flag = -1)
#define ivm_vmstate_unlockGCFlag(state) ((state)->gc_flag = 0)

#define ivm_vmstate_getHeapAt(state, i) ((state)->heaps + (i))
#define ivm_vmstate_getHeaps(state) ((state)->heaps)

IVM_INLINE
void
IVM_WBCORO(ivm_vmstate_t *state,
		   ivm_coro_t *coro)
{
	// IVM_TRACE("write barrier! %p %d\n", coro, coro ? coro->wb : -1);
	if (coro && !ivm_coro_getWB(coro)) {
		ivm_collector_addWBCoro(&state->gc, coro);
		ivm_coro_setWB(coro, IVM_TRUE);
	}

	return;
}

IVM_INLINE
ivm_bool_t
IVM_WBOBJ(ivm_vmstate_t *state,
		  ivm_object_t *parent,
		  ivm_object_t *child)
{
	if (IVM_OBJECT_GET(parent, GEN) &&
		!IVM_OBJECT_GET(child, GEN)) {
		if (!IVM_OBJECT_GET(parent, WB)) {
			ivm_collector_addWBObj(&state->gc, parent);
			IVM_OBJECT_SET(parent, WB, 1);
		}
		return IVM_TRUE;
	}

	return IVM_FALSE;
}

IVM_INLINE
ivm_bool_t
IVM_WBOBJ_SLOT(ivm_vmstate_t *state,
			   ivm_object_t *obj,
			   ivm_slot_table_t *table)
{
	if (obj && IVM_OBJECT_GET(obj, GEN) &&
		!ivm_slot_table_getGen(table)) {
		if (!IVM_OBJECT_GET(obj, WB)) {
			ivm_collector_addWBObj(&state->gc, obj);
			IVM_OBJECT_SET(obj, WB, 1);
		}
		return IVM_TRUE;
	}

	return IVM_FALSE;
}

IVM_INLINE
ivm_bool_t
IVM_WBSLOT(ivm_vmstate_t *state,
		   ivm_slot_table_t *table,
		   ivm_object_t *value)
{
	// IVM_TRACE("wb %d %d\n", ivm_slot_table_getGen(table), IVM_OBJECT_GET(value, GEN));
	if (ivm_slot_table_getGen(table) && value &&
		!IVM_OBJECT_GET(value, GEN)) {
		if (!ivm_slot_table_getWB(table)) {
			// IVM_TRACE("write %p %p(%s)\n", table, value, IVM_OBJECT_GET(value, TYPE_NAME));
			ivm_collector_addWBSlotTable(&state->gc, table);
			ivm_slot_table_setWB(table, 1);
		}
		return IVM_TRUE;
	}

	return IVM_FALSE;
}

/*
IVM_INLINE
ivm_bool_t
IVM_WBCTX(ivm_vmstate_t *state,
		  ivm_context_t *ctx,
		  ivm_slot_table_t *value)
{
	if (ivm_context_getGen(ctx) && value &&
		!ivm_slot_table_getGen(value)) {
		if (!ivm_context_getWB(ctx)) {
			ivm_collector_addWBContext(&state->gc, ctx);
			ivm_context_setWB(ctx, 1);
		}
		return IVM_TRUE;
	}

	return IVM_FALSE;
}
*/

IVM_INLINE
ivm_bool_t
IVM_WBCTX(ivm_vmstate_t *state,
		  ivm_context_t *ctx,
		  ivm_slot_table_t *slots)
{
	if (ivm_context_getGen(ctx) &&
		!ivm_slot_table_getGen(slots)) {
		if (!ivm_context_getWB(ctx)) {
			ivm_collector_addWBContext(&state->gc, ctx);
			ivm_context_setWB(ctx, 1);
		}
		return IVM_TRUE;
	}

	return IVM_FALSE;
}

IVM_INLINE
ivm_bool_t
IVM_WBCTX_OBJ(ivm_vmstate_t *state,
			  ivm_context_t *ctx,
			  ivm_object_t *obj)
{
	if (ivm_context_getGen(ctx) &&
		!IVM_OBJECT_GET(obj, GEN)) {
		if (!ivm_context_getWB(ctx)) {
			ivm_collector_addWBContext(&state->gc, ctx);
			ivm_context_setWB(ctx, 1);
		}
		return IVM_TRUE;
	}

	return IVM_FALSE;
}

/*

IVM_INLINE
ivm_coro_t *
ivm_vmstate_curCoro(ivm_vmstate_t *state)
{
	return state->cur_coro;
}

*/

IVM_INLINE
ivm_coro_t *
ivm_vmstate_mainCoro(ivm_vmstate_t *state)
{
	return state->main_coro;
}

IVM_INLINE
void
ivm_vmstate_setMemError(ivm_vmstate_t *state)
{
	state->except
	= ivm_exception_new(state, IVM_ERROR_MSG_MEM_ERROR, "<core>", 0);
	return;
}

IVM_INLINE
void *
ivm_vmstate_alloc(ivm_vmstate_t *state, ivm_size_t size)
{
	ivm_bool_t add_block = IVM_FALSE;
	void *ret = ivm_heap_alloc_c((state)->heaps, size, &add_block);

	if (!ret) {
		ivm_vmstate_setMemError(state);
	}

	if (IVM_UNLIKELY(add_block)) {
		ivm_vmstate_openGCFlag(state);
	}

	return ret;
}

IVM_INLINE
ivm_string_t *
ivm_vmstate_preallocStr(ivm_vmstate_t *state,
						ivm_size_t len,
						ivm_char_t **buf)
{
	ivm_string_t *str = ivm_vmstate_alloc(state, IVM_STRING_GET_SIZE(len));

	ivm_string_initHead(str, IVM_FALSE, len);

	if (buf)
		*buf = ivm_string_trimHead(str);

	return str;
}

IVM_INLINE
void *
ivm_vmstate_addCopy(ivm_vmstate_t *state,
					void *ptr, ivm_size_t size)
{
	ivm_bool_t add_block = IVM_FALSE;
	void *ret = ivm_heap_addCopy_c((state)->heaps, ptr, size, &add_block);

	if (IVM_UNLIKELY(add_block)) {
		ivm_vmstate_openGCFlag(state);
	}

	return ret;
}

IVM_INLINE
void *
ivm_vmstate_addCopyAt(ivm_vmstate_t *state,
					  void *ptr, ivm_size_t size,
					  ivm_int_t heap)
{
	ivm_bool_t add_block = IVM_FALSE;
	void *ret = ivm_heap_addCopy_c((state)->heaps + heap, ptr, size, &add_block);

	if (IVM_UNLIKELY(add_block)) {
		ivm_vmstate_openGCFlag(state);
	}

	return ret;
}

IVM_INLINE
void *
ivm_vmstate_allocAt(ivm_vmstate_t *state, ivm_size_t size, ivm_int_t heap)
{
	ivm_bool_t add_block = IVM_FALSE;
	void *ret = ivm_heap_alloc_c((state)->heaps + heap, size, &add_block);

	if (!ret) {
		ivm_vmstate_setMemError(state);
	}

	if (IVM_UNLIKELY(add_block)) {
		ivm_vmstate_openGCFlag(state);
	}

	return ret;
}

IVM_INLINE
void
ivm_vmstate_swapHeap(ivm_vmstate_t *state,
					 ivm_int_t i, ivm_int_t j)
{
	ivm_heap_t tmp = state->heaps[i];
	state->heaps[i] = state->heaps[j];
	state->heaps[j] = tmp;

	return;
}

IVM_INLINE
void
ivm_vmstate_addWildSize(ivm_vmstate_t *state,
						ivm_size_t size)
{
	if (IVM_UNLIKELY(
			(state->wild_size += size) >
			IVM_DEFAULT_GC_WILD_THRESHOLD
		)) {
		state->wild_size = 0;
		ivm_collector_setGen(&state->gc, 1);
		ivm_vmstate_openGCFlag(state);
	}

	return;
}

IVM_INLINE
void *
ivm_vmstate_allocWild(ivm_vmstate_t *state,
					  ivm_size_t size)
{
	void *ret;

	ivm_vmstate_addWildSize(state, size);
	ret = STD_ALLOC(size);
	
	if (!ret && size) {
		ivm_vmstate_setMemError(state);
		return IVM_NULL;
	}

	// IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC);

	return ret;
}

IVM_INLINE
void *
ivm_vmstate_reallocWild(ivm_vmstate_t *state,
						void *orig,
						ivm_size_t size)
{
	void *ret;

	ivm_vmstate_addWildSize(state, size);
	ret = STD_REALLOC(orig, size);

	if (!ret && size) {
		ivm_vmstate_setMemError(state);
		return IVM_NULL;
	}

	return ret;
}

IVM_INLINE
void *
ivm_vmstate_tryAlloc(ivm_vmstate_t *state,
					 ivm_size_t size,
					 ivm_bool_t *is_wild)
{
	void *ret;

	if (size <= IVM_DEFAULT_MAX_OBJECT_SIZE) {
		*is_wild = IVM_FALSE;
		return ivm_vmstate_alloc(state, size);
	}

	ivm_vmstate_addWildSize(state, size);
	*is_wild = IVM_TRUE;
	ret = STD_ALLOC(size);

	if (!ret && size) {
		ivm_vmstate_setMemError(state);
		return IVM_NULL;
	}

	// IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC);

	return ret;
}

IVM_INLINE
const ivm_string_t *
ivm_vmstate_allocRawString(ivm_vmstate_t *state,
						   const ivm_char_t *str)
{
	ivm_size_t len = IVM_STRLEN(str),
			   size = IVM_STRING_GET_SIZE(len);
	ivm_string_t *ret;

	if (size < IVM_DEFAULT_MAX_CONST_STRING_SIZE) {
		return (const ivm_string_t *)
		ivm_string_pool_registerRaw(state->const_pool, str);
	}

	ret = ivm_vmstate_alloc(state, size);
	STD_MEMCPY(ivm_string_trimHead(ret), str, sizeof(*str) * (len + 1));
	ivm_string_initHead(ret, IVM_FALSE, len);

	return ret;
}

IVM_INLINE
const ivm_string_t *
ivm_vmstate_allocRawString_len(ivm_vmstate_t *state,
							   const ivm_char_t *str,
							   ivm_size_t len)
{
	ivm_char_t *cont;
	ivm_size_t size = IVM_STRING_GET_SIZE(len);
	ivm_string_t *ret;

	if (size < IVM_DEFAULT_MAX_CONST_STRING_SIZE) {
		return (const ivm_string_t *)
		ivm_string_pool_registerRaw_n(state->const_pool, str, len);
	}

	ret = ivm_vmstate_alloc(state, size);
	cont = ivm_string_trimHead(ret);
	STD_MEMCPY(cont, str, sizeof(*str) * len);
	cont[len] = '\0';

	ivm_string_initHead(ret, IVM_FALSE, len);

	return ret;
}

IVM_INLINE
const ivm_string_t *
ivm_vmstate_allocString(ivm_vmstate_t *state,
						const ivm_string_t *str)
{
	ivm_size_t size;
	ivm_string_t *ret;

	if (!ivm_string_isConst(str)) {
		size = IVM_STRING_GET_SIZE(ivm_string_length(str));
		ret = ivm_vmstate_alloc(state, size);
		
		STD_MEMCPY(ret, str, size);

		return ret;
	}

	return str;
}

IVM_INLINE
const ivm_string_t *
ivm_vmstate_allocChar(ivm_vmstate_t *state,
					  ivm_char_t c)
{
	return ivm_string_pool_registerRaw_n(state->const_pool, &c, 1);
}

/* function pool */
#if IVM_USE_FUNCTION_POOL

#define ivm_vmstate_allocFunc(state) \
	(ivm_function_pool_alloc((state)->func_pool))
#define ivm_vmstate_dumpFunc(state, func) \
	(ivm_function_pool_dump((state)->func_pool, (func)))

#else

#define ivm_vmstate_allocFunc(state) \
	(STD_ALLOC(sizeof(ivm_function_t)))
#define ivm_vmstate_dumpFunc(state, func) \
	(STD_FREE(func))

#endif

#define ivm_vmstate_allocFrame(state) \
	(STD_ALLOC(sizeof(ivm_frame_t)))
#define ivm_vmstate_dumpFrame(state, fr) \
	(STD_FREE(fr))

/* coro pool */
#if IVM_USE_CORO_POOL

#define ivm_vmstate_allocCoro(state) \
	(ivm_coro_pool_alloc(&(state)->cr_pool))
#define ivm_vmstate_dumpCoro(state, cr) \
	(ivm_coro_pool_dump(&(state)->cr_pool, (cr)))

#else

#define ivm_vmstate_allocCoro(state) \
	(STD_ALLOC(sizeof(ivm_coro_t)))
#define ivm_vmstate_dumpCoro(state, cr) \
	(STD_FREE(cr))

#endif

/* context pool */
#define ivm_vmstate_allocContext(state) \
	(ivm_context_pool_alloc(&(state)->ct_pool))
#define ivm_vmstate_dumpContext(state, ctx) \
	(ivm_context_pool_dump(&(state)->ct_pool, (ctx)))

#if IVM_USE_BLOCK_POOL

#define ivm_vmstate_allocBlock(state, count) \
	(ivm_block_pool_alloc(&(state)->block_pool, (count)))

#define ivm_vmstate_dumpBlock(state, block, count) \
	(ivm_block_pool_free(&(state)->block_pool, (block), (count)))

#define ivm_vmstate_reallocBlock(state, block, ocount, count) \
	(ivm_block_pool_realloc(&(state)->block_pool, (block), (ocount), (count)))

#else

#define ivm_vmstate_allocBlock(state, count) \
	(STD_ALLOC(sizeof(ivm_block_t) * (count)))

#define ivm_vmstate_dumpBlock(state, block, count) \
	(STD_FREE(block))

#define ivm_vmstate_reallocBlock(state, block, ocount, count) \
	(STD_REALLOC((block), sizeof(ivm_block_t) * (count)))

#endif

#define ivm_vmstate_getType(state, tag) (&(state)->type_list[tag])
#define ivm_vmstate_getTypeProto(state, tag) \
	ivm_type_getProto(ivm_vmstate_getType((state), (tag)))
#define ivm_vmstate_getTypeHeader(state, tag) \
	ivm_type_getHeader(ivm_vmstate_getType((state), (tag)))
#define ivm_vmstate_getTypeName(state, tag) \
	ivm_type_getName(ivm_vmstate_getType((state), (tag)))
#define ivm_vmstate_getTypeCons(state, tag) \
	ivm_type_getCons(ivm_vmstate_getType((state), (tag)))

#define ivm_vmstate_registerFunc(state, func) ivm_func_list_register(&(state)->func_list, (func))
#define ivm_vmstate_getFunc(state, id) ivm_func_list_at(&(state)->func_list, (id))
#define ivm_vmstate_getFuncID(state, func) ivm_func_list_find(&(state)->func_list, (func))
#define ivm_vmstate_getLinkOffset(state) ivm_func_list_size(&(state)->func_list)

IVM_INLINE
ivm_string_pool_t *
ivm_vmstate_getFuncStringPool(ivm_vmstate_t *state)
{
	if (ivm_func_list_size(&state->func_list)) {
		return ivm_function_getStringPool(ivm_vmstate_getFunc(state, 0));
	}

	return IVM_NULL;
}

IVM_INLINE
ivm_string_pool_t *
ivm_vmstate_getConstPool(ivm_vmstate_t *state)
{
	return state->const_pool;
}

#define ivm_vmstate_addDesLog(state, obj) (ivm_collector_addDesLog(&(state)->gc, (obj)))

#if 0

#define ivm_vmstate_checkGC(state) (ivm_collector_collect((state)->gc, (state), (state)->cur_heap))

#else

#define ivm_vmstate_checkGC(state) \
	ivm_vmstate_isGCFlagOpen(state)

#define ivm_vmstate_doGC(state) \
	ivm_collector_collect(&(state)->gc, (state), (state)->heaps); \
	ivm_vmstate_closeGCFlag(state);

#endif

#define ivm_vmstate_setException(state, obj) ((state)->except = (obj))
#define ivm_vmstate_getException(state) ((state)->except)

IVM_INLINE
ivm_object_t *
ivm_vmstate_popException(ivm_vmstate_t *state)
{
	ivm_object_t *tmp = state->except;
	state->except = IVM_NULL;
	return tmp;
}

#define ivm_vmstate_getLoadedMod(state) ((state)->loaded_mod)
#define _ivm_vmstate_setLoadedMod(state, obj) ((state)->loaded_mod = (obj))

#define ivm_vmstate_getTypePool(state) (&(state)->type_pool)

#define ivm_vmstate_getNone(state) ((state)->obj_none)
#define _ivm_vmstate_setNone(state, obj) ((state)->obj_none = (obj))

IVM_INLINE
void
ivm_vmstate_addCoroSet(ivm_vmstate_t *state,
					   ivm_coro_t *coro)
{
	ivm_coro_set_insert(&state->coro_set, coro);
	return;
}

IVM_INLINE
void
ivm_vmstate_removeCoroSet(ivm_vmstate_t *state,
						  ivm_coro_t *coro)
{
	ivm_coro_set_remove(&state->coro_set, coro);
	return;
}

/*
IVM_INLINE
void
ivm_vmstate_setCurCoro(ivm_vmstate_t *state,
					   ivm_coro_t *coro)
{
	IVM_WBCORO(state, state->cur_coro);
	// ivm_coro_free(state->cur_coro, state);
	IVM_WBCORO(state, state->cur_coro = coro);
	// ivm_coro_addRef(coro);

	return;
}
*/

IVM_INLINE
void
ivm_vmstate_setMainCoro(ivm_vmstate_t *state,
						ivm_coro_t *coro)
{
	if (state->main_coro) {
		IVM_FATAL("reset main coro");
	}

	state->main_coro = coro;
	ivm_coro_addRef(coro);

	return;
}

IVM_INLINE
ivm_object_t *
ivm_vmstate_resumeMainCoro(ivm_vmstate_t *state,
						  ivm_object_t *init)
{
	ivm_object_t *ret;

	if (state->main_coro) {
		// ivm_vmstate_threadStart(state);
		ret = ivm_coro_resume(state->main_coro, state, init);
		// ivm_vmstate_threadEnd(state);

		return ret;
	}

	return IVM_NULL;
}

ivm_bool_t
ivm_vmstate_joinAllThread(ivm_vmstate_t *state,
						  ivm_bool_t stop_clock);

ivm_object_t *
ivm_vmstate_spawnThread(ivm_vmstate_t *state,
						ivm_coro_t *coro,
						ivm_object_t *init);

// slot table UID
#define ivm_vmstate_genSTUID(state) ivm_uid_gen_nextPtr(&(state)->st_uid_gen)
#define ivm_vmstate_curSTUID(state) ivm_uid_gen_curPtr(&(state)->st_uid_gen)

void
ivm_vmstate_initInstrCache(ivm_vmstate_t *state);

IVM_COM_END

#endif
