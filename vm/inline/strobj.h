#ifndef _IVM_VM_INLINE_STROBJ_H_
#define _IVM_VM_INLINE_STROBJ_H_

#include "pub/com.h"
#include "pub/vm.h"

#include "../strobj.h"
#include "../obj.h"

IVM_COM_HEADER

IVM_INLINE
ivm_object_t *
ivm_string_object_new_c(ivm_vmstate_t *state,
						ivm_string_t *val)
{
	ivm_string_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), state, IVM_STRING_OBJECT_T);

	ret->val = val;
	ret->is_wild = IVM_FALSE;

	return IVM_AS_OBJ(ret);
}

/* string is allocated in system heap */
IVM_INLINE
ivm_object_t *
ivm_string_object_new_r(ivm_vmstate_t *state,
						ivm_string_t *val)
{
	ivm_string_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), state, IVM_STRING_OBJECT_T);

	ret->val = val;
	ret->is_wild = IVM_TRUE;

	ivm_vmstate_addDesLog(state, IVM_AS_OBJ(ret));

	return IVM_AS_OBJ(ret);
}

IVM_INLINE
ivm_object_t *
ivm_string_object_new(ivm_vmstate_t *state,
					  const ivm_string_t *val)
{
	ivm_string_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), state, IVM_STRING_OBJECT_T);

	ret->val = ivm_string_copyIfNotConst_pool(val, state);
	ret->is_wild = IVM_FALSE;

	return IVM_AS_OBJ(ret);
}

IVM_COM_END

#endif
