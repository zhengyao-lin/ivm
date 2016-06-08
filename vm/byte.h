#ifndef _IVM_VM_BYTE_H_
#define _IVM_VM_BYTE_H_

#include "pub/com.h"
#include "pub/type.h"

IVM_COM_HEADER

#define IVM_STRING_POOL_INDEX_SIZE (sizeof(ivm_sint32_t))

const ivm_char_t *
ivm_byte_readString(ivm_byte_t *bytes, ivm_size_t *size);

#define ivm_byte_readSInt8(bytes) (*((ivm_sint8_t *)bytes))
#define ivm_byte_readSInt16(bytes) (*((ivm_sint16_t *)(bytes)))
#define ivm_byte_readSInt32(bytes) (*((ivm_sint32_t *)(bytes)))
#define ivm_byte_readSInt64(bytes) (*((ivm_sint64_t *)(bytes)))

#if 0
#define ivm_byte_readSInt16(bytes) \
	(((ivm_sint16_t)(bytes)[0] << 8) + \
	 ((bytes)[1]))

#define ivm_byte_readSInt32(bytes) \
	(((ivm_sint32_t)(bytes)[0] << 24) + \
	 ((ivm_sint32_t)(bytes)[1] << 16) + \
	 ((ivm_sint32_t)(bytes)[2] << 8) + \
	 ((bytes)[3]))

#define ivm_byte_readSInt64(bytes) \
	(((ivm_sint64_t)(bytes)[0] << 56) + \
	 ((ivm_sint64_t)(bytes)[1] << 48) + \
	 ((ivm_sint64_t)(bytes)[2] << 40) + \
	 ((ivm_sint64_t)(bytes)[3] << 32) + \
	 ((ivm_sint64_t)(bytes)[4] << 24) + \
	 ((ivm_sint64_t)(bytes)[5] << 16) + \
	 ((ivm_sint64_t)(bytes)[6] << 8) + \
	 ((bytes)[7]))
#endif

ivm_byte_t *
ivm_byte_newString(const ivm_char_t *str);
ivm_byte_t *
ivm_byte_newSInt8(ivm_sint8_t num);
ivm_byte_t *
ivm_byte_newSInt16(ivm_sint16_t num);
ivm_byte_t *
ivm_byte_newSInt32(ivm_sint32_t num);
ivm_byte_t *
ivm_byte_newSInt64(ivm_sint64_t num);

ivm_size_t
ivm_byte_writeString(ivm_byte_t *bytes, ivm_size_t size, const ivm_char_t *str);
ivm_size_t
ivm_byte_writeSInt8(ivm_byte_t *bytes, ivm_size_t size, ivm_sint8_t num);
ivm_size_t
ivm_byte_writeSInt16(ivm_byte_t *bytes, ivm_size_t size, ivm_sint16_t num);
ivm_size_t
ivm_byte_writeSInt32(ivm_byte_t *bytes, ivm_size_t size, ivm_sint32_t num);
ivm_size_t
ivm_byte_writeSInt64(ivm_byte_t *bytes, ivm_size_t size, ivm_sint64_t num);

IVM_COM_END

#endif
