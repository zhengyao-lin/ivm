#ifndef _IVM_VM_OBJ_H_
#define _IVM_VM_OBJ_H_

#include "pub/mem.h"
#include "pub/const.h"
#include "std/list.h"
#include "type.h"
#include "slot.h"

#if IVM_DEBUG

#define IVM_DEFAULT_TYPE_LIST_BUFFER_SIZE 1

#else

#define IVM_DEFAULT_TYPE_LIST_BUFFER_SIZE 8

#endif

#define IVM_OBJECT_HEADER \
	ivm_type_t *type; \
	ivm_slot_table_t *slots; \
	ivm_mark_t mark;

struct ivm_object_t_tag;
struct ivm_vmstate_t_tag;
struct ivm_function_t_tag;
struct ivm_collector_t_tag;
struct ivm_traverser_arg_t_tag;

typedef void (*ivm_destructor_t)(struct ivm_object_t_tag *, struct ivm_vmstate_t_tag *);
typedef void (*ivm_traverser_t)(struct ivm_object_t_tag *, struct ivm_traverser_arg_t_tag *);
typedef ivm_bool_t (*ivm_bool_converter_t)(struct ivm_object_t_tag *, struct ivm_vmstate_t_tag *);

typedef struct ivm_type_t_tag {
	ivm_type_tag_t tag;
	const ivm_char_t *name;
	ivm_size_t size;

	ivm_destructor_t des;
	ivm_traverser_t trav;
	ivm_bool_converter_t to_bool;
} ivm_type_t;

ivm_type_t *
ivm_type_new(ivm_type_t type);
void
ivm_type_free(ivm_type_t *type);
#define ivm_type_setTag(type, t) ((type)->tag = (t))

typedef ivm_ptlist_t ivm_type_list_t;

#define ivm_type_list_new() (ivm_ptlist_new_c(IVM_DEFAULT_TYPE_LIST_BUFFER_SIZE))
#define ivm_type_list_free ivm_ptlist_free
#define ivm_type_list_register ivm_ptlist_push
#define ivm_type_list_size ivm_ptlist_size
#define ivm_type_list_at(list, i) ((ivm_type_t *)ivm_ptlist_at((list), (i)))
#define ivm_type_list_foreach(list, each) (ivm_ptlist_foreach((list), (ivm_ptlist_foreach_proc_t)(each)))

typedef struct ivm_object_t_tag {
	IVM_OBJECT_HEADER
} ivm_object_t;

#define IVM_TYPE_OF(obj) ((obj)->type)
#define IVM_OBJECT_GET_TYPE_TAG(obj) ((obj)->type->tag)
#define IVM_OBJECT_GET_TYPE_NAME(obj) ((obj)->type->name)
#define IVM_OBJECT_GET_TYPE_SIZE(obj) ((obj)->type->size)
#define IVM_OBJECT_GET_TYPE_DES(obj) ((obj)->type->des)
#define IVM_OBJECT_GET_TYPE_TRAV(obj) ((obj)->type->trav)
#define IVM_OBJECT_GET_TYPE_TO_BOOL(obj) ((obj)->type->to_bool)
#define IVM_OBJECT_GET_SLOTS(obj) ((obj)->slots)
#define IVM_OBJECT_GET_MARK(obj) ((obj)->mark)

#define IVM_OBJECT_SET_SLOTS(obj, val) ((obj)->slots = (val))
#define IVM_OBJECT_SET_MARK(obj, val) ((obj)->mark = (val))

#define IVM_OBJECT_GET(obj, member) IVM_GET((obj), IVM_OBJECT, member)
#define IVM_OBJECT_SET(obj, member, val) IVM_SET((obj), IVM_OBJECT, member, (val))

ivm_object_t *
ivm_object_new(struct ivm_vmstate_t_tag *state);
ivm_object_t *
ivm_object_newNull(struct ivm_vmstate_t_tag *state);
ivm_object_t *
ivm_object_newUndefined(struct ivm_vmstate_t_tag *state);

#define IVM_NULL_OBJ(state) (ivm_object_newNull(state))
#define IVM_UNDEFINED(state) (ivm_object_newUndefined(state))

void
ivm_object_init(ivm_object_t *obj,
				struct ivm_vmstate_t_tag *state,
				ivm_type_tag_t type);

#define ivm_object_destruct(obj, state) \
	if ((obj) && IVM_TYPE_OF(obj) && IVM_OBJECT_GET(obj, TYPE_DES)) { \
		IVM_OBJECT_GET(obj, TYPE_DES)((obj), (state)); \
	}
	
#if 0

void
ivm_object_free(ivm_object_t *obj,
				struct ivm_vmstate_t_tag *state);

#endif

ivm_bool_t
ivm_object_isTrue(ivm_object_t *obj,
				  struct ivm_vmstate_t_tag *state);
ivm_bool_t
ivm_object_alwaysTrue(ivm_object_t *obj,
					  struct ivm_vmstate_t_tag *state);
ivm_bool_t
ivm_object_alwaysFalse(ivm_object_t *obj,
					   struct ivm_vmstate_t_tag *state);

ivm_bool_t
ivm_object_toBool(ivm_object_t *obj,
				  struct ivm_vmstate_t_tag *state);

ivm_slot_t *
ivm_object_setSlot(ivm_object_t *obj,
				   struct ivm_vmstate_t_tag *state,
				   const ivm_char_t *key,
				   ivm_object_t *value);
ivm_slot_t *
ivm_object_getSlot(ivm_object_t *obj,
				   struct ivm_vmstate_t_tag *state,
				   const ivm_char_t *key);

#define ivm_object_getSlotValue(obj, state, key) (ivm_slot_getValue(ivm_object_getSlot((obj), (state), (key)), (state)))

#if IVM_DEBUG

void
ivm_object_printSlots(ivm_object_t *obj);

#endif

#define IVM_AS(obj, type) ((type *)(obj))
#define IVM_AS_OBJ(obj) ((ivm_object_t *)(obj))

#endif
