#ifndef _IVM_VM_NUM_H_
#define _IVM_VM_NUM_H_

#include "pub/com.h"
#include "pub/type.h"

#include "obj.h"

IVM_COM_HEADER

struct ivm_vmstate_t_tag;

#define IVM_NUM(a) ((ivm_number_t)(a))

#define IVM_NUM_ADD(a, b) ((a) + (b))
#define IVM_NUM_SUB(a, b) ((a) - (b))
#define IVM_NUM_MUL(a, b) ((a) * (b))
#define IVM_NUM_DIV(a, b) ((a) / (b))
#define IVM_NUM_MOD(a, b) ((a) % (b))

#define IVM_NUM_SHL(a, b) ((a) << (b))
#define IVM_NUM_SHR(a, b) ((a) >> (b))
#define IVM_NUM_OR(a, b) ((a) | (b))
#define IVM_NUM_AND(a, b) ((a) & (b))
#define IVM_NUM_XOR(a, b) ((a) ^ (b))
#define IVM_NUM_INV(a) (~(a))

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
