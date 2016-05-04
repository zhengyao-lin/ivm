#ifndef _IVM_VM_FUNC_H_
#define _IVM_VM_FUNC_H_

#include "type.h"
#include "context.h"
#include "exec.h"
#include "runtime.h"
#include "obj.h"

struct ivm_vmstate_t_tag;
struct ivm_caller_info_t_tag;
struct ivm_coro_t_tag;

typedef ivm_uint16_t ivm_argc_t;
typedef ivm_uint16_t ivm_signal_mask_t;
typedef ivm_object_t *(*ivm_native_function_t)(struct ivm_vmstate_t_tag *state,
											   ivm_ctchain_t *context, ivm_object_t *base,
											   ivm_argc_t argc, ivm_object_t **argv);

#define IVM_NATIVE_FUNC(name) ivm_object_t *IVM_GET_NATIVE_FUNC(name)(\
											struct ivm_vmstate_t_tag *state, \
											ivm_ctchain_t *context, ivm_object_t *base, \
											ivm_argc_t argc, ivm_object_t **argv)
#define IVM_GET_NATIVE_FUNC(name) ivm_native_function_##name

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
ivm_function_new(ivm_ctchain_t *context, ivm_exec_t *body, ivm_signal_mask_t intsig);
ivm_function_t *
ivm_function_newNative(ivm_native_function_t func, ivm_signal_mask_t intsig);
void
ivm_function_free(ivm_function_t *func);
ivm_function_t *
ivm_function_clone(ivm_function_t *func);

#define ivm_function_isNative(func) ((func) && (func)->is_native)
ivm_runtime_t *
ivm_function_createRuntime(const ivm_function_t *func);
struct ivm_caller_info_t_tag *
ivm_function_invoke(const ivm_function_t *func, struct ivm_coro_t_tag *coro);
ivm_object_t *
ivm_function_callNative(const ivm_function_t *func,
						struct ivm_vmstate_t_tag *state,
						ivm_ctchain_t *context, ivm_object_t *base,
						ivm_argc_t argc, ivm_object_t **argv);

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

#endif
