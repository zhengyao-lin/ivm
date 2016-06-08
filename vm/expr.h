#ifndef _IVM_VM_EXPR_H_
#define _IVM_VM_EXPR_H_

#include "pub/const.h"

#include "std/list.h"

IVM_COM_HEADER

struct ivm_type_t_tag;
struct ivm_object_t_tag;
struct ivm_vmstate_t_tag;

typedef struct ivm_object_t_tag *(*ivm_binary_op_proc_t)(struct ivm_vmstate_t_tag *state,
														 struct ivm_object_t_tag *op1,
														 struct ivm_object_t_tag *op2);

typedef ivm_ptlist_t ivm_binary_op_proc_list_t;

#define ivm_binary_op_proc_list_new() (ivm_ptlist_new_c(IVM_DEFAULT_BINARY_OP_PROC_LIST_BUFFER_SIZE))
#define ivm_binary_op_proc_list_free ivm_ptlist_free
#define ivm_binary_op_proc_list_at(list, i) \
	((ivm_binary_op_proc_t)((list) && ivm_ptlist_has((list), (i)) ? ivm_ptlist_at((list), (i)) : IVM_NULL))
#define ivm_binary_op_proc_list_set(list, i, val) (ivm_ptlist_set((list), (i), (void *)(val)))

void
ivm_binary_op_initType(struct ivm_type_t_tag *type,
					   struct ivm_vmstate_t_tag *state);

IVM_COM_END

#endif
