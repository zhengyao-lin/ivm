#ifndef _IVM_VM_INLINE_TYPEOBJ_H_
#define _IVM_VM_INLINE_TYPEOBJ_H_

#include "pub/com.h"
#include "pub/vm.h"

#include "vm/typeobj.h"
#include "vm/obj.h"

IVM_COM_HEADER

IVM_INLINE
ivm_object_t *
ivm_type_object_new(ivm_vmstate_t *state,
					ivm_type_t *val)
{
	ivm_type_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), IVM_BTTYPE(state, IVM_TYPE_OBJECT_T));

	ret->val = val;

	return IVM_AS_OBJ(ret);
}

IVM_COM_END

#endif
