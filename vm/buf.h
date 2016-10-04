#ifndef _IVM_VM_BUF_H_
#define _IVM_VM_BUF_H_

#include "pub/com.h"
#include "pub/type.h"

#include "obj.h"

IVM_COM_HEADER

struct ivm_vmstate_t_tag;

typedef struct {
	IVM_OBJECT_HEADER

	ivm_size_t alloc;
	ivm_size_t size;
	ivm_byte_t *buf;
} ivm_buffer_object_t;

ivm_object_t *
ivm_buffer_object_new(struct ivm_vmstate_t_tag *state,
					  ivm_size_t init);

ivm_object_t *
ivm_buffer_object_new_c(struct ivm_vmstate_t_tag *state,
						ivm_size_t init, ivm_byte_t *buf);

IVM_INLINE
ivm_size_t
ivm_buffer_object_getSize(ivm_buffer_object_t *buf)
{
	return buf->size;
}

IVM_INLINE
ivm_byte_t *
ivm_buffer_object_getRaw(ivm_buffer_object_t *buf)
{
	return buf->buf;
}

void
ivm_buffer_object_destructor(ivm_object_t *obj,
							 struct ivm_vmstate_t_tag *state);

IVM_COM_END

#endif
