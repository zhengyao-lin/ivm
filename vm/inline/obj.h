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
				ivm_vmstate_t *state,
				ivm_type_tag_t type)
{
	STD_INIT(&obj->slots, sizeof(obj->slots) + sizeof(obj->mark));
	obj->proto = ivm_type_getProto(
		obj->type = ivm_vmstate_getType(state, type)
	);

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

	ivm_object_init(ret, state, IVM_OBJECT_T);

	return ret;
}

IVM_INLINE
ivm_object_t *
ivm_object_new_c(struct ivm_vmstate_t_tag *state,
				 ivm_size_t prealloc)
{
	ivm_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(ret, state, IVM_OBJECT_T);
	ret->slots = ivm_slot_table_new_c(state, prealloc);

	return ret;
}

IVM_INLINE
ivm_object_t *
ivm_object_new_t(struct ivm_vmstate_t_tag *state,
				 ivm_slot_table_t *slots)
{
	ivm_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(ret, state, IVM_OBJECT_T);
	ret->slots = slots;
	if (slots) {
		ivm_slot_table_setLinked(slots);
		ret->mark.sub.oop = IVM_TRUE; // ivm_slot_table_hasOop(slots);
	}

	return ret;
}

IVM_INLINE
ivm_object_t *
ivm_none_new(ivm_vmstate_t *state)
{
	ivm_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(ret, state, IVM_NONE_T);
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
		if (ivm_object_hasOop(obj)) {
			tmp = ivm_slot_table_getOop(obj->slots, op);
			if (tmp) return tmp;
		}
		obj = obj->proto;
	} while (obj);

	return IVM_NULL;
}

IVM_INLINE
void
ivm_object_merge(ivm_object_t *obj,
				 ivm_vmstate_t *state,
				 ivm_object_t *mergee,
				 ivm_bool_t overw)
{
	if (obj->slots) {
		if (mergee->slots) {
			ivm_slot_table_copyOnWrite(obj->slots, state);
			ivm_slot_table_merge(obj->slots, state, mergee->slots, overw);
			obj->mark.sub.oop |= mergee->mark.sub.oop;
		}
	} else if (mergee->slots) {
		obj->slots = ivm_slot_table_copy_state(mergee->slots, state);
		obj->mark.sub.oop = mergee->mark.sub.oop;
	}

	return;
}

IVM_COM_END

#endif
