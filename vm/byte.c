#include "pub/mem.h"
#include "byte.h"
#include "str.h"
#include "err.h"

const ivm_char_t *
ivm_byte_readString(ivm_byte_t *bytes, ivm_size_t *size)
{
	if (size)
		*size = sizeof(const ivm_char_t *)
				* (IVM_STRLEN((const ivm_char_t *)bytes) + 1)
				/ sizeof(*bytes);
	return (const ivm_char_t *)bytes;
}

ivm_byte_t *
ivm_byte_newString(const ivm_char_t *str)
{
	ivm_byte_t *ret = (ivm_byte_t *)IVM_STRDUP(str);

	IVM_ASSERT(sizeof(ivm_byte_t) == sizeof(const ivm_char_t),
			   IVM_ERROR_MSG_BYTE_NOT_EQUAL_TO_CHAR);

	return ret;
}

ivm_byte_t *
ivm_byte_newSInt8(ivm_sint8_t num)
{
	ivm_byte_t *ret = MEM_ALLOC(sizeof(*ret)
								* (sizeof(ivm_sint8_t)
								   / sizeof(*ret)));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("byte for signed integer of size 8"));

	*ret = num;

	return ret;
}

ivm_byte_t *
ivm_byte_newSInt16(ivm_sint16_t num)
{
	ivm_byte_t *ret = MEM_ALLOC(sizeof(*ret)
								* (sizeof(ivm_sint16_t)
								   / sizeof(*ret)));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("byte for signed integer of size 16"));

	ret[0] = num >> 8;
	ret[1] = num;

	return ret;
}

ivm_byte_t *
ivm_byte_newSInt32(ivm_sint32_t num)
{
	ivm_byte_t *ret = MEM_ALLOC(sizeof(*ret)
								* (sizeof(ivm_sint32_t)
								   / sizeof(*ret)));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("byte for signed integer of size 32"));

	ret[0] = num >> 24;
	ret[1] = num >> 16;
	ret[2] = num >> 8;
	ret[3] = num;

	return ret;
}

ivm_byte_t *
ivm_byte_newSInt64(ivm_sint64_t num)
{
	ivm_byte_t *ret = MEM_ALLOC(sizeof(*ret)
								* (sizeof(ivm_sint64_t)
								   / sizeof(*ret)));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("byte for signed integer of size 64"));

	ret[0] = num >> 56;
	ret[1] = num >> 48;
	ret[2] = num >> 40;
	ret[3] = num >> 32;
	ret[4] = num >> 24;
	ret[5] = num >> 16;
	ret[6] = num >> 8;
	ret[7] = num;

	return ret;
}

ivm_size_t
ivm_byte_writeString(ivm_byte_t *bytes, ivm_size_t size, const ivm_char_t *str)
{
	ivm_size_t ret_size = sizeof(const ivm_char_t) * (IVM_STRLEN(str) + 1);

	if (size < ret_size / sizeof(*bytes)) {
		return 0;
	}

	MEM_COPY(bytes, str, ret_size);

	return ret_size / sizeof(*bytes);
}

ivm_size_t
ivm_byte_writeSInt8(ivm_byte_t *bytes, ivm_size_t size, ivm_sint8_t num)
{
	if (size < sizeof(num) / sizeof(*bytes)) {
		return 0;
	}

	*bytes = num;

	return sizeof(num) / sizeof(*bytes);
}

ivm_size_t
ivm_byte_writeSInt16(ivm_byte_t *bytes, ivm_size_t size, ivm_sint16_t num)
{
	if (size < sizeof(num) / sizeof(*bytes)) {
		return 0;
	}

	bytes[0] = num >> 8;
	bytes[1] = num;

	return sizeof(num) / sizeof(*bytes);
}

ivm_size_t
ivm_byte_writeSInt32(ivm_byte_t *bytes, ivm_size_t size, ivm_sint32_t num)
{
	if (size < sizeof(num) / sizeof(*bytes)) {
		return 0;
	}

	bytes[0] = num >> 24;
	bytes[1] = num >> 16;
	bytes[2] = num >> 8;
	bytes[3] = num;

	return sizeof(num) / sizeof(*bytes);
}

ivm_size_t
ivm_byte_writeSInt64(ivm_byte_t *bytes, ivm_size_t size, ivm_sint64_t num)
{
	if (size < sizeof(num) / sizeof(*bytes)) {
		return 0;
	}

	bytes[0] = num >> 56;
	bytes[1] = num >> 48;
	bytes[2] = num >> 40;
	bytes[3] = num >> 32;
	bytes[4] = num >> 24;
	bytes[5] = num >> 16;
	bytes[6] = num >> 8;
	bytes[7] = num;

	return sizeof(num) / sizeof(*bytes);
}
