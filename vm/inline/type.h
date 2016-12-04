#ifndef _IVM_VM_INLINE_TYPE_H_
#define _IVM_VM_INLINE_TYPE_H_

#include "pub/com.h"
#include "pub/vm.h"

#include "vm/func.h"
#include "vm/type.h"

IVM_COM_HEADER

IVM_INLINE
ivm_bool_t
ivm_type_checkCons(ivm_type_t *type,
				   ivm_native_function_t cons)
{
	struct ivm_function_t_tag *func = type->cons;

	return ivm_function_isNative(func) && ivm_function_getNative(func) == cons;
}

IVM_COM_END

#endif
