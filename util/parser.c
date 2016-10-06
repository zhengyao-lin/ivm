#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "pub/com.h"
#include "pub/type.h"
#include "pub/err.h"
#include "pub/vm.h"

#include "std/mem.h"
#include "std/list.h"
#include "std/string.h"

#include "parser.h"

IVM_INLINE
ivm_bool_t
_is_hex(ivm_char_t c)
{
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

IVM_INLINE
ivm_bool_t
_is_oct(ivm_char_t c)
{
	return c >= '0' && c <= '7';
}

IVM_INLINE
ivm_bool_t
_is_dec(ivm_char_t c)
{
	return c >= '0' && c <= '9';
}

IVM_INLINE
ivm_bool_t
_is_digit(ivm_char_t c)
{
	return c >= '0' && c <= '9';
}

IVM_INLINE
ivm_bool_t
_is_legal(ivm_char_t c, ivm_uint_t rdx)
{
	if (rdx <= 10) {
		return c >= '0' && c < ('0' + rdx);
	}

	/* hex */
	return _is_hex(c);
}

IVM_INLINE
ivm_int_t
_hex_to_digit(ivm_char_t c)
{
	if (c >= '0' && c <= '9') {
		return c - '0';
	}

	if (c >= 'a' && c <= 'f') 
		return c - 'a' + 10;

	return c - 'A' + 10;
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
				ret += (ivm_double_t)_hex_to_digit(*src) / pow(radix, dec_pos);
				dec_pos++;
			} else {
				ret *= radix;
				ret += _hex_to_digit(*src);
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

ivm_size_t
ivm_parser_parseStr_c(ivm_char_t *buf,
					  const ivm_char_t *str,
					  ivm_size_t len,
					  const ivm_char_t **err)
{
	ivm_char_t *i, tmp;
	ivm_int_t num_count, size;
	const ivm_char_t *end;
	ivm_uint32_t c;
	ivm_char_t ubuf[6];

#define PARSE_ERR(msg) \
	if (err) {         \
		*err = (msg);  \
		return -1;     \
	}

	for (i = buf, end = str + len;
		 str != end; str++) {
		if (*str == '\\' &&
			str + 1 != end) {
			str++;
#define DEF_ESC(c, t) case c: *i++ = (t); continue;
			switch (tmp = *str) {
				DEF_ESC('a', '\a')
				DEF_ESC('b', '\b')
				DEF_ESC('f', '\f')
				DEF_ESC('n', '\n')
				DEF_ESC('r', '\r')
				DEF_ESC('t', '\t')
				DEF_ESC('v', '\v')
				DEF_ESC('\\', '\\')
				DEF_ESC('"', '"')
				DEF_ESC('\'', '\'')

			#define READ_NUM_C(type, rdx, max) \
				c = 0;                                      \
				num_count = 0;                              \
				while (_is_##type(tmp = *str)) {            \
					c = c * (rdx) + _hex_to_digit(tmp);     \
					if (++num_count >= (max) ||             \
						str + 1 == end ||                   \
						!_is_##type(*(str + 1))) break;     \
					str++;                                  \
				}

			#define READ_NUM(type, rdx, max) \
                READ_NUM_C(type, (rdx), (max));             \
				if (num_count) {                            \
					*i++ = c;                               \
					continue;                               \
				}

				case 'X':
				case 'x':
					if (str + 1 == end ||
						!_is_hex(*(str + 1))) {
						PARSE_ERR("wrong hex escape");
					}

					str++;

					READ_NUM(hex, 16, 2);

				case 'D':
				case 'd':
					if (str + 1 == end ||
						!_is_dec(*(str + 1))) {
						PARSE_ERR("wrong decimal escape");
					}
					str++;

					READ_NUM(dec, 10, 3);

				case 'U':
				case 'u':
					if (str + 1 == end ||
						!_is_hex(*(str + 1))) {
						PARSE_ERR("wrong Unicode escape");
					}
					str++;

					READ_NUM_C(hex, 16, 8);

					if (num_count) {
						size = ivm_enc_utf8_encode_c(c, ubuf);
						if (size > 0) {
							STD_MEMCPY(i, ubuf, sizeof(*ubuf) * size);
							i += size;
						} else {
							PARSE_ERR("illegal Unicode character");
						}
						continue;
					}

				case 'O':
				case 'o':
					if (str + 1 == end ||
						!_is_oct(*(str + 1))) {
						PARSE_ERR("wrong octal escape");
					}
					str++;

				default: READ_NUM(oct, 8, 3);
			#undef READ_NUM
			#undef READ_NUM_C
			}
#undef DES_ESC
		}

		*i++ = *str;
	}

	*i++ = '\0';

	return IVM_PTR_DIFF(i, buf, ivm_char_t);
}

ivm_char_t *
ivm_parser_parseStr(const ivm_char_t *str,
					ivm_size_t len,
					const ivm_char_t **err)
{
	ivm_char_t *buf = STD_ALLOC(sizeof(*buf) * (len + 1));

	IVM_MEMCHECK(buf);

	if (ivm_parser_parseStr_c(buf, str, len, err) == -1) {
		return IVM_NULL;
	}

	return buf;
}

ivm_char_t *
ivm_parser_parseStr_heap(ivm_heap_t *heap,
						 const ivm_char_t *str,
						 ivm_size_t len,
						 const ivm_char_t **err)
{
	ivm_char_t *buf = ivm_heap_alloc(heap, sizeof(*buf) * (len + 1));

	if (ivm_parser_parseStr_c(buf, str, len, err) == -1) {
		return IVM_NULL;
	}

	return buf;
}
