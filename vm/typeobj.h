#ifndef _IVM_VM_TYPEOBJ_H_
#define _IVM_VM_TYPEOBJ_H_

#include "pub/com.h"
#include "pub/const.h"
#include "pub/type.h"

#include "obj.h"

IVM_COM_HEADER

struct ivm_vmstate_t_tag;
struct ivm_heap_t_tag;

typedef struct {
	IVM_OBJECT_HEADER
	ivm_type_t *val;
} ivm_type_object_t;

#define ivm_type_object_getValue(obj) (IVM_AS((obj), ivm_type_object_t)->val)

void
ivm_type_object_traverser(ivm_object_t *obj,
						  struct ivm_traverser_arg_t_tag *arg);

void
ivm_type_object_destructor(ivm_object_t *obj,
						   struct ivm_vmstate_t_tag *state);

IVM_COM_END

#endif
