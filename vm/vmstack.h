#ifndef _IVM_VM_VMSTACK_H_
#define _IVM_VM_VMSTACK_H_

#include "pub/com.h"
#include "pub/const.h"
#include "pub/type.h"
#include "pub/mem.h"
#include "pub/err.h"

IVM_COM_HEADER

struct ivm_object_t_tag;
struct ivm_frame_stack_t_tag;
struct ivm_coro_t_tag;

typedef struct {
	ivm_size_t size;
	struct ivm_object_t_tag **edge;
	struct ivm_object_t_tag **bottom;
} ivm_vmstack_t;

ivm_vmstack_t *
ivm_vmstack_new();

void
ivm_vmstack_free(ivm_vmstack_t *stack);

void
ivm_vmstack_init(ivm_vmstack_t *stack);

void
ivm_vmstack_dump(ivm_vmstack_t *stack);

void
ivm_vmstack_inc(ivm_vmstack_t *stack,
				struct ivm_frame_stack_t_tag *fstack);

void
ivm_vmstack_inc_c(ivm_vmstack_t *stack,
				  struct ivm_coro_t_tag *coro);

void
ivm_vmstack_push(struct ivm_coro_t_tag *coro,
				 struct ivm_object_t_tag *obj);

#define ivm_vmstack_bottom(stack) ((stack)->bottom)
#define ivm_vmstack_edge(stack) ((stack)->edge)
#define ivm_vmstack_offset(stack, ptr) \
	(((ivm_ptr_t)(ptr) - (ivm_ptr_t)(stack)->bottom) / sizeof(*ptr))

#define ivm_vmstack_at(stack, i) ((stack)->bottom[i])
#define ivm_vmstack_ptrAt(stack, i) ((stack)->bottom + (i))

#if 0
#define ivm_vmstack_new() (ivm_stack_new_c(IVM_DEFAULT_VMSTACK_BUFFER_SIZE))
#define ivm_vmstack_free ivm_stack_free
#define ivm_vmstack_pushAt ivm_stack_pushAt
#define ivm_vmstack_inc ivm_stack_inc
#define ivm_vmstack_bottom(stack) ((struct ivm_object_t_tag **)ivm_stack_core(stack))
#define ivm_vmstack_edge(stack) ((struct ivm_object_t_tag **)ivm_stack_end(stack))
#define ivm_vmstack_offset(stack, ptr) (ivm_stack_offset((stack), (ptr)))
#define ivm_vmstack_at(stack, i) ((struct ivm_object_t_tag *)ivm_stack_at((stack), (i)))
#define ivm_vmstack_ptrAt(stack, i) ((struct ivm_object_t_tag **)ivm_stack_ptrAt((stack), (i)))
#define ivm_vmstack_cut(stack, i) ((struct ivm_object_t_tag **)ivm_stack_cut((stack), (i)))
#endif

IVM_COM_END

#endif
