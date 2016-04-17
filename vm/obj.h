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

#define IVM_TYPE_OF(obj) ((obj)->type)
#define IVM_TYPE_TAG_OF(obj) ((obj)->type->tag)
#define IVM_TYPE_DES_OF(obj) ((obj)->type->des)
#define IVM_TYPE_MARKER_OF(obj) ((obj)->type->marker)

struct ivm_object_t_tag;
struct ivm_vmstate_t_tag;
struct ivm_function_t_tag;

typedef void (*ivm_destructor_t)(struct ivm_object_t_tag *, struct ivm_vmstate_t_tag *);
typedef void (*ivm_marker_t)(struct ivm_object_t_tag *, struct ivm_vmstate_t_tag *);

typedef struct ivm_type_t_tag {
	ivm_type_tag_t tag;
	ivm_destructor_t des;
	ivm_marker_t marker;
} ivm_type_t;

ivm_type_t *
ivm_type_new(ivm_type_tag_t tag, ivm_destructor_t des, ivm_marker_t marker);
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
	union {
		ivm_numeric_t num;
		struct ivm_function_t_tag *func;
		void *p;
	} u;
} ivm_object_t;

ivm_object_t *ivm_object_new(struct ivm_vmstate_t_tag *state);
ivm_object_t *ivm_object_newNull(struct ivm_vmstate_t_tag *state);

#define IVM_NULL_OBJ(state) (ivm_object_newNull(state))

void ivm_object_init(ivm_object_t *obj, struct ivm_vmstate_t_tag *state);

/* dump: clean the data the object contains, but not free itself */
#define ivm_object_dump(obj, state) \
	if (obj) { \
		if (IVM_TYPE_OF(obj) && IVM_TYPE_DES_OF(obj)) { \
			IVM_TYPE_DES_OF(obj)(obj, state); \
		} \
		ivm_slot_table_free(obj->slots, state); \
	}
	
void ivm_object_free(ivm_object_t *obj, struct ivm_vmstate_t_tag *state);

ivm_slot_t *
ivm_object_setSlot(ivm_object_t *obj, struct ivm_vmstate_t_tag *state, const ivm_char_t *key, ivm_object_t *value);
ivm_slot_t *
ivm_object_getSlot(ivm_object_t *obj, struct ivm_vmstate_t_tag *state, const ivm_char_t *key);

#define ivm_object_getSlotValue(obj, state, key) (ivm_slot_getValue(ivm_object_getSlot((obj), (state), (key)), (state)))

#endif
