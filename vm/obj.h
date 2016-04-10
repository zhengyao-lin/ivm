#ifndef _IVM_VM_OBJ_H_
#define _IVM_VM_OBJ_H_

#include "pub/mem.h"
#include "type.h"
#include "slot.h"

#define IVM_OBJECT_HEADER \
	ivm_type_t type; \
	ivm_slot_table_t *slots; \
	ivm_destructor_t des;

#define IVM_TYPE_OF(obj) ((obj)->type)

struct ivm_object_t_tag;
struct ivm_vmstate_t_tag;

typedef void (*ivm_destructor_t)(struct ivm_vmstate_t_tag *, struct ivm_object_t_tag *);

typedef struct ivm_object_t_tag {
	IVM_OBJECT_HEADER
	union {
		ivm_numeric_t num;
		void *p;
	} u;
} ivm_object_t;

ivm_object_t *ivm_new_object(struct ivm_vmstate_t_tag *state);
void ivm_init_object(struct ivm_vmstate_t_tag *state, ivm_object_t *obj);

/* dump: clean the data the object contains, but not free itself */
#define ivm_dump_object(state, obj) \
	if (obj) { \
		if (obj->des) { \
			obj->des(state, obj); \
		} \
		ivm_free_slot_table(state, obj->slots); \
	}
	
void ivm_free_object(struct ivm_vmstate_t_tag *state, ivm_object_t *obj);

ivm_slot_t *
ivm_obj_set_slot(struct ivm_vmstate_t_tag *state, ivm_object_t *obj, const ivm_char_t *key, ivm_object_t *value);
ivm_slot_t *
ivm_obj_get_slot(struct ivm_vmstate_t_tag *state, ivm_object_t *obj, const ivm_char_t *key);

#define ivm_obj_get_slot_value(state, obj, key) (ivm_slot_get_value((state), ivm_obj_get_slot((state), (obj), (key))))

#endif
