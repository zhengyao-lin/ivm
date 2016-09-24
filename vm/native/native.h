#ifndef _IVM_VM_NATIVE_NATIVE_H_
#define _IVM_VM_NATIVE_NATIVE_H_

#include "pub/com.h"
#include "pub/type.h"
#include "pub/vm.h"

#include "nrange.h"
#include "nlistobj.h"
#include "nstrobj.h"
#include "nfunc.h"
#include "nnum.h"
#include "nobj.h"

/* 0 for success, other return stand for the position of the wrong argument */
ivm_int_t
ivm_native_matchArgument(ivm_function_arg_t arg,
						 ivm_vmstate_t *state,
						 const ivm_char_t *rule, ...);

#define IVM_NATIVE_WRAP(state, name) \
	ivm_function_object_newNative((state), IVM_GET_NATIVE_FUNC(name))

#endif
