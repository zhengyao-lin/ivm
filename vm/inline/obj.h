#ifndef _IVM_VM_INLINE_OBJ_H_
#define _IVM_VM_INLINE_OBJ_H_

#include "pub/com.h"
#include "pub/vm.h"

#include "std/string.h"

#include "../instr.h"
#include "../obj.h"
#include "../slot.h"

IVM_COM_HEADER

IVM_INLINE
void
ivm_object_init(ivm_object_t *obj,
				ivm_vmstate_t *state,
				ivm_type_tag_t type)
{
	MEM_INIT(&obj->slots, sizeof(obj->slots) + sizeof(obj->mark));
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

	MEM_COPY(ret, obj, size);
	ivm_slot_table_copyShared(ret->slots);

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

	return ret;
}

IVM_INLINE
ivm_object_t *
ivm_object_newNull(ivm_vmstate_t *state)
{
	ivm_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(ret, state, IVM_NULL_T);

	return ret;
}

IVM_INLINE
ivm_object_t *
ivm_object_newUndefined(ivm_vmstate_t *state)
{
	ivm_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(ret, state, IVM_UNDEFINED_T);

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

#define IVM_NULL_OBJ(state) (ivm_object_newNull(state))
#define IVM_UNDEFINED(state) (ivm_object_newUndefined(state))


IVM_INLINE
void
ivm_object_setOop(ivm_object_t *obj,
				  ivm_vmstate_t *state,
				  ivm_int_t op,
				  ivm_object_t *func)
{
	ivm_slot_table_t *slots = obj->slots;

	if (slots) {
		if (ivm_slot_table_isShared(slots)) {
			slots = obj->slots = ivm_slot_table_copyOnWrite(slots, state);
			IVM_WBOBJ_SLOT(state, obj, slots);
		}
	} else {
		slots = obj->slots = ivm_slot_table_newAt(state, IVM_OBJECT_GET(obj, GEN));
	}

	ivm_object_markOop(obj);
	ivm_slot_table_setOop(slots, state, op, func);

	return;
}

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

IVM_COM_END

#endif
