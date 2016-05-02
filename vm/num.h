#ifndef _IVM_VM_NUM_H_
#define _IVM_VM_NUM_H_

#include "type.h"
#include "obj.h"

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

ivm_object_t *ivm_numeric_new(struct ivm_vmstate_t_tag *state, ivm_number_t val);

#endif
