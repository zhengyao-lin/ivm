#ifndef _IVM_VM_STROBJ_H_
#define _IVM_VM_STROBJ_H_

#include <string.h>

#include "pub/com.h"
#include "pub/const.h"
#include "pub/type.h"

#include "obj.h"

IVM_COM_HEADER

struct ivm_vmstate_t_tag;
struct ivm_heap_t_tag;

typedef struct {
	IVM_OBJECT_HEADER
	const ivm_string_t *val;
} ivm_string_object_t;

#define ivm_string_object_getValue(obj) (IVM_AS((obj), ivm_string_object_t)->val)

void
ivm_string_object_traverser(ivm_object_t *obj,
							struct ivm_traverser_arg_t_tag *arg);

IVM_COM_END

#endif
