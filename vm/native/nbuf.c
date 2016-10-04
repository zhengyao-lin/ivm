#include "pub/type.h"
#include "pub/const.h"
#include "pub/inlines.h"
#include "pub/vm.h"
#include "pub/obj.h"

#include "priv.h"
#include "nbuf.h"

IVM_NATIVE_FUNC(_buffer_cons)
{
	ivm_byte_t *buf;
	ivm_size_t size;

	CHECK_ARG_1(IVM_NUMERIC_T);

	size = ivm_numeric_getValue(NAT_ARG_AT(1));
	buf = ivm_vmstate_allocWild(NAT_STATE(), sizeof(*buf) * size);

	RTM_ASSERT(buf, IVM_ERROR_MSG_FAILED_ALLOC_BUFFER(size));

	return ivm_buffer_object_new_c(NAT_STATE(), size, buf);
}
