#include "pub/mem.h"
#include "obj.h"
#include "slot.h"
#include "vm.h"
#include "err.h"
#include "gc/gc.h"

#if 0

ivm_type_t *
ivm_type_new(ivm_type_tag_t tag, ivm_destructor_t des, ivm_marker_t marker)
{
	ivm_type_t *ret = MEM_ALLOC(sizeof(*ret));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("type"));

	ret->tag = tag;
	ret->des = des;
	ret->marker = marker;

	return ret;
}

#endif

ivm_type_t *
ivm_type_new(ivm_type_t type)
{
	ivm_type_t *ret = MEM_ALLOC(sizeof(*ret));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("type"));

	MEM_COPY(ret, &type, sizeof(*ret));

	return ret;
}

void
ivm_type_free(ivm_type_t *type)
{
	MEM_FREE(type);
	return;
}

ivm_object_t *
ivm_object_new(ivm_vmstate_t *state)
{
	ivm_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(ret, state, IVM_OBJECT_T);

	return ret;
}

ivm_object_t *
ivm_object_newNull(ivm_vmstate_t *state)
{
	ivm_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(ret, state, IVM_NULL_T);

	return ret;
}

ivm_object_t *
ivm_object_newUndefined(ivm_vmstate_t *state)
{
	ivm_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(ret, state, IVM_UNDEFINED_T);

	return ret;
}

void
ivm_object_init(ivm_object_t *obj,
				ivm_vmstate_t *state,
				ivm_type_tag_t type)
{
	obj->type = ivm_vmstate_getType(state, type);
	obj->slots = IVM_NULL;
	obj->mark = IVM_MARK_WHITE;
	obj->copy = IVM_NULL;

	return;
}

ivm_slot_t *
ivm_object_setSlot(ivm_object_t *obj,
				   ivm_vmstate_t *state,
				   const ivm_char_t *key,
				   ivm_object_t *value)
{
	ivm_slot_t *found;

	IVM_ASSERT(obj, IVM_ERROR_MSG_OP_SLOT_OF_UNDEFINED("set"));

	if (!(found = ivm_slot_table_findSlot(obj->slots, state, key))) {
		/* not found */
		if (!obj->slots) {
			obj->slots = ivm_slot_table_new(state);
		}
		found = ivm_slot_table_addSlot(obj->slots, state, key, value);
	} else {
		ivm_slot_setValue(found, state, value);
	}

	return found;
}

ivm_slot_t *
ivm_object_getSlot(ivm_object_t *obj,
				   ivm_vmstate_t *state,
				   const ivm_char_t *key)
{
	IVM_ASSERT(obj, IVM_ERROR_MSG_OP_SLOT_OF_UNDEFINED("get"));

	return ivm_slot_table_findSlot(obj->slots, state, key);
}
