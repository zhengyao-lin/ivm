#ifndef _IVM_STD_ENC_H_
#define _IVM_STD_ENC_H_

#include "pub/com.h"
#include "pub/type.h"

IVM_COM_HEADER

enum {
	IVM_ENCODING_UTF8
};

ivm_bool_t
ivm_enc_isAllAscii(const ivm_char_t *str);

ivm_bool_t
ivm_enc_isAllAscii_n(const ivm_char_t *str,
					 ivm_size_t n);

ivm_int_t
ivm_enc_utf8_step(const ivm_char_t *from);

/*
	return -1 if mbs has an unexpected ending
 */
ivm_size_t
ivm_enc_utf8_strlen_n(const ivm_char_t *mbs,
					  ivm_size_t n);

ivm_int_t
ivm_enc_utf8_encode_c(ivm_uint32_t c,
					  ivm_char_t buf[6]);

IVM_COM_END

#endif
