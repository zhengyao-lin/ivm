#ifndef _IVM_VM_NUM_H_
#define _IVM_VM_NUM_H_

#include "type.h"
#include "obj.h"

#define IVM_NUM(a) ((ivm_numeric_t)(a))

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

ivm_object_t *ivm_new_num(ivm_numeric_t val);

#endif
