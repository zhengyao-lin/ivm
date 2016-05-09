#ifndef _IVM_VM_FUNC_H_
#define _IVM_VM_FUNC_H_

#include "type.h"
#include "context.h"
#include "exec.h"
#include "obj.h"

#define IVM_FUNCTION_COMMON_ARG ivm_object_t *base, ivm_argc_t argc, ivm_object_t **argv
#define IVM_FUNCTION_COMMON_ARG_PASS base, argc, argv

#define IVM_FUNCTION_SET_ARG_2(argc, argv) IVM_NULL, (argc), (argv)
#define IVM_FUNCTION_SET_ARG_3(base, argc, argv) (base), (argc), (argv)

struct ivm_vmstate_t_tag;
struct ivm_caller_info_t_tag;
struct ivm_coro_t_tag;
struct ivm_runtime_t_tag;
struct ivm_traverser_arg_t_tag;

typedef ivm_uint32_t ivm_argc_t;
typedef ivm_uint16_t ivm_signal_mask_t;
typedef ivm_object_t *(*ivm_native_function_t)(struct ivm_vmstate_t_tag *state,
											   ivm_ctchain_t *context,
											   IVM_FUNCTION_COMMON_ARG);

#define IVM_NATIVE_FUNC(name) ivm_object_t *IVM_GET_NATIVE_FUNC(name)(\
											struct ivm_vmstate_t_tag *state, \
											ivm_ctchain_t *context, ivm_object_t *base, \
											ivm_argc_t argc, ivm_object_t **argv)
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
#define IVM_PARAM_LIST_EACHPTR(list, iter) IVM_PTLIST_EACHPTR((list), (iter), ivm_parameter_t)

enum {
	IVM_INTSIG_NONE			= 0,
	IVM_INTSIG_RETN			= 1 << 0,
	IVM_INTSIG_BREAK		= 1 << 1,
	IVM_INTSIG_CONTINUE		= 1 << 2
};

typedef struct ivm_function_t_tag {
	ivm_bool_t is_native;
	union {
		struct {
			ivm_ctchain_t *closure;
			ivm_exec_t *body;
		} f;
		ivm_native_function_t native;
	} u;
	ivm_signal_mask_t intsig;
} ivm_function_t;

ivm_function_t *
ivm_function_new(ivm_ctchain_t *context,
				 ivm_exec_t *body,
				 ivm_signal_mask_t intsig);
ivm_function_t *
ivm_function_newNative(ivm_native_function_t func, ivm_signal_mask_t intsig);
void
ivm_function_free(ivm_function_t *func);
ivm_function_t *
ivm_function_clone(ivm_function_t *func);

#define ivm_function_isNative(func) ((func) && (func)->is_native)

struct ivm_runtime_t_tag *
ivm_function_createRuntime(struct ivm_vmstate_t_tag *state,
						   const ivm_function_t *func);

struct ivm_caller_info_t_tag *
ivm_function_invoke(const ivm_function_t *func,
					struct ivm_coro_t_tag *coro);

ivm_object_t *
ivm_function_callNative(const ivm_function_t *func,
						struct ivm_vmstate_t_tag *state,
						ivm_ctchain_t *context, IVM_FUNCTION_COMMON_ARG);

#if 0
void
ivm_function_setParam(const ivm_function_t *func,
					  struct ivm_vmstate_t_tag *state,
					  ivm_ctchain_t *context, IVM_FUNCTION_COMMON_ARG);
#endif

typedef struct {
	IVM_OBJECT_HEADER
	ivm_function_t *val;
} ivm_function_object_t;

void
ivm_function_object_destructor(ivm_object_t *obj,
							   struct ivm_vmstate_t_tag *state);

ivm_object_t *
ivm_function_object_new(struct ivm_vmstate_t_tag *state,
						ivm_function_t *func);
/* no clone */
ivm_object_t *
ivm_function_object_new_nc(struct ivm_vmstate_t_tag *state,
						   ivm_function_t *func);

void
ivm_function_object_traverser(ivm_object_t *obj,
							  struct ivm_traverser_arg_t_tag *arg);

#endif
