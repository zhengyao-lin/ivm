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
	ivm_long_t size;

	CHECK_ARG_1(IVM_NUMERIC_T);

	size = ivm_numeric_getValue(NAT_ARG_AT(1));

	RTM_ASSERT(size >= 0, IVM_ERROR_MSG_ILLEGAL_BUFFER_SIZE(size));

	buf = ivm_vmstate_allocWild(NAT_STATE(), sizeof(*buf) * size);

	RTM_ASSERT(buf, IVM_ERROR_MSG_FAILED_ALLOC_BUFFER(size));

	return ivm_buffer_object_new_c(NAT_STATE(), size, buf);
}

IVM_NATIVE_FUNC(_buffer_size)
{
	CHECK_BASE(IVM_BUFFER_OBJECT_T);
	return ivm_numeric_new(NAT_STATE(), ivm_buffer_object_getSize(IVM_AS(NAT_BASE(), ivm_buffer_object_t)));
}

IVM_INLINE
ivm_char_t // convert lower 4 bits to char
_to_hex(ivm_byte_t b)
{
	b = b & 0xF;
	return b <= 9 ? '0' + b : 'a' + b - 10;
}

IVM_NATIVE_FUNC(_buffer_to_s)
{
	ivm_buffer_object_t *buf_obj;
	ivm_char_t *buf;
	ivm_string_t *str;
	ivm_size_t size;
	ivm_byte_t *i, tmp, *end;

	CHECK_BASE(IVM_BUFFER_OBJECT_T);

	buf_obj = IVM_AS(NAT_BASE(), ivm_buffer_object_t);
	i = ivm_buffer_object_getRaw(buf_obj);
	size = ivm_buffer_object_getSize(buf_obj);
	str = ivm_vmstate_preallocStr(NAT_STATE(), size * 2 + 2, &buf); // 2 char per byte + '0x' prefix

#define _WR(c) (*(buf++) = (c))

	_WR('0');
	_WR('x');

	for (end = i + size;
		 i != end; i++) {
		tmp = *i;
		_WR(_to_hex(tmp >> 4));
		_WR(_to_hex(tmp));
	}

	_WR('\0');

#undef _WR

	return ivm_string_object_new(NAT_STATE(), str);
}
