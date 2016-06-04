#ifndef _IVM_VM_PROTO_H_
#define _IVM_VM_PROTO_H_

struct ivm_type_t_tag;
struct ivm_vmstate_t_tag;

void
ivm_proto_initType(struct ivm_type_t_tag *type,
				   struct ivm_vmstate_t_tag *state);

#endif
