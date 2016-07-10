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

typedef struct {
	ivm_object_t *base;
	ivm_argc_t argc;
	ivm_object_t **argv;
} ivm_function_arg_t;

typedef ivm_uint16_t ivm_signal_mask_t;
typedef ivm_object_t *(*ivm_native_function_t)(struct ivm_vmstate_t_tag *state,
											   ivm_ctchain_t *context,
											   ivm_function_arg_t arg);

#define IVM_FUNCTION_COMMON_ARG_PASS base, argc, argv

#define IVM_FUNCTION_SET_ARG_2(argc, argv) \
	(ivm_function_arg_t){ IVM_NULL, (argc), (argv) }
#define IVM_FUNCTION_SET_ARG_3(base, argc, argv) \
	(ivm_function_arg_t){ (base), (argc), (argv) }

#define IVM_NATIVE_FUNC(name) ivm_object_t *IVM_GET_NATIVE_FUNC(name)( \
											struct ivm_vmstate_t_tag *state, \
											ivm_ctchain_t *context, \
											ivm_function_arg_t arg)
#define IVM_GET_NATIVE_FUNC(name) ivm_native_function_##name

typedef ivm_ptlist_t ivm_param_list_t;
typedef ivm_char_t *ivm_parameter_t;
typedef ivm_parameter_t *ivm_param_list_iterator_t;

#define ivm_parameter_getName(param) (param)

#define ivm_param_list_new(argc) (ivm_ptlist_new_c(argc))
#define ivm_param_list_free ivm_ptlist_free
#define ivm_param_list_add ivm_ptlist_push
#define ivm_param_list_size ivm_ptlist_size
#define ivm_param_list_at(list, i) ((ivm_parameter_t *)ivm_ptlist_at((list), (i)))

#define IVM_PARAM_LIST_ITER_GET(iter) (*(iter))
#define IVM_PARAM_LIST_EACHPTR(list, iter) IVM_PTLIST_EACHPTR((list), iter, ivm_parameter_t)

enum {
	IVM_INTSIG_NONE			= 0,
	IVM_INTSIG_RETN			= 1 << 0,
	IVM_INTSIG_BREAK		= 1 << 1,
	IVM_INTSIG_CONTINUE		= 1 << 2
};

typedef struct ivm_function_t_tag {
	ivm_bool_t is_native;
	union {
		ivm_exec_t body;
		ivm_native_function_t native;
	} u;
	// ivm_signal_mask_t intsig;
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

#define ivm_function_callNative(func, state, context, arg) \
	(func)->u.native((state), (context), (arg));

#if 0
void
ivm_function_setParam(const ivm_function_t *func,
					  struct ivm_vmstate_t_tag *state,
					  ivm_ctchain_t *context, IVM_FUNCTION_COMMON_ARG);
#endif

typedef struct {
	IVM_OBJECT_HEADER
	ivm_ctchain_t *closure;
	const ivm_function_t *val;
} ivm_function_object_t;

void
ivm_function_object_destructor(ivm_object_t *obj,
							   struct ivm_vmstate_t_tag *state);

ivm_object_t *
ivm_function_object_new(struct ivm_vmstate_t_tag *state,
						ivm_ctchain_t *context,
						const ivm_function_t *func);

void
ivm_function_object_traverser(ivm_object_t *obj,
							  struct ivm_traverser_arg_t_tag *arg);

#define ivm_function_object_getClosure(obj) ((obj)->closure)
#define ivm_function_object_getFunc(obj) ((obj)->val)

typedef ivm_ptpool_t ivm_function_pool_t;

#define ivm_function_pool_new(count) (ivm_ptpool_new((count), sizeof(ivm_function_t)))
#define ivm_function_pool_free ivm_ptpool_free
#define ivm_function_pool_alloc(pool) ((ivm_function_t *)ivm_ptpool_alloc(pool))
#define ivm_function_pool_dump ivm_ptpool_dump
#define ivm_function_pool_dumpAll ivm_ptpool_dumpAll

typedef ivm_size_t ivm_func_id_t;
typedef ivm_ptlist_t ivm_func_list_t;
typedef IVM_PTLIST_ITER_TYPE(ivm_function_t *) ivm_func_list_iterator_t;

#define ivm_func_list_new() (ivm_ptlist_new_c(IVM_DEFAULT_FUNC_LIST_BUFFER_SIZE))
#define ivm_func_list_size ivm_ptlist_size
#define ivm_func_list_at(list, i) ((ivm_function_t *)ivm_ptlist_at((list), (i)))

#define IVM_FUNC_LIST_ITER_INDEX IVM_PTLIST_ITER_INDEX
#define IVM_FUNC_LIST_ITER_SET(iter, val) IVM_PTLIST_ITER_SET((iter), (val))
#define IVM_FUNC_LIST_ITER_GET(iter) IVM_PTLIST_ITER_GET(iter)
#define IVM_FUNC_LIST_EACHPTR(list, iter) IVM_PTLIST_EACHPTR((list), iter, ivm_function_t *)

IVM_INLINE
ivm_size_t
ivm_func_list_register(ivm_func_list_t *list,
					   ivm_function_t *func)
{
	ivm_func_list_iterator_t fiter;

	IVM_FUNC_LIST_EACHPTR(list, fiter) {
		if (IVM_FUNC_LIST_ITER_GET(fiter) == func)
			return IVM_FUNC_LIST_ITER_INDEX(list, fiter);
	}

	return ivm_ptlist_push(list, func);
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

IVM_COM_END

#endif
