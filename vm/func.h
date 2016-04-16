#ifndef _IVM_VM_FUNC_H_
#define _IVM_VM_FUNC_H_

#include "context.h"
#include "exec.h"
#include "type.h"

struct ivm_vmstate_t_tag;

typedef ivm_uint16_t ivm_argc_t;
typedef ivm_uint16_t ivm_signal_mask_t;
typedef ivm_object_t *(*ivm_native_function_t)(struct ivm_vmstate_t_tag *state,
											   ivm_ctchain_t *context, ivm_object_t *base,
											   ivm_argc_t argc, ivm_object_t **argv);

enum {
	IVM_INTSIG_NONE			= 0,
	IVM_INTSIG_RETN			= 1 << 0,
	IVM_INTSIG_BREAK		= 1 << 1,
	IVM_INTSIG_CONTINUE		= 1 << 2
};

typedef struct {
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

#endif
