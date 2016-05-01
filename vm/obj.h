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
	ivm_mark_t mark; \
	struct ivm_object_t_tag *copy;

#define IVM_TYPE_OF(obj) ((obj)->type)
#define IVM_TYPE_TAG_OF(obj) ((obj)->type->tag)
#define IVM_TYPE_DES_OF(obj) ((obj)->type->des)
#define IVM_TYPE_TRAV_OF(obj) ((obj)->type->trav)
#define IVM_TYPE_SIZE_OF(obj) ((obj)->type->size)

#define IVM_OBJECT_SLOTS(obj) ((obj)->slots)
#define IVM_OBJECT_MARK(obj) ((obj)->mark)

struct ivm_object_t_tag;
struct ivm_vmstate_t_tag;
struct ivm_function_t_tag;
struct ivm_collector_t_tag;
struct ivm_traverser_arg_t_tag;

typedef void (*ivm_destructor_t)(struct ivm_object_t_tag *, struct ivm_vmstate_t_tag *);
typedef void (*ivm_traverser_t)(struct ivm_object_t_tag *, struct ivm_traverser_arg_t_tag *);

typedef struct ivm_type_t_tag {
	ivm_type_tag_t tag;
	ivm_size_t size;

	ivm_destructor_t des;
	ivm_traverser_t trav;
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

ivm_object_t *
ivm_object_new(struct ivm_vmstate_t_tag *state);
ivm_object_t *
ivm_object_newNull(struct ivm_vmstate_t_tag *state);

#define IVM_NULL_OBJ(state) (ivm_object_newNull(state))

void
ivm_object_init(ivm_object_t *obj,
				struct ivm_vmstate_t_tag *state,
				ivm_type_tag_t type);

#define ivm_object_destruct(obj, state) \
	if ((obj) && IVM_TYPE_OF(obj) && IVM_TYPE_DES_OF(obj)) { \
		IVM_TYPE_DES_OF(obj)(obj, state); \
	}
	
#if 0

void
ivm_object_free(ivm_object_t *obj,
				struct ivm_vmstate_t_tag *state);

#endif

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

#define IVM_AS(obj, type) ((type *)(obj))
#define IVM_AS_OBJ(obj) ((ivm_object_t *)(obj))

#endif
