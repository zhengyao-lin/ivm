#ifndef _IVM_VM_INLINE_STROBJ_H_
#define _IVM_VM_INLINE_STROBJ_H_

#include "pub/com.h"
#include "pub/vm.h"

#include "vm/strobj.h"
#include "vm/obj.h"

IVM_COM_HEADER

IVM_INLINE
ivm_object_t *
ivm_string_object_new_c(ivm_vmstate_t *state,
						ivm_string_t *val)
{
	ivm_string_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), IVM_BTTYPE(state, IVM_STRING_OBJECT_T));

	ret->val = val;

	return IVM_AS_OBJ(ret);
}

/* string is allocated in system heap(wild) */
/*
IVM_INLINE
ivm_object_t *
ivm_string_object_new_w(ivm_vmstate_t *state,
						ivm_string_t *val)
{
	ivm_string_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), state, IVM_STRING_OBJECT_T);

	ret->val = val;
	ret->is_wild = IVM_TRUE;

	ivm_vmstate_addDesLog(state, IVM_AS_OBJ(ret));

	return IVM_AS_OBJ(ret);
}
*/

/* create with raw c string */
IVM_INLINE
ivm_object_t *
ivm_string_object_new_r(ivm_vmstate_t *state,
						const ivm_char_t *val)
{
	ivm_string_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), IVM_BTTYPE(state, IVM_STRING_OBJECT_T));

	ret->val = ivm_vmstate_allocRawString(state, val);

	return IVM_AS_OBJ(ret);
}

IVM_INLINE
ivm_object_t *
ivm_string_object_newChar(ivm_vmstate_t *state,
						  ivm_char_t c)
{
	ivm_string_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), IVM_BTTYPE(state, IVM_STRING_OBJECT_T));

	ret->val = ivm_vmstate_allocChar(state, c);

	return IVM_AS_OBJ(ret);
}

IVM_INLINE
ivm_object_t *
ivm_string_object_new_rl(ivm_vmstate_t *state,
						 const ivm_char_t *val,
						 ivm_size_t len)
{
	ivm_string_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), IVM_BTTYPE(state, IVM_STRING_OBJECT_T));

	ret->val = ivm_vmstate_allocRawString_len(state, val, len);

	return IVM_AS_OBJ(ret);
}

IVM_INLINE
ivm_object_t *
ivm_string_object_new(ivm_vmstate_t *state,
					  const ivm_string_t *val)
{
	ivm_string_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), IVM_BTTYPE(state, IVM_STRING_OBJECT_T));

	ret->val = ivm_vmstate_allocString(state, val);

	return IVM_AS_OBJ(ret);
}

IVM_INLINE
ivm_bool_t
ivm_string_object_multiply(ivm_string_object_t *obj,
						   ivm_vmstate_t *state,
						   ivm_size_t times)
{
	const ivm_string_t *str = ivm_string_object_getValue(obj);
	ivm_size_t len = ivm_string_length(str);
	ivm_size_t dlen, i;
	ivm_string_t *ret;
	const ivm_char_t *orig;
	ivm_char_t *cur;

	dlen = len * times;

	if (!dlen) {
		obj->val = IVM_VMSTATE_CONST(state, C_EMPTY);
		return IVM_TRUE;
	}

	obj->val = ret = ivm_vmstate_alloc(state, IVM_STRING_GET_SIZE(dlen));

	if (!ret) {
		return IVM_FALSE;
	}

	orig = ivm_string_trimHead(str);
	cur = ivm_string_trimHead(ret);

	for (i = 0; i != times; i++, cur += len) {
		STD_MEMCPY(cur, orig, sizeof(ivm_char_t) * len);
	}

	ivm_string_trimHead(ret)[dlen] = '\0';
	ivm_string_initHead(ret, IVM_FALSE, dlen);

	return IVM_TRUE;
}

IVM_COM_END

#endif
