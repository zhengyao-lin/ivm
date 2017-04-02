#include "pub/type.h"
#include "pub/vm.h"
#include "pub/inlines.h"

#include "std/mem.h"

#include "buf.h"

ivm_object_t *
ivm_buffer_object_new(ivm_vmstate_t *state,
					  ivm_size_t init)
{
	ivm_buffer_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), IVM_BTTYPE(state, IVM_BUFFER_OBJECT_T));

	ret->alloc = ret->size = init;

	IVM_ASSERT(init, IVM_ERROR_MSG_EMPTY_BUFFER);

	ret->buf = ivm_vmstate_allocWild(state, sizeof(*ret->buf) * init);

	ivm_vmstate_addDesLog(state, IVM_AS_OBJ(ret));

	return IVM_AS_OBJ(ret);
}

ivm_object_t *
ivm_buffer_object_new_c(ivm_vmstate_t *state,
						ivm_size_t init,
						ivm_byte_t *buf)
{
	ivm_buffer_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), IVM_BTTYPE(state, IVM_BUFFER_OBJECT_T));

	ret->alloc = ret->size = init;
	ret->buf = buf;

	ivm_vmstate_addDesLog(state, IVM_AS_OBJ(ret));

	return IVM_AS_OBJ(ret);
}

ivm_object_t *
ivm_buffer_object_newCopy(ivm_vmstate_t *state,
						  ivm_size_t init,
						  ivm_byte_t *buf)
{
	ivm_buffer_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), IVM_BTTYPE(state, IVM_BUFFER_OBJECT_T));

	ret->alloc = ret->size = init;
	ret->buf = ivm_vmstate_allocWild(state, sizeof(*buf) * init);
	STD_MEMCPY(ret->buf, buf, sizeof(*buf) * init);

	ivm_vmstate_addDesLog(state, IVM_AS_OBJ(ret));

	return IVM_AS_OBJ(ret);
}

void
ivm_buffer_object_destructor(ivm_object_t *obj,
							 ivm_vmstate_t *state)
{
	STD_FREE(IVM_AS(obj, ivm_buffer_object_t)->buf);
	return;
}

void
ivm_buffer_object_cloner(ivm_object_t *obj,
						 ivm_vmstate_t *state)
{
	ivm_buffer_object_t *buf = IVM_AS(obj, ivm_buffer_object_t);
	ivm_byte_t *orig = buf->buf;
	ivm_size_t size = sizeof(*buf->buf) * buf->size;

	buf->buf = ivm_vmstate_allocWild(state, size);
	if (!buf->buf) {
		buf->alloc = buf->size = 0;
	} else {
		STD_MEMCPY(buf->buf, orig, size);
	}

	ivm_vmstate_addDesLog(state, obj);

	return;
}
