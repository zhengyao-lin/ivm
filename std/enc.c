#include "pub/type.h"
#include "pub/err.h"

#include "enc.h"

ivm_bool_t
ivm_enc_isAllAscii(const ivm_char_t *str)
{
	while (*str) {
		if ((ivm_uchar_t)*str > 0x7F)
			return IVM_FALSE;
		str++;
	}

	return IVM_TRUE;
}

ivm_bool_t
ivm_enc_isAllAscii_n(const ivm_char_t *str,
					 ivm_size_t n)
{
	ivm_size_t i;

	for (i = 0; i < n; i++) {
		if ((ivm_uchar_t)str[i] > 0x7F)
			return IVM_FALSE;
	}

	return IVM_TRUE;
}

ivm_size_t
ivm_enc_utf8_strlen_n(const ivm_char_t *mbs,
					  ivm_size_t n)
{
	ivm_uchar_t c;
	ivm_size_t i = 0, ret = 0;

	while (i < n) {
		c = mbs[i];

		if (c <= 0x7F) i++;
		else if ((c & 0xE0) == 0xC0) i += 2;
		else if ((c & 0xF0) == 0xE0) i += 3;
		else if ((c & 0xF8) == 0xF0) i += 4;
		else if ((c & 0xFC) == 0xF8) i += 5;
		else if ((c & 0xFE) == 0xFC) i += 6;
		else return -1;

		ret++;
	}

	if (i != n) return -1;

	return ret;
}

ivm_size_t
ivm_enc_strlen_n(const ivm_char_t *mbs,
				 ivm_size_t n,
				 ivm_int_t enc)
{
	// TODO: implement
	IVM_FATAL("no implement");
	return -1;
}
