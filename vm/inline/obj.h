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
	obj->slots = IVM_NULL;
	obj->mark = IVM_MARK_INIT;
	obj->proto = ivm_type_getProto(
		obj->type = ivm_vmstate_getType(state, type)
	);

	return;
}

IVM_INLINE
ivm_object_t *
ivm_object_getSlotValue_np(ivm_object_t *obj,
						   ivm_vmstate_t *state,
						   const ivm_string_t *key)
{
	IVM_ASSERT(obj, IVM_ERROR_MSG_OP_SLOT_OF_UNDEFINED("get"));

	return ivm_slot_getValue(
		ivm_slot_table_findSlot(obj->slots, state, key),
		state
	);
}

IVM_INLINE
ivm_object_t *
ivm_object_getSlotValue_np_cc(ivm_object_t *obj,
							  ivm_vmstate_t *state,
							  const ivm_string_t *key,
							  ivm_instr_cache_t *cache)
{
	IVM_ASSERT(obj, IVM_ERROR_MSG_OP_SLOT_OF_UNDEFINED("get"));

	return ivm_slot_getValue(
		ivm_slot_table_findSlot_cc(obj->slots, state, key, cache),
		state
	);
}

#undef STR_IS_PROTO

IVM_COM_END

#endif
