#ifndef _IVM_VM_NUM_H_
#define _IVM_VM_NUM_H_

#include "pub/com.h"
#include "pub/type.h"

#include "obj.h"

IVM_COM_HEADER

struct ivm_vmstate_t_tag;

#define IVM_NUM(a) ((ivm_number_t)(a))

typedef struct ivm_numeric_t_tag {
	IVM_OBJECT_HEADER
	ivm_number_t val;
} ivm_numeric_t;

ivm_bool_t
ivm_numeric_isTrue(ivm_object_t *obj,
				   struct ivm_vmstate_t_tag *state);

#define ivm_numeric_getValue(obj) (IVM_AS((obj), ivm_numeric_t)->val)
#define ivm_numeric_setValue(obj, v) (IVM_AS((obj), ivm_numeric_t)->val = (v))

IVM_COM_END

#endif
