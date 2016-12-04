#ifndef _IVM_VM_NATIVE_NATIVE_H_
#define _IVM_VM_NATIVE_NATIVE_H_

#include "pub/com.h"
#include "pub/type.h"
#include "pub/vm.h"

#include "ncoro.h"
#include "nbuf.h"
#include "nrange.h"
#include "nlistobj.h"
#include "nstrobj.h"
#include "nfunc.h"
#include "nnum.h"
#include "ntypeobj.h"
#include "nobj.h"

#include "glob.h"

/* 0 for success, other return stand for the position of the wrong argument */
ivm_int_t
ivm_native_matchArgument(ivm_function_arg_t arg,
						 ivm_vmstate_t *state,
						 const ivm_char_t *rule, ...);

#define IVM_NATIVE_WRAP(state, name) \
	ivm_function_object_newNative((state), IVM_GET_NATIVE_FUNC(name))

#define IVM_BUILTIN_WRAP(state, name) \
	ivm_function_object_new((state), IVM_NULL, IVM_GET_BUILTIN_FUNC(name)(state))

#define IVM_NATIVE_WRAP_C(state, name) \
	ivm_function_newNative((state), IVM_GET_NATIVE_FUNC(name))

#define IVM_BUILTIN_WRAP_C(state, name) \
	IVM_GET_BUILTIN_FUNC(name)(state)

ivm_object_t *
IVM_NATIVE_WRAP_CONS_c(ivm_vmstate_t *state,
					   ivm_object_t *proto,
					   ivm_native_function_t func);

#define IVM_NATIVE_WRAP_CONS(state, proto, name) \
	IVM_NATIVE_WRAP_CONS_c((state), (proto), IVM_GET_NATIVE_FUNC(name))

#define IVM_NATIVE_IMPORT_FUNC "$import"
#define IVM_NATIVE_IS_FUNC "$is"
#define IVM_NATIVE_VARG_NAME "$varg"

#endif
