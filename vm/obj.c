#include "pub/mem.h"
#include "obj.h"
#include "err.h"
#include "slot.h"

ivm_object_t *ivm_new_obj()
{
	ivm_object_t *ret = MEM_ALLOC_INIT(sizeof(*ret));

	IVM_ASSERT(ret, "Failed to allocate new room for new object");

	ret->type = IVM_OBJECT_T;
	ret->slots = IVM_NULL;
	ret->des = IVM_NULL;

	return ret;
}

ivm_slot_t *
ivm_obj_set_slot(ivm_object_t *obj,
				 const ivm_char_t *key,
				 ivm_object_t *value)
{
	ivm_slot_t *found;

	IVM_ASSERT(obj, "Set slot of undefined object");

	if (!(found = ivm_slot_find_slot(obj->slots, key))) {
		/* not found */
		if (!obj->slots) {
			obj->slots = ivm_new_slot_table();
		}
		found = ivm_slot_add_slot(obj->slots, key, value);
	} else {
		ivm_slot_set_value(found, value);
	}

	return found;
}

ivm_slot_t *
ivm_obj_get_slot(ivm_object_t *obj, const ivm_char_t *key)
{
	IVM_ASSERT(obj, "Get slot of undefined object");

	return ivm_slot_find_slot(obj->slots, key);
}
