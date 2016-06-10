#include "pub/const.h"
#include "pub/mem.h"
#include "pub/com.h"
#include "pub/err.h"
#include "pub/vm.h"

#include "std/string.h"
#include "inline/obj.h"
#include "gc/gc.h"
#include "obj.h"
#include "slot.h"

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
	if (type) {
		ivm_oprt_binary_table_free(type->add_table);
		ivm_oprt_binary_table_free(type->sub_table);
		ivm_oprt_binary_table_free(type->mul_table);
		ivm_oprt_binary_table_free(type->div_table);
		ivm_oprt_binary_table_free(type->mod_table);
		ivm_oprt_binary_table_free(type->cmp_table);
		MEM_FREE(type);
	}

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

	return IVM_OBJECT_GET(obj, TYPE_CONST_BOOL);
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
				   const ivm_string_t *key,
				   ivm_object_t *value)
{
	ivm_slot_t *found;
	ivm_char_t *tmp = ivm_string_trimHead(key);

	IVM_ASSERT(obj, IVM_ERROR_MSG_OP_SLOT_OF_UNDEFINED("set"));

	if (STR_IS_PROTO(tmp)) {
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
						  const ivm_string_t *key,
						  ivm_object_t *value)
{
	ivm_slot_t *found;
	ivm_char_t *tmp = ivm_string_trimHead(key);

	IVM_ASSERT(obj, IVM_ERROR_MSG_OP_SLOT_OF_UNDEFINED("set"));

	if (STR_IS_PROTO(tmp)) {
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
						   const ivm_string_t *key)
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
						const ivm_string_t *key)
{
	ivm_object_t *ret;
	ivm_char_t *tmp = ivm_string_trimHead(key);

	IVM_ASSERT(obj, IVM_ERROR_MSG_OP_SLOT_OF_UNDEFINED("get"));

	if (STR_IS_PROTO(tmp)) {
		return IVM_OBJECT_GET(obj, PROTO);
	}

	ret = ivm_slot_getValue(ivm_slot_table_findSlot(obj->slots, state, key),
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
