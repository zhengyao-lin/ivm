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

typedef void (*ivm_destructor_t)(struct ivm_object_t_tag *, struct ivm_vmstate_t_tag *);

typedef struct ivm_object_t_tag {
	IVM_OBJECT_HEADER
	union {
		ivm_numeric_t num;
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
		if (obj->des) { \
			obj->des(obj, state); \
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
