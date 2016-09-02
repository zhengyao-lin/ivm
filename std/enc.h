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

/*
	return -1 if mbs contains illegal character in utf-8
 */
ivm_size_t
ivm_enc_utf8_strlen_n(const ivm_char_t *mbs,
					  ivm_size_t n);

ivm_size_t
ivm_enc_strlen_n(const ivm_char_t *mbs,
				 ivm_size_t n,
				 ivm_int_t enc);

IVM_COM_END

#endif
