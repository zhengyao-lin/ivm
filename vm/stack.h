#ifndef _IVM_VM_STACK_H_
#define _IVM_VM_STACK_H_

#include "type.h"
#include "obj.h"

typedef struct {
	ivm_size_t top;
	ivm_size_t size;
	ivm_object_t **st;
} ivm_vmstack_t;

#endif
