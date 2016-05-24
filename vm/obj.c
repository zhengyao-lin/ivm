#include "pub/const.h"
#include "pub/mem.h"
#include "pub/com.h"
#include "obj.h"
#include "slot.h"
#include "vm.h"
#include "err.h"
#include "gc/gc.h"

ivm_type_t *
ivm_type_new(ivm_type_t type)
{
	ivm_type_t *ret = MEM_ALLOC(sizeof(*ret),
								ivm_type_t *);

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
	obj->mark = IVM_MARK_INIT;
	obj->proto = ivm_vmstate_getTypeProto(state, type);

	return;
}

ivm_bool_t
ivm_object_isTrue(ivm_object_t *obj,
				  ivm_vmstate_t *state)
{
	return obj != IVM_NULL;
}

ivm_bool_t
ivm_object_alwaysTrue(ivm_object_t *obj,
					  ivm_vmstate_t *state)
{
	return IVM_TRUE;
}
ivm_bool_t
ivm_object_alwaysFalse(ivm_object_t *obj,
					   ivm_vmstate_t *state)
{
	return IVM_FALSE;
}

ivm_bool_t
ivm_object_toBool(ivm_object_t *obj,
				  struct ivm_vmstate_t_tag *state)
{
	ivm_bool_converter_t conv = IVM_OBJECT_GET(obj, TYPE_TO_BOOL);

	if (conv)
		return conv(obj, state);

	return IVM_FALSE;
}

#define STR_IS_PROTO(str) \
	str[0] == 'p' && \
	str[1] == 'r' && \
	str[2] == 'o' && \
	str[3] == 't' && \
	str[4] == 'o' && \
	str[5] == '\0'

void
ivm_object_setSlot(ivm_object_t *obj,
				   ivm_vmstate_t *state,
				   const ivm_char_t *key,
				   ivm_object_t *value)
{
	ivm_slot_t *found;

	IVM_ASSERT(obj, IVM_ERROR_MSG_OP_SLOT_OF_UNDEFINED("set"));

	if (STR_IS_PROTO(key)) {
		IVM_OBJECT_SET(obj, PROTO, value);
	} else {
		if (!(found = ivm_slot_table_findSlot(obj->slots, state, key))) {
			/* not found */
			if (!obj->slots) {
				obj->slots = ivm_slot_table_new(state);
			}
			ivm_slot_table_addSlot(obj->slots, state, key, value);
		} else {
			ivm_slot_setValue(found, state, value);
		}
	}

	return;
}

ivm_bool_t
ivm_object_setSlotIfExist(ivm_object_t *obj,
						  ivm_vmstate_t *state,
						  const ivm_char_t *key,
						  ivm_object_t *value)
{
	ivm_slot_t *found;

	IVM_ASSERT(obj, IVM_ERROR_MSG_OP_SLOT_OF_UNDEFINED("set"));

	if (STR_IS_PROTO(key)) {
		IVM_OBJECT_SET(obj, PROTO, value);
	} else if ((found = ivm_slot_table_findSlot(obj->slots, state, key))
			   != IVM_NULL) {
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
						   const ivm_char_t *key)
{
	ivm_object_t *i = IVM_OBJECT_GET(obj, PROTO),
				 *ret = IVM_NULL;

	if (!i) return ret;

	IVM_OBJECT_SET(obj, TRAV_PROTECT, IVM_TRUE);

	while (i && !IVM_OBJECT_GET(i, TRAV_PROTECT)) {
		IVM_OBJECT_SET(i, TRAV_PROTECT, IVM_TRUE);
		ret = ivm_slot_getValue(ivm_slot_table_findSlot(i->slots, state, key),
								state);
		if (ret) {
			break;
		}
		i = IVM_OBJECT_GET(i, PROTO);
	}

	i = obj;
	while (i && IVM_OBJECT_GET(i, TRAV_PROTECT)) {
		IVM_OBJECT_SET(i, TRAV_PROTECT, IVM_FALSE);
		i = IVM_OBJECT_GET(i, PROTO);
	}

	return ret;
}

ivm_object_t *
ivm_object_getSlotValue(ivm_object_t *obj,
						ivm_vmstate_t *state,
						const ivm_char_t *key)
{
	ivm_object_t *ret;

	IVM_ASSERT(obj, IVM_ERROR_MSG_OP_SLOT_OF_UNDEFINED("get"));

	if (STR_IS_PROTO(key)) {
		return IVM_OBJECT_GET(obj, PROTO);
	}

	ret = ivm_slot_getValue(ivm_slot_table_findSlot(obj->slots, state, key),
							state);

	if (!ret) {
		ret = ivm_object_searchProtoSlot(obj, state, key);
	}

	return ret;
}

ivm_object_t *
ivm_object_getSlotValue_np(ivm_object_t *obj,
						   ivm_vmstate_t *state,
						   const ivm_char_t *key)
{
	IVM_ASSERT(obj, IVM_ERROR_MSG_OP_SLOT_OF_UNDEFINED("get"));

	if (STR_IS_PROTO(key)) {
		return IVM_OBJECT_GET(obj, PROTO);
	}

	return ivm_slot_getValue(ivm_slot_table_findSlot(obj->slots, state, key),
							 state);
}

IVM_PRIVATE
void
ivm_object_printSlots_proc(ivm_slot_t *slot, void *arg)
{
	fprintf(stderr, "\tkey %s: %p\n", slot->k, (void *)slot->v);
	return;
}

void
ivm_object_printSlots(ivm_object_t *obj)
{
	fprintf(stderr, "IVM_DEBUG slots in object %p\n", (void *)obj);
	ivm_slot_table_foreach(obj->slots,
						   (ivm_slot_table_foreach_proc_t)ivm_object_printSlots_proc,
						   IVM_NULL);
	return;
}
