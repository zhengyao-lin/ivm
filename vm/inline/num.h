#ifndef _IVM_VM_INLINE_NUM_H_
#define _IVM_VM_INLINE_NUM_H_

#include "pub/com.h"
#include "pub/vm.h"

#include "../num.h"
#include "../obj.h"

IVM_COM_HEADER

IVM_INLINE
ivm_object_t *
ivm_numeric_new(ivm_vmstate_t *state, ivm_number_t val)
{
	ivm_numeric_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), state, IVM_NUMERIC_T);

	ret->val = val;

	return IVM_AS_OBJ(ret);
}

IVM_COM_END

#endif
