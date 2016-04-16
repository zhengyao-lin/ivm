#include "pub/mem.h"
#include "obj.h"
#include "err.h"
#include "slot.h"
#include "vm.h"
#include "gc/heap.h"

ivm_object_t *
ivm_object_new(ivm_vmstate_t *state)
{
#if 0
	ivm_object_t *ret = MEM_ALLOC_INIT(sizeof(*ret));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("object"));

	ret->type = IVM_OBJECT_T;
	ret->slots = IVM_NULL;
	ret->des = IVM_NULL;

	return ret;
#endif
	
	return ivm_vmstate_newObject(state);
}

ivm_object_t *ivm_object_newNull(struct ivm_vmstate_t_tag *state)
{
	ivm_object_t *ret = ivm_vmstate_newObject(state);
	
	ret->type = IVM_NULL_T;

	return ret;
}

void
ivm_object_init(ivm_object_t *obj, ivm_vmstate_t *state)
{
	if (obj) {
		obj->type = IVM_OBJECT_T;
		obj->slots = IVM_NULL;
		obj->des = IVM_NULL;
	}

	return;
}

void
ivm_object_free(ivm_object_t *obj,
				ivm_vmstate_t *state)
{
	ivm_vmstate_freeObject(state, obj);
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
		found = ivm_slot_table_addSlot(obj->slots, state, key, value, obj);
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
