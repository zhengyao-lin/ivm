#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "pub/mem.h"
#include "pub/com.h"
#include "pub/type.h"
#include "pub/err.h"
#include "pub/vm.h"

#include "std/list.h"
#include "std/string.h"

#include "parser.h"

IVM_INLINE
ivm_bool_t
_is_legal(ivm_char_t c, ivm_uint_t rdx)
{
	if (rdx <= 10) {
		return c >= '0' && c < ('0' + rdx);
	}

	/* hex */
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

IVM_INLINE
ivm_int_t
_to_digit(ivm_char_t c, ivm_uint_t rdx)
{
	if (c >= '0' && c <= '9') {
		return c - '0';
	}

	if (c >= 'a' && c <= 'f') 
		return c - 'a';

	return c - 'A';
}

ivm_double_t
ivm_parser_parseNum(const ivm_char_t *src,
					ivm_size_t len,
					ivm_bool_t *overflow,
					ivm_bool_t *err)
{
	ivm_int_t sign = 1;
	const ivm_char_t *end;
	enum {
		R_DEC = 10,
		R_BIN = 2,
		R_OCT = 8,
		R_HEX = 16,
	} radix = R_DEC;
	ivm_double_t ret = 0;
	ivm_uint_t dec_pos = 0;

	/* sign & prefix */
	if (len) {
		/* sign */
		if (src[0] == '+') {
			src++; len--;
		} else if (src[0] == '-') {
			sign = -1;
			src++; len--;
		}

		if (len >= 2 && src[0] == '0') {
			if (src[1] == 'b' ||
				src[1] == 'B') {
				radix = R_BIN;
				src += 2; len -= 2;
			} else if (src[1] == 'o' ||
					   src[1] == 'O') {
				radix = R_OCT;
				src += 2; len -= 2;
			} else if (src[1] == 'x' ||
					   src[1] == 'X') {
				radix = R_HEX;
				src += 2; len -= 2;
			}
		}
	}

	for (end = src + len;
		 src != end;
		 src++) {
		if (*src == '.') {
			dec_pos = 1;
			continue;
		}

		if (_is_legal(*src, radix)) {
			if (dec_pos) {
				ret += (ivm_double_t)_to_digit(*src, radix) / pow(radix, dec_pos);
				dec_pos++;
			} else {
				ret *= radix;
				ret += _to_digit(*src, radix);
			}
		} else {
			// PARSER_ERR_P((ivm_ptr_t)src - (ivm_ptr_t)beg,
			// 			 PARSER_ERR_MSG_ILLEGAL_CHAR_RADIX(*src, radix));
			if (err) *err = IVM_TRUE;
		}
	}

	if (ret >= IVM_INT_MAX(ivm_int_t)
		&& overflow) {
		*overflow = IVM_TRUE;
	}

	return ret * sign;
}

ivm_char_t *
ivm_parser_parseStr(const ivm_char_t *str,
					ivm_size_t len)
{
	ivm_char_t *ret = MEM_ALLOC(sizeof(*ret) * (len + 1), ivm_char_t *);
	ivm_char_t *i = ret;
	const ivm_char_t *end;

	for (end = str + len;
		 str != end; str++) {
		if (*str == '\\' &&
			str + 1 != end) {
			str++;
#define DEF_ESC(c, t) case c: *i++ = (t); continue;
			switch (*str) {
				DEF_ESC('a', '\a')
				DEF_ESC('b', '\b')
				DEF_ESC('f', '\f')
				DEF_ESC('n', '\n')
				DEF_ESC('r', '\r')
				DEF_ESC('t', '\t')
				DEF_ESC('v', '\v')
				DEF_ESC('\\', '\\')
				DEF_ESC('"', '"')
			}
#undef DES_ESC
		}

		*i++ = *str;
	}

	*i = '\0';

	return ret;
}
