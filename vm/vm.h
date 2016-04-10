#ifndef _IVM_VM_VM_H_
#define _IVM_VM_VM_H_

#include "obj.h"
#include "num.h"
#include "op.h"

typedef struct ivm_vmstate_t_tag {
	int placeholder;
} ivm_vmstate_t;

ivm_vmstate_t *
ivm_new_state();
void
ivm_free_state(ivm_vmstate_t *state);

#endif
