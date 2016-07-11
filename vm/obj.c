#include "pub/const.h"
#include "pub/mem.h"
#include "pub/com.h"
#include "pub/err.h"
#include "pub/vm.h"
#include "pub/inlines.h"

#include "std/string.h"

#include "gc/gc.h"
#include "obj.h"
#include "instr.h"
#include "slot.h"

ivm_type_t *
ivm_type_new(ivm_type_t type)
{
	ivm_type_t *ret = MEM_ALLOC(sizeof(*ret),
								ivm_type_t *);
	ivm_binop_table_t *i, *end;

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("type"));

	MEM_COPY(ret, &type, sizeof(*ret));

	for (i = ret->binops, end = i + IVM_ARRLEN(ret->binops);
		 i != end; i++) {
		ivm_binop_table_init(i);
	}

	ivm_uniop_table_init(ret->uniops);

	return ret;
}

void
ivm_type_free(ivm_type_t *type)
{
	ivm_int_t i;

	if (type) {
		for (i = 0; i < IVM_BINOP_COUNT; i++) {
			ivm_binop_table_dump(type->binops + i);
		}

		MEM_FREE(type);
	}

	return;
}

ivm_bool_t
ivm_object_toBool(ivm_object_t *obj,
				  ivm_vmstate_t *state)
{
	ivm_type_t *type = IVM_TYPE_OF(obj);
	ivm_bool_converter_t conv = type->to_bool;

	if (conv)
		return conv(obj, state);

	return type->const_bool;
}

void
ivm_object_setSlot(ivm_object_t *obj,
				   ivm_vmstate_t *state,
				   const ivm_string_t *key,
				   ivm_object_t *value)
{
	ivm_slot_table_t *slots;
	ivm_slot_t *found;

	IVM_ASSERT(obj, IVM_ERROR_MSG_OP_SLOT_OF_UNDEFINED("set"));

	slots = obj->slots;

	if (slots) {
		if (ivm_slot_table_isShared(slots)) {
			slots = obj->slots = ivm_slot_table_copyOnWrite(slots, state);
		}

		found = ivm_slot_table_findSlot(slots, state, key);
		if (found) {
			ivm_slot_setValue(found, state, value);
		} else {
			ivm_slot_table_addSlot(slots, state, key, value);
		}
	} else {
		slots = obj->slots = ivm_slot_table_new(state);
		ivm_slot_table_addSlot(slots, state, key, value);
	}

	return;
}

void
ivm_object_setSlot_r(ivm_object_t *obj,
					 ivm_vmstate_t *state,
					 const ivm_char_t *rkey,
					 ivm_object_t *value)
{
	ivm_slot_table_t *slots;
	ivm_slot_t *found;
	const ivm_string_t *key;

	IVM_ASSERT(obj, IVM_ERROR_MSG_OP_SLOT_OF_UNDEFINED("set"));

	key = (const ivm_string_t *)
		  ivm_string_pool_registerRaw(IVM_VMSTATE_GET(state, CONST_POOL), rkey);

	slots = obj->slots;

	if (slots) {
		if (ivm_slot_table_isShared(slots)) {
			slots = obj->slots = ivm_slot_table_copyOnWrite(slots, state);
		}

		found = ivm_slot_table_findSlot(slots, state, key);
		if (found) {
			ivm_slot_setValue(found, state, value);
		} else {
			/* not found */
			ivm_slot_table_addSlot(slots, state, key, value);
		}
	} else {
		slots = obj->slots = ivm_slot_table_new(state);
		ivm_slot_table_addSlot(slots, state, key, value);
	}

	return;
}

void
ivm_object_setSlot_cc(ivm_object_t *obj,
					  struct ivm_vmstate_t_tag *state,
					  const ivm_string_t *key,
					  ivm_object_t *value,
					  ivm_instr_cache_t *cache)
{
	ivm_slot_table_t *slots;
	ivm_slot_t *found;

	IVM_ASSERT(obj, IVM_ERROR_MSG_OP_SLOT_OF_UNDEFINED("set"));

	slots = obj->slots;

	if (slots) {
		if (ivm_slot_table_isShared(slots)) {
			slots = obj->slots = ivm_slot_table_copyOnWrite(slots, state);
		}

		found = ivm_slot_table_findSlot_cc(slots, state, key, cache);
		if (found) {
			ivm_slot_setValue(found, state, value);
		} else {
			/* not found */
			ivm_slot_table_addSlot_cc(slots, state, key, value, cache);
		}
	} else {
		slots = obj->slots = ivm_slot_table_new(state);
		ivm_slot_table_addSlot(slots, state, key, value);
	}

	return;
}

ivm_bool_t
ivm_object_setSlotIfExist(ivm_object_t *obj,
						  ivm_vmstate_t *state,
						  const ivm_string_t *key,
						  ivm_object_t *value)
{
	ivm_slot_t *found;

	IVM_ASSERT(obj, IVM_ERROR_MSG_OP_SLOT_OF_UNDEFINED("set"));

	found = ivm_slot_table_findSlot(obj->slots, state, key);

	if (found) {
		ivm_slot_setValue(found, state, value);
	} else {
		return IVM_FALSE;
	}

	return IVM_TRUE;
}

ivm_bool_t
ivm_object_setSlotIfExist_cc(ivm_object_t *obj,
							 ivm_vmstate_t *state,
							 const ivm_string_t *key,
							 ivm_object_t *value,
							 ivm_instr_cache_t *cache)
{
	ivm_slot_t *found;
	IVM_ASSERT(obj, IVM_ERROR_MSG_OP_SLOT_OF_UNDEFINED("set"));

	found = ivm_slot_table_findSlot_cc(obj->slots, state, key, cache);

	if (found) {
		ivm_slot_setValue(found, state, value);
	} else {
		return IVM_FALSE;
	}

	return IVM_TRUE;
}

IVM_PRIVATE
ivm_object_t *
ivm_object_searchProtoSlot(ivm_object_t *obj,
						   ivm_vmstate_t *state,
						   const ivm_string_t *key)
{
	ivm_object_t *i = IVM_OBJECT_GET(obj, PROTO),
				 *ret = IVM_NULL;

	if (!i) return ret;

	// IVM_OBJECT_SET(obj, TRAV_PROTECT, IVM_TRUE);

	/* no loop is allowed when setting proto */
	while (i) {
		ret = ivm_slot_getValue(ivm_slot_table_findSlot(i->slots, state, key),
								state);
		if (ret) break;
		i = IVM_OBJECT_GET(i, PROTO);
	}

#if 0
	i = obj;
	while (i && IVM_OBJECT_GET(i, TRAV_PROTECT)) {
		IVM_OBJECT_SET(i, TRAV_PROTECT, IVM_FALSE);
		i = IVM_OBJECT_GET(i, PROTO);
	}
#endif

	return ret;
}

ivm_object_t *
ivm_object_getSlotValue(ivm_object_t *obj,
						ivm_vmstate_t *state,
						const ivm_string_t *key)
{
	ivm_object_t *ret;

	IVM_ASSERT(obj, IVM_ERROR_MSG_OP_SLOT_OF_UNDEFINED("get"));

	ret = ivm_slot_getValue(ivm_slot_table_findSlot(obj->slots, state, key),
							state);

	if (!ret) {
		ret = ivm_object_searchProtoSlot(obj, state, key);
	}

	return ret;
}

ivm_object_t *
ivm_object_getSlotValue_cc(ivm_object_t *obj,
						   ivm_vmstate_t *state,
						   const ivm_string_t *key,
						   ivm_instr_cache_t *cache)
{
	ivm_object_t *ret;
	
	IVM_ASSERT(obj, IVM_ERROR_MSG_OP_SLOT_OF_UNDEFINED("get"));

	ret = ivm_slot_getValue(ivm_slot_table_findSlot_cc(obj->slots, state, key, cache),
							state);

	if (!ret) {
		ret = ivm_object_searchProtoSlot(obj, state, key);
	}

	return ret;
}

void
ivm_object_printSlots(ivm_object_t *obj)
{
	ivm_slot_table_iterator_t iter;
	ivm_size_t ecount = 0; /* empty count */

	IVM_TRACE("IVM_DEBUG slots in object %p(slot table using %s)\n",
			  (void *)obj, obj->slots && obj->slots->is_hash ? "hash table" : "list");

	if (obj->slots) {
		IVM_SLOT_TABLE_EACHPTR(obj->slots, iter) {
			if (IVM_SLOT_TABLE_ITER_GET_KEY(iter)) {
				IVM_TRACE("\tkey %s: %p\n",
						  ivm_string_trimHead(IVM_SLOT_TABLE_ITER_GET_KEY(iter)),
						  (void *)IVM_SLOT_TABLE_ITER_GET_VAL(iter));
			} else {
				ecount++;
			}
		}

		if (obj->slots->is_hash) {
			IVM_TRACE("\thash table load factor: %f\n",
					  (double)ecount / obj->slots->size);
		}
	} else {
		IVM_TRACE("\tnone\n");
	}

	return;
}
