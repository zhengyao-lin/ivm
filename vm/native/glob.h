#ifndef _IVM_VM_NATIVE_GLOB_H_
#define _IVM_VM_NATIVE_GLOB_H_

#include "pub/com.h"
#include "pub/type.h"
#include "pub/vm.h"

IVM_COM_HEADER

void
ivm_native_global_bind(ivm_vmstate_t *state,
					   ivm_context_t *ctx);

IVM_COM_END

#endif
