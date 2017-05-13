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
		STD_FREE(buf);
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
		STD_FREE(buf);
		return IVM_NULL;
	}

	return buf;
}

ivm_char_t *
ivm_parser_parseStr_heap_n(ivm_heap_t *heap,
						   const ivm_char_t *str,
						   ivm_size_t len,
						   const ivm_char_t **err,
						   ivm_size_t *olen)
{
	ivm_char_t *buf = ivm_heap_alloc(heap, sizeof(*buf) * (len + 1));
	ivm_size_t ret = ivm_parser_parseStr_c(buf, str, len, err);

	if (ret == -1) {
		STD_FREE(buf);
		return IVM_NULL;
	}

	if (olen)
		*olen = ret - 1;

	return buf;
}
