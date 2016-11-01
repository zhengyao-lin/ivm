#ifndef _IVM_VM_FUNC_H_
#define _IVM_VM_FUNC_H_

#include "pub/com.h"
#include "pub/type.h"

#include "std/pool.h"

#include "context.h"
#include "exec.h"
#include "obj.h"

IVM_COM_HEADER

struct ivm_vmstate_t_tag;
struct ivm_frame_t_tag;
struct ivm_coro_t_tag;
struct ivm_runtime_t_tag;
struct ivm_traverser_arg_t_tag;
struct ivm_frame_stack_t_tag;

typedef ivm_uint16_t ivm_signal_mask_t;

#define IVM_FUNCTION_COMMON_ARG_PASS base, argc, argv

#define IVM_FUNCTION_SET_ARG_2(argc, argv) \
	(ivm_function_arg_t){ IVM_NULL, (argc), (argv) }
#define IVM_FUNCTION_SET_ARG_3(base, argc, argv) \
	(ivm_function_arg_t){ (base), (argc), (argv) }

#define IVM_GET_NATIVE_FUNC(name) ivm_native_function_##name

#define IVM_NATIVE_FUNC(name) \
	ivm_object_t *IVM_GET_NATIVE_FUNC(name)(struct ivm_vmstate_t_tag *__state__, \
											struct ivm_coro_t_tag *__coro__, \
											ivm_context_t *__context__, \
											ivm_function_arg_t __arg__)

#define IVM_NATIVE_FUNC_C(name) \
	ivm_object_t *name(struct ivm_vmstate_t_tag *__state__, \
					   struct ivm_coro_t_tag *__coro__, \
					   ivm_context_t *__context__, \
					   ivm_function_arg_t __arg__)

#define NAT_STATE() (__state__)
#define NAT_CORO() (__coro__)
#define NAT_CONTEXT() (__context__)

#define NAT_ARG() (__arg__)
#define NAT_BASE() (__arg__.base)
#define NAT_BASE_C(t) (IVM_AS(__arg__.base, t))
#define NAT_ARGC() (__arg__.argc)
#define NAT_ARG_AT(i) (__arg__.argv[-(i)])

typedef const ivm_string_t *ivm_parameter_t;

typedef ivm_ptlist_t ivm_param_list_t;
typedef IVM_PTLIST_ITER_TYPE(ivm_parameter_t) ivm_param_list_iterator_t;

#define ivm_parameter_getName(param) (param)

#define ivm_param_list_new(argc) (ivm_ptlist_new_c(argc))
#define ivm_param_list_free ivm_ptlist_free
#define ivm_param_list_add ivm_ptlist_push
#define ivm_param_list_size ivm_ptlist_size
#define ivm_param_list_at(list, i) ((ivm_parameter_t)ivm_ptlist_at((list), (i)))

#define IVM_PARAM_LIST_ITER_GET(iter) (*(iter))
#define IVM_PARAM_LIST_EACHPTR(list, iter) IVM_PTLIST_EACHPTR((list), iter, ivm_parameter_t)

typedef ivm_ptlist_t ivm_func_list_t;

typedef struct ivm_function_t_tag {
	union {
		ivm_exec_t body;
		ivm_native_function_t native;
	} u;

	ivm_uint_t ref;
	ivm_bool_t is_native;
} ivm_function_t;

ivm_function_t *
ivm_function_new(struct ivm_vmstate_t_tag *state,
				 ivm_exec_t *body);

ivm_function_t *
ivm_function_newNative(struct ivm_vmstate_t_tag *state,
					   ivm_native_function_t func);

void
ivm_function_free(ivm_function_t *func,
				  struct ivm_vmstate_t_tag *state);

#define ivm_function_isNative(func) ((func) && (func)->is_native)

#define ivm_function_callNative(func, state, coro, context, arg) \
	((func)->u.native((state), (coro), (context), (arg)))

#define ivm_function_getStringPool(func) ivm_exec_pool(&(func)->u.body)

IVM_INLINE
ivm_function_t *
ivm_function_addRef(ivm_function_t *func)
{
	func->ref++;
	return func;
}

/*
IVM_INLINE
ivm_int_t
ivm_function_getMaxStack(const ivm_function_t *func)
{
	if (!func->is_native) return func->u.body.max_stack;
	return 0;
}
*/

#define IVM_CALLABLE_HEADER \
	IVM_OBJECT_HEADER       \
	ivm_context_t *scope;   \
	ivm_function_t *val;

typedef struct {
	IVM_CALLABLE_HEADER
} ivm_function_object_t;

IVM_INLINE
ivm_bool_t
ivm_function_object_checkNative_c(ivm_object_t *obj,
								  ivm_native_function_t expect)
{
	ivm_function_t *f = IVM_AS(obj, ivm_function_object_t)->val;
	return ivm_function_isNative(f) && f->u.native == expect;
}

#define ivm_function_object_checkNative(obj, func) \
	ivm_function_object_checkNative_c((obj), IVM_GET_NATIVE_FUNC(func))

void
ivm_function_object_destructor(ivm_object_t *obj,
							   struct ivm_vmstate_t_tag *state);

void
ivm_function_object_cloner(ivm_object_t *obj,
						   struct ivm_vmstate_t_tag *state);

void
ivm_function_object_traverser(ivm_object_t *obj,
							  struct ivm_traverser_arg_t_tag *arg);

#define ivm_function_object_getScope(obj) ((obj)->scope)
#define ivm_function_object_getFunc(obj) ((obj)->val)

typedef ivm_ptpool_t ivm_function_pool_t;

#define ivm_function_pool_new(count) (ivm_ptpool_new((count), sizeof(ivm_function_t)))
#define ivm_function_pool_free ivm_ptpool_free
#define ivm_function_pool_alloc(pool) ((ivm_function_t *)ivm_ptpool_alloc(pool))
#define ivm_function_pool_dump ivm_ptpool_dump
#define ivm_function_pool_dumpAll ivm_ptpool_dumpAll

typedef ivm_size_t ivm_func_id_t;
// typedef ivm_func_list_t -> above ivm_function_t
typedef IVM_PTLIST_ITER_TYPE(ivm_function_t *) ivm_func_list_iterator_t;

#define ivm_func_list_new() (ivm_ptlist_new_c(IVM_DEFAULT_FUNC_LIST_BUFFER_SIZE))
#define ivm_func_list_init(list) (ivm_ptlist_init_c((list), IVM_DEFAULT_FUNC_LIST_BUFFER_SIZE))
#define ivm_func_list_size ivm_ptlist_size
#define ivm_func_list_at(list, i) ((ivm_function_t *)ivm_ptlist_at((list), (i)))
#define ivm_func_list_find ivm_ptlist_find

#define IVM_FUNC_LIST_ITER_INDEX IVM_PTLIST_ITER_INDEX
#define IVM_FUNC_LIST_ITER_SET(iter, val) IVM_PTLIST_ITER_SET((iter), (val))
#define IVM_FUNC_LIST_ITER_GET(iter) IVM_PTLIST_ITER_GET(iter)
#define IVM_FUNC_LIST_EACHPTR(list, iter) IVM_PTLIST_EACHPTR((list), iter, ivm_function_t *)

IVM_INLINE
ivm_size_t
ivm_func_list_register(ivm_func_list_t *list,
					   ivm_function_t *func)
{
/*
	ivm_func_list_iterator_t fiter;

	IVM_FUNC_LIST_EACHPTR(list, fiter) {
		if (IVM_FUNC_LIST_ITER_GET(fiter) == func)
			return IVM_FUNC_LIST_ITER_INDEX(list, fiter);
	}
*/
	return ivm_ptlist_push(list, ivm_function_addRef(func));
}

IVM_INLINE
void
ivm_func_list_free(ivm_func_list_t *list,
				   struct ivm_vmstate_t_tag *state)
{
	ivm_func_list_iterator_t fiter;

	if (list) {
		IVM_FUNC_LIST_EACHPTR(list, fiter) {
			ivm_function_free(IVM_FUNC_LIST_ITER_GET(fiter), state);
		}
		ivm_ptlist_free(list);
	}

	return;
}

IVM_INLINE
void
ivm_func_list_dump(ivm_func_list_t *list,
				   struct ivm_vmstate_t_tag *state)
{
	ivm_func_list_iterator_t fiter;

	if (list) {
		IVM_FUNC_LIST_EACHPTR(list, fiter) {
			ivm_function_free(IVM_FUNC_LIST_ITER_GET(fiter), state);
		}
		ivm_ptlist_dump(list);
	}

	return;
}

IVM_INLINE
void
ivm_func_list_empty(ivm_func_list_t *list,
					struct ivm_vmstate_t_tag *state)
{
	ivm_func_list_iterator_t fiter;

	IVM_FUNC_LIST_EACHPTR(list, fiter) {
		ivm_function_free(IVM_FUNC_LIST_ITER_GET(fiter), state);
	}
	ivm_ptlist_empty(list);

	return;
}

void
ivm_function_setExec(ivm_function_t *func,
					 struct ivm_vmstate_t_tag *state,
					 ivm_exec_t *body);

/*

ivm_instr_t *
ivm_function_createRuntime(const ivm_function_t *func,
						   struct ivm_vmstate_t_tag *state,
						   ivm_context_t *ctx,
						   struct ivm_coro_t_tag *coro);

ivm_instr_t *
ivm_function_invoke(const ivm_function_t *func,
					struct ivm_vmstate_t_tag *state,
					ivm_context_t *ctx,
					struct ivm_runtime_t_tag *runtime,
					struct ivm_frame_stack_t_tag *frame_st);

ivm_instr_t *
ivm_function_invokeBase(const ivm_function_t *func,
						struct ivm_vmstate_t_tag *state,
						ivm_context_t *ctx,
						struct ivm_runtime_t_tag *runtime,
						struct ivm_frame_stack_t_tag *frame_st,
						ivm_object_t *base);

ivm_instr_t *
ivm_function_invoke_r(const ivm_function_t *func,
					  struct ivm_vmstate_t_tag *state,
					  struct ivm_coro_t_tag *coro,
					  ivm_context_t *ctx);

ivm_instr_t *
ivm_function_object_invoke(ivm_function_object_t *obj,
						   struct ivm_vmstate_t_tag *state,
						   struct ivm_coro_t_tag *coro);

*/

IVM_COM_END

#endif
