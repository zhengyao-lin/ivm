#ifndef _IVM_VM_OP_H_
#define _IVM_VM_OP_H_

typedef enum {
	IVM_OP_NONE = 0,
	IVM_OP_NEW_OBJ,
	IVM_OP_GET_SLOT,
	IVM_OP_SET_SLOT
} ivm_opcode_t;

#endif
