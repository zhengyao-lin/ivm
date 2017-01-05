#ifndef _IVM_VM_INLINE_OBJ_H_
#define _IVM_VM_INLINE_OBJ_H_

#include "pub/com.h"
#include "pub/vm.h"

#include "std/string.h"

#include "vm/instr.h"
#include "vm/obj.h"
#include "vm/slot.h"

IVM_COM_HEADER

IVM_INLINE
void
ivm_object_init(ivm_object_t *obj,
				ivm_type_t *type)
{
	// STD_INIT(&obj->slots, sizeof(obj->slots) + sizeof(obj->mark));
	// obj->proto = ivm_type_getProto(
	// 	obj->type = ivm_vmstate_getType(state, type)
	// );

	STD_MEMCPY(obj, ivm_type_getHeader(type), sizeof(obj->type) + sizeof(obj->proto));
	STD_INIT(&obj->slots, sizeof(obj->slots) + sizeof(obj->mark));

	return;
}

IVM_INLINE
ivm_object_t *
ivm_object_clone(ivm_object_t *obj,
				 ivm_vmstate_t *state)
{
	ivm_type_t *type = IVM_TYPE_OF(obj);
	ivm_size_t size = type->size;
	ivm_object_t *ret = ivm_vmstate_alloc(state, size);

	STD_MEMCPY(ret, obj, size);

	IVM_OBJECT_SET(ret, GEN, 0);
	IVM_OBJECT_SET(ret, WB, 0);
	ret->slots = ivm_slot_table_copyShared(ret->slots, state);

	if (type->clone) {
		type->clone(ret, state);
	}

	return ret;
}

IVM_INLINE
ivm_object_t *
ivm_object_new(ivm_vmstate_t *state)
{
	ivm_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(ret, IVM_BTTYPE(state, IVM_OBJECT_T));

	return ret;
}

IVM_INLINE
ivm_object_t *
ivm_object_new_c(ivm_vmstate_t *state,
				 ivm_size_t prealloc)
{
	ivm_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(ret, IVM_BTTYPE(state, IVM_OBJECT_T));
	ret->slots = ivm_slot_table_new_c(state, prealloc);

	return ret;
}

IVM_INLINE
ivm_object_t *
ivm_object_new_t(ivm_vmstate_t *state,
				 ivm_slot_table_t *slots)
{
	ivm_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(ret, IVM_BTTYPE(state, IVM_OBJECT_T));
	ret->slots = slots;

	if (slots) {
		ret->mark.sub.oop = IVM_TRUE; // ivm_slot_table_hasOop(slots);
	}

	return ret;
}

IVM_INLINE
void
ivm_object_initSlots(ivm_object_t *obj,
					 ivm_vmstate_t *state,
					 ivm_size_t prealloc)
{
	obj->slots = ivm_slot_table_new_c(state, prealloc);
	return;
}

IVM_INLINE
ivm_object_t *
ivm_none_new(ivm_vmstate_t *state)
{
	ivm_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(ret, IVM_BTTYPE(state, IVM_NONE_T));
	ret->mark.sub.locked = IVM_TRUE;

	return ret;
}

IVM_INLINE
ivm_bool_t
ivm_object_hasProto(ivm_object_t *obj,
					ivm_object_t *proto)
{
	while (obj) {
		if (obj == proto) return IVM_TRUE;
		obj = obj->proto;
	}

	return IVM_FALSE;
}

IVM_INLINE
void
ivm_object_setProto(ivm_object_t *obj,
					ivm_vmstate_t *state,
					ivm_object_t *proto)
{
	if (proto) {
		IVM_WBOBJ(state, obj, proto);
		// obj->mark.sub.oop |= proto->mark.sub.oop;
	}
	
	obj->proto = proto;

	return;
}

IVM_INLINE
ivm_object_t *
ivm_object_getProto(ivm_object_t *obj)
{
	return obj->proto;
}

#define IVM_NONE(state) ivm_vmstate_getNone(state)
#define IVM_IS_NONE(state, obj) ((obj) == ivm_vmstate_getNone(state))

IVM_INLINE
ivm_object_t *
ivm_object_getOop(ivm_object_t *obj,
				  ivm_int_t op)
{
	register ivm_object_t *tmp;

	/* HACK: costs too much time */
	do {
		if (IVM_UNLIKELY(ivm_object_hasOop(obj))) {
			tmp = ivm_slot_table_getOop(obj->slots, op);
			if (tmp) return tmp;
		}
		obj = obj->proto;
	} while (obj);

	return IVM_NULL;
}

IVM_INLINE
ivm_slot_table_t *
ivm_object_copyOnWrite(ivm_object_t *obj,
					   ivm_vmstate_t *state)
{
	if (obj->slots && ivm_slot_table_isShared(obj->slots)) {
		obj->slots = ivm_slot_table_copyOnWrite(obj->slots, state);
		IVM_WBOBJ_SLOT(state, obj, obj->slots);
	}

	return obj->slots;
}

IVM_INLINE
void
ivm_object_merge(ivm_object_t *obj,
				 ivm_vmstate_t *state,
				 ivm_object_t *mergee,
				 ivm_bool_t overw)
{
	if (obj->slots && mergee->slots) {
		ivm_object_copyOnWrite(obj, state);
		ivm_slot_table_merge(obj->slots, state, mergee->slots, overw);
		obj->mark.sub.oop |= mergee->mark.sub.oop;
	} else if (mergee->slots) {
		obj->slots = ivm_slot_table_copy_state(mergee->slots, state);
		obj->mark.sub.oop = mergee->mark.sub.oop;
		IVM_WBOBJ_SLOT(state, obj, obj->slots);
	}

	return;
}

IVM_INLINE
ivm_function_object_t *
ivm_object_callable(ivm_object_t *obj,
					ivm_vmstate_t *state,
					ivm_object_t **base_p)
{
	if (IVM_IS_BTTYPE(obj, state, IVM_FUNCTION_OBJECT_T)) {
		*base_p = IVM_NULL;
		return IVM_AS(obj, ivm_function_object_t);
	}

	ivm_object_t *bas = IVM_NULL;

	do {
		bas = obj;
		obj = ivm_object_getOop(obj, IVM_OOP_ID(CALL));
		if (!obj) {
			*base_p = IVM_NULL;
			return IVM_AS(obj, ivm_function_object_t);
		}
	} while (!IVM_IS_BTTYPE(obj, state, IVM_FUNCTION_OBJECT_T));

	*base_p = bas;

	return IVM_AS(obj, ivm_function_object_t);
}

IVM_COM_END

#endif
