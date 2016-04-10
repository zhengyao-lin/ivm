#include "pub/mem.h"
#include "obj.h"
#include "err.h"
#include "slot.h"
#include "vm.h"
#include "gc/heap.h"

ivm_object_t *
ivm_new_object(ivm_vmstate_t *state)
{
#if 0
	ivm_object_t *ret = MEM_ALLOC_INIT(sizeof(*ret));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("object"));

	ret->type = IVM_OBJECT_T;
	ret->slots = IVM_NULL;
	ret->des = IVM_NULL;

	return ret;
#endif
	return ivm_state_new_object(state);
}

void
ivm_init_object(ivm_vmstate_t *state, ivm_object_t *obj)
{
	if (obj) {
		obj->type = IVM_OBJECT_T;
		obj->slots = IVM_NULL;
		obj->des = IVM_NULL;
	}

	return;
}

void
ivm_free_object(ivm_vmstate_t *state,
				ivm_object_t *obj)
{
	ivm_state_free_object(state, obj);
	return;
}

ivm_slot_t *
ivm_obj_set_slot(ivm_vmstate_t *state,
				 ivm_object_t *obj,
				 const ivm_char_t *key,
				 ivm_object_t *value)
{
	ivm_slot_t *found;

	IVM_ASSERT(obj, IVM_ERROR_MSG_OP_SLOT_OF_UNDEFINED("set"));

	if (!(found = ivm_slot_table_find_slot(state, obj->slots, key))) {
		/* not found */
		if (!obj->slots) {
			obj->slots = ivm_new_slot_table(state);
		}
		found = ivm_slot_table_add_slot(state, obj->slots, key, value, obj);
	} else {
		ivm_slot_set_value(state, found, value);
	}

	return found;
}

ivm_slot_t *
ivm_obj_get_slot(ivm_vmstate_t *state,
				 ivm_object_t *obj,
				 const ivm_char_t *key)
{
	IVM_ASSERT(obj, IVM_ERROR_MSG_OP_SLOT_OF_UNDEFINED("get"));

	return ivm_slot_table_find_slot(state, obj->slots, key);
}
