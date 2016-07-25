#ifndef _IVM_VM_NATIVE_NATIVE_H_
#define _IVM_VM_NATIVE_NATIVE_H_

#include "pub/com.h"
#include "pub/type.h"
#include "pub/vm.h"

#include "list.h"

#define IVM_NATIVE_WRAP(state, name) \
	ivm_function_object_newNative((state), IVM_GET_NATIVE_FUNC(name))

#endif
