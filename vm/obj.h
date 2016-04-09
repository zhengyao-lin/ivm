#ifndef _IVM_VM_OBJ_H_
#define _IVM_VM_OBJ_H_

#include "pub/mem.h"
#include "type.h"

#define IVM_OBJECT_HEADER \
	ivm_type_t type; \
	ivm_slot_table_t *slots; \
	ivm_destructor_t des;

#define IVM_TYPE_OF(obj) ((obj)->type)

struct ivm_slot_table_t_tag;
struct ivm_object_t_tag;

typedef void (*ivm_destructor_t)(struct ivm_object_t_tag *);

typedef struct ivm_slot_table_t_tag {
	ivm_char_t *k;
	struct ivm_object_t_tag *v;

	struct ivm_slot_table_t_tag *next;
} ivm_slot_table_t;

typedef struct ivm_object_t_tag {
	IVM_OBJECT_HEADER
	union {
		ivm_numeric_t num;
		void *p;
	} u;
} ivm_object_t;

ivm_object_t *ivm_new_obj();
#define ivm_free_obj(obj) \
	if (obj && obj->des) { \
		obj->des(obj); \
	} \
	MEM_FREE(obj);

#endif
