#ifndef _IVM_STD_ENC_H_
#define _IVM_STD_ENC_H_

#include "pub/com.h"
#include "pub/type.h"

IVM_COM_HEADER

ivm_bool_t
ivm_enc_isAllAscii(const ivm_char_t *str);

ivm_size_t
ivm_enc_utf8_strlen(const ivm_char_t *mbs);

IVM_COM_END

#endif
