#include "pub/type.h"

#include "enc.h"

ivm_bool_t
ivm_enc_isAllAscii(const ivm_char_t *str)
{
	while (*str) {
		if ((ivm_uchar_t)*str > 0x7F) {
			return IVM_FALSE;
		}
		str++;
	}

	return IVM_TRUE;
}

ivm_size_t
ivm_enc_utf8_strlen(const ivm_char_t *mbs)
{
	ivm_char_t c;
	ivm_size_t ret = 0;

	while ((c = *mbs) != '\0') {
		if (c >= 0) mbs++;
		else if ((c & 0xE0) == 0xC0) mbs += 2;
		else if ((c & 0xF0) == 0xE0) mbs += 3;
		else if ((c & 0xF8) == 0xF0) mbs += 4;
		else if ((c & 0xFC) == 0xF8) mbs += 5;
		else if ((c & 0xFE) == 0xFC) mbs += 6;
		  
		ret++;
	}

	return ret;
}
