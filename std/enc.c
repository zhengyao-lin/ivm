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
	ivm_size_t i = 0, ret = 0;

	while (i < n) {
		i += ivm_enc_utf8_step(mbs + i);
		ret++;
	}

	if (i != n) return -1;

	return ret;
}

#define HEAD(n) ((ivm_uchar_t)(~0 << (8 - (n))))
#define PREFIX 0x80
#define LAST6(c) ((c) & 0x3F)
#define FIRSTNBITS(c, n) ((c) & HEAD(n))

/* n: number of bytes */
#define FIRSTC(n, c) (HEAD(n) | ((c) >> (((n) - 1) * 6)))

/* i: index of the byte(1 ~ (n - 1)) */
#define NEXTNC(n, i, c) (PREFIX | LAST6((c) >> (((n) - (i) - 1) * 6)))

ivm_int_t
ivm_enc_utf8_step(const ivm_char_t *from)
{
	ivm_uchar_t c = *from;

	if (c <= 0x7F) return 1;
	else if (FIRSTNBITS(c, 3) == HEAD(2)) return 2;
	else if (FIRSTNBITS(c, 4) == HEAD(3)) return 3;
	else if (FIRSTNBITS(c, 5) == HEAD(4)) return 4;
	else if (FIRSTNBITS(c, 6) == HEAD(5)) return 5;
	else if (FIRSTNBITS(c, 7) == HEAD(6)) return 6;
	
	return 1;
}

ivm_int_t
ivm_enc_utf8_encode_c(ivm_uint32_t c,
					  ivm_char_t buf[6])
{

	if (c <= 0x7F) {
		buf[0] = c;

		return 1;
	} else if (c <= 0x7FF) {
		buf[0] = FIRSTC(2, c);
		buf[1] = NEXTNC(2, 1, c);

		return 2;
	} else if (c <= 0xFFFF) {
		buf[0] = FIRSTC(3, c);
		buf[1] = NEXTNC(3, 1, c);
		buf[2] = NEXTNC(3, 2, c);

		return 3;
	} else if (c <= 0x1FFFFF) {
		buf[0] = FIRSTC(4, c);
		buf[1] = NEXTNC(4, 1, c);
		buf[2] = NEXTNC(4, 2, c);
		buf[3] = NEXTNC(4, 3, c);

		return 4;
	} else if (c <= 0x3FFFFFF) {
		buf[0] = FIRSTC(5, c);
		buf[1] = NEXTNC(5, 1, c);
		buf[2] = NEXTNC(5, 2, c);
		buf[3] = NEXTNC(5, 3, c);
		buf[4] = NEXTNC(5, 4, c);

		return 5;
	} else if (c <= 0x7FFFFFFF) {
		buf[0] = FIRSTC(6, c);
		buf[1] = NEXTNC(6, 1, c);
		buf[2] = NEXTNC(6, 2, c);
		buf[3] = NEXTNC(6, 3, c);
		buf[4] = NEXTNC(6, 4, c);
		buf[5] = NEXTNC(6, 5, c);

		return 6;
	}

	return -1;
}
