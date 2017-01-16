#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "pub/type.h"
#include "pub/vm.h"
#include "pub/inlines.h"

#include "std/mem.h"
#include "std/conv.h"

#include "util/parser.h"

#include "vm/native/native.h"
#include "vm/native/priv.h"

#define JSON_ERROR_MSG_FAILED_PARSE(pos, msg)				"failed to parse JSON data: position %ld: %s", (pos), (msg)
#define JSON_ERROR_MSG_ILLEGAL_CHAR_BEFORE(value)			"illegal character before " value
#define JSON_ERROR_MSG_UNEXPECTED_ENDING					"unexpected ending"
#define JSON_ERROR_MSG_EXPECTING(sth)						"expecting '" sth "'"
#define JSON_ERROR_MSG_FAILED_PARSE_NUM						"failed to parse number"
#define JSON_ERROR_MSG_NON_SPACE_AFTER						"unexpected non-whitespace character after JSON data"
#define JSON_ERROR_MSG_MAX_RECUR							"max recursion count reached"

struct encode_flag_t {
	ivm_bool_t no_compact;
};

#define FLAG(...) ((struct encode_flag_t) { __VA_ARGS__ })

struct string_buffer_t {
	ivm_char_t *buf;
	ivm_size_t cur; // current efficient size
	ivm_size_t alloc;
};

#define STRING_BUF_INIT_BUF_SIZE 64

IVM_INLINE
struct string_buffer_t *
string_buffer_new()
{
	struct string_buffer_t *ret = STD_ALLOC(sizeof(*ret));

	IVM_MEMCHECK(ret);

	ret->buf = STD_ALLOC(sizeof(*ret->buf) * STRING_BUF_INIT_BUF_SIZE);

	IVM_MEMCHECK(ret->buf);

	ret->cur = 0;
	ret->alloc = STRING_BUF_INIT_BUF_SIZE;

	return ret;
}

IVM_INLINE
void
string_buffer_free(struct string_buffer_t *buf)
{
	if (buf) {
		STD_FREE(buf->buf);
		STD_FREE(buf);
	}

	return;
}

IVM_INLINE
void
string_buffer_expand(struct string_buffer_t *buf)
{
	buf->alloc <<= 1;

	if (buf->alloc <= buf->cur) {
		buf->alloc += buf->cur;
	}

	buf->buf = STD_REALLOC(buf->buf, sizeof(*buf->buf) * buf->alloc);
	IVM_MEMCHECK(buf->buf);

	return;
}

IVM_INLINE
void
string_buffer_addn(struct string_buffer_t *buf,
				   ivm_char_t *raw, ivm_size_t n)
{
	ivm_size_t orig = buf->cur;

	buf->cur += n;

	if (buf->cur >= buf->alloc) {
		string_buffer_expand(buf);
	}

	STD_MEMCPY(buf->buf + orig, raw, sizeof(*raw) * n);

	return;
}

IVM_INLINE
void
string_buffer_addc(struct string_buffer_t *buf,
				   ivm_char_t c)
{
	if (buf->cur + 1 >= buf->alloc) {
		string_buffer_expand(buf);
	}

	buf->buf[buf->cur] = c;
	buf->cur += 1;

	return;
}

IVM_INLINE
void
_json_writeEscape(struct string_buffer_t *buf,
				  const ivm_char_t *str,
				  ivm_size_t len)
{
	ivm_size_t n;

	string_buffer_addc(buf, '"');

	for (n = 0; n < len; n++) {
		switch (str[n]) {
			case '"': case '\\':
			case '/':

				string_buffer_addc(buf, '\\');
				string_buffer_addc(buf, str[n]);
				break;

			case '\b':

				string_buffer_addc(buf, '\\');
				string_buffer_addc(buf, 'b');
				break;

			case '\f':

				string_buffer_addc(buf, '\\');
				string_buffer_addc(buf, 'f');
				break;

			case '\n':

				string_buffer_addc(buf, '\\');
				string_buffer_addc(buf, 'n');
				break;

			case '\r':

				string_buffer_addc(buf, '\\');
				string_buffer_addc(buf, 'r');
				break;

			case '\t':

				string_buffer_addc(buf, '\\');
				string_buffer_addc(buf, 't');
				break;

			default:
				string_buffer_addc(buf, str[n]);
		}
	}

	string_buffer_addc(buf, '"');

	return;
}

/*
	4 types(ivm type -> json type):
		1. string -> string
		2. numeric -> number
		3. none -> null
		4. list -> array
		5. object
 */

IVM_PRIVATE
void
_json_encode_c(ivm_vmstate_t *state,
			   ivm_object_t *root,
			   struct string_buffer_t *buf,
			   struct encode_flag_t *flag)
{
	if (!root || IVM_IS_BTTYPE(root, state, IVM_NONE_T)) {

		string_buffer_addn(buf, "null", 4);

	} else if (IVM_IS_BTTYPE(root, state, IVM_STRING_OBJECT_T)) {
		
		const ivm_string_t *str = ivm_string_object_getValue(root);
		_json_writeEscape(buf, ivm_string_trimHead(str), ivm_string_length(str));

	} else if (IVM_IS_BTTYPE(root, state, IVM_NUMERIC_T)) {

		ivm_char_t dbuf[25];
		ivm_int_t size;

		size = ivm_conv_dtoa(ivm_numeric_getValue(root), dbuf);

		string_buffer_addn(buf, dbuf, size /* remove the \0 in the tail */);

	} else if (IVM_IS_BTTYPE(root, state, IVM_LIST_OBJECT_T)) {

		string_buffer_addc(buf, '[');

		ivm_list_object_t *lobj = IVM_AS(root, ivm_list_object_t);
		ivm_list_object_iterator_t iter;
		ivm_object_t *tmp_obj;
		ivm_bool_t is_first = IVM_TRUE;

		IVM_LIST_OBJECT_ALLPTR(lobj, iter) {
			if (!is_first) {
				string_buffer_addc(buf, ',');
			} else {
				is_first = IVM_FALSE;
			}

			tmp_obj = IVM_LIST_OBJECT_ITER_GET(iter);
			_json_encode_c(state, tmp_obj, buf, flag);
		}

		string_buffer_addc(buf, ']');

	} else {

		ivm_slot_table_t *slots = IVM_OBJECT_GET(root, SLOTS);
		ivm_slot_table_iterator_t iter;
		const ivm_string_t *key;
		ivm_object_t *val;
		ivm_bool_t is_first = IVM_TRUE;

		string_buffer_addc(buf, '{');

		if (slots) {
			IVM_SLOT_TABLE_EACHPTR(slots, iter) {
				if (!is_first) {
					string_buffer_addc(buf, ',');
				} else {
					is_first = IVM_FALSE;
				}

				key = IVM_SLOT_TABLE_ITER_GET_KEY(iter);
				val = IVM_SLOT_TABLE_ITER_GET_VAL(iter);

				_json_writeEscape(buf, ivm_string_trimHead(key), ivm_string_length(key));
				string_buffer_addc(buf, ':');
				_json_encode_c(state, val, buf, flag);
			}
		}

		string_buffer_addc(buf, '}');

	}

	return;
}

#define R(name) _json_parse_rule_##name

#define RULE(name) \
	ivm_object_t *                       \
	R(name)(ivm_vmstate_t *state,        \
			const ivm_char_t *src,       \
			ivm_size_t len,              \
			ivm_size_t *cur,             \
			ivm_size_t *err_pos,         \
			const ivm_char_t **err_msg,  \
			ivm_int_t recur)

#define CHECK_RECUR() \
	if (recur >= IVM_NATIVE_MAX_RECUR_COUNT) {  \
		*err_pos = *cur;                        \
		*err_msg = JSON_ERROR_MSG_MAX_RECUR;    \
		return IVM_NULL;                        \
	}

#define CALL_RULE(name, cur) \
	R(name)(state, src, len, (cur), err_pos, err_msg, recur + 1)

#define ERR_RET(pos, msg) \
	if (!*err_msg) {      \
		*err_pos = (pos); \
		*err_msg = (msg); \
	}                     \
	return IVM_NULL;

#define NO_MATCH() \
	return IVM_NULL;

IVM_INLINE
RULE(string)
{
	ivm_size_t i, str_len = 0;
	ivm_bool_t esc = IVM_FALSE;
	ivm_bool_t str_open = IVM_FALSE;
	const ivm_char_t *start = IVM_NULL;
	const ivm_char_t *msg = IVM_NULL;
	ivm_char_t *tmp_str;
	ivm_object_t *ret;

	for (i = *cur; i < len; i++) {
		switch (src[i]) {
			case '"':
				if (!str_open) {
					str_open = IVM_TRUE;
					start = src + i + 1;
				} else if (esc) {
					str_len++;
					esc = IVM_FALSE;
				} else {
					// string closed
					tmp_str = ivm_parser_parseStr(start, str_len, &msg);

					// IVM_TRACE("parse: %.*s %d %d\n", str_len, start, str_len, tmp_str);

					if (!tmp_str) {
						ERR_RET(i, msg);
					}

					*cur = i + 1;

					ret = ivm_string_object_new_r(state, tmp_str);
					STD_FREE(tmp_str);

					return ret;
				}

				break;

			case '\\':
				if (!str_open) {
					ERR_RET(i, JSON_ERROR_MSG_ILLEGAL_CHAR_BEFORE("string"));
				}

				str_len++;
				esc = !esc;
				break;

			case '\t': case ' ':
			case '\r': case '\n':
				if (str_open) {
					str_len++;
					esc = IVM_FALSE;
				}
				break;

			default:
				if (str_open) {
					str_len++;
					esc = IVM_FALSE;
				} else {
					// ERR_RET(i, JSON_ERROR_MSG_ILLEGAL_CHAR_BEFORE("string"));
					NO_MATCH();
				}
		}
	}

	ERR_RET(i, JSON_ERROR_MSG_UNEXPECTED_ENDING);
}

IVM_INLINE
RULE(number)
{
	ivm_size_t i, num_len = 0;
	const ivm_char_t *start = IVM_NULL;
	ivm_char_t c;
	ivm_double_t num;
	ivm_bool_t err = IVM_NULL;

	for (i = *cur; i < len; i++) {
		c = src[i];

		if (_is_dec(c) || c == '-' || c == '+') {
			start = src + i;
			break;
		}

		if (c != '\t' && c != ' ' &&
			c != '\r' && c != '\n') {
			// ERR_RET(i, JSON_ERROR_MSG_ILLEGAL_CHAR_BEFORE("number"));
			NO_MATCH();
		}
	}

	if (!start) {
		ERR_RET(i, JSON_ERROR_MSG_UNEXPECTED_ENDING);
	}

	for (; i < len; i++) {
		c = src[i];
		if (_is_dec(c) || c == '-' || c == '+' ||
			c == '.' || c == 'e' || c == 'E') {
			num_len++;
		} else break;
	}

	num = ivm_conv_parseDouble(start, num_len, IVM_NULL, &err);

	// IVM_TRACE("parse num: %.*s %d\n", num_len, start, err);

	if (err) {
		ERR_RET(i, JSON_ERROR_MSG_FAILED_PARSE_NUM);
	}

	*cur = i;

	return ivm_numeric_new(state, num);
}

IVM_PRIVATE
RULE(value);

IVM_INLINE
RULE(array)
{
	ivm_size_t i;
	ivm_char_t c;
	ivm_bool_t arr_open = IVM_FALSE;
	ivm_object_t *elem;
	ivm_bool_t find_comma = IVM_FALSE;
	ivm_list_object_t *ret;

	for (i = *cur; i < len; i++) {
		c = src[i];
		if (c == '[') {
			arr_open = IVM_TRUE;
			i++;
			break;
		}

		if (c != '\t' && c != ' ' &&
			c != '\r' && c != '\n') {
			// ERR_RET(i, JSON_ERROR_MSG_ILLEGAL_CHAR_BEFORE("array"));
			NO_MATCH();
		}
	}

	if (!arr_open) {
		ERR_RET(i, JSON_ERROR_MSG_UNEXPECTED_ENDING);
	}

	ret = IVM_AS(ivm_list_object_new(state), ivm_list_object_t);

	while (1) {
		elem = CALL_RULE(value, &i);

		if (elem) {
			ivm_list_object_push(ret, state, elem);
			
			find_comma = IVM_FALSE;

			for (; i < len; i++) {
				c = src[i];
				if (c == ',') {
					find_comma = IVM_TRUE;
					i++;
					break;
				}

				if (c != '\t' && c != ' ' &&
					c != '\r' && c != '\n')
					break;
			}
			
			if (!find_comma) break;
		} else break;
	}

	for (; i < len; i++) {
		c = src[i];
		if (c == ']') {
			// success
			*cur = i + 1;
			return IVM_AS_OBJ(ret);
		}

		if (c != '\t' && c != ' ' &&
			c != '\r' && c != '\n') {
			ERR_RET(i, JSON_ERROR_MSG_EXPECTING("]")); // illegal char
		}
	}

	ERR_RET(i, JSON_ERROR_MSG_UNEXPECTED_ENDING); // unexpected ending
}

IVM_INLINE
RULE(object)
{
	ivm_size_t i;
	ivm_char_t c;
	ivm_bool_t obj_open = IVM_FALSE;
	ivm_object_t *elem;
	ivm_bool_t find_comma = IVM_FALSE, find_colon = IVM_FALSE;
	ivm_object_t *ret;
	const ivm_string_t *key;

	for (i = *cur; i < len; i++) {
		c = src[i];
		if (c == '{') {
			obj_open = IVM_TRUE;
			i++;
			break;
		}

		if (c != '\t' && c != ' ' &&
			c != '\r' && c != '\n') {
			// ERR_RET(i, JSON_ERROR_MSG_ILLEGAL_CHAR_BEFORE("object")); // illegal char
			NO_MATCH();
		}
	}

	if (!obj_open) {
		ERR_RET(i, JSON_ERROR_MSG_UNEXPECTED_ENDING); // unexpected ending
	}

	ret = ivm_object_new(state);

	while (1) {
		elem = CALL_RULE(string, &i);

		if (!elem) break;
		key = ivm_string_object_getValue(elem);

		find_colon = IVM_FALSE;
		for (; i < len; i++) {
			c = src[i];
			if (c == ':') {
				find_colon = IVM_TRUE;
				i++;
				break;
			}

			if (c != '\t' && c != ' ' &&
				c != '\r' && c != '\n')
				break;
		}

		if (!find_colon) {
			ERR_RET(i, JSON_ERROR_MSG_EXPECTING(":")); // expecting colon
		}

		elem = CALL_RULE(value, &i);

		if (elem) {
			ivm_object_setSlot(ret, state, key, elem);
			
			find_comma = IVM_FALSE;

			for (; i < len; i++) {
				c = src[i];
				if (c == ',') {
					find_comma = IVM_TRUE;
					i++;
					break;
				}

				if (c != '\t' && c != ' ' &&
					c != '\r' && c != '\n')
					break;
			}
			
			if (!find_comma) break;
		} else break;
	}

	for (; i < len; i++) {
		c = src[i];
		if (c == '}') {
			// success
			*cur = i + 1;
			return IVM_AS_OBJ(ret);
		}

		if (c != '\t' && c != ' ' &&
			c != '\r' && c != '\n') {
			ERR_RET(i, JSON_ERROR_MSG_EXPECTING("}")); // illegal char
		}
	}

	ERR_RET(i, JSON_ERROR_MSG_UNEXPECTED_ENDING); // unexpected ending
}

IVM_PRIVATE
RULE(value)
{
	CHECK_RECUR();

	ivm_object_t *ret;
	ivm_size_t i, rest_len;
	ivm_char_t c;
	const ivm_char_t *rest;

	ret = CALL_RULE(object, cur);
	if (ret) return ret;

	ret = CALL_RULE(array, cur);
	if (ret) return ret;

	ret = CALL_RULE(string, cur);
	if (ret) return ret;

	ret = CALL_RULE(number, cur);
	if (ret) return ret;

	for (i = *cur; i < len; i++) {
		c = src[i];
		if (c != '\t' && c != ' ' &&
			c != '\r' && c != '\n')
			break;
	}

	rest = src + i;
	rest_len = len - i;

#define PARSE_CONST(name, value) \
	if (rest_len >= (IVM_ARRLEN(name) - 1) && \
		!IVM_STRNCMP(name, (IVM_ARRLEN(name) - 1), rest, (IVM_ARRLEN(name) - 1))) { \
		*cur = i + IVM_ARRLEN(name) - 1; \
		return (value); \
	}

	PARSE_CONST("true", ivm_bool_new(state, IVM_TRUE));
	PARSE_CONST("false", ivm_bool_new(state, IVM_FALSE));
	PARSE_CONST("null", IVM_NONE(state));

	ERR_RET(i, JSON_ERROR_MSG_ILLEGAL_CHAR_BEFORE("value"));
}

IVM_PRIVATE
ivm_object_t *
_json_parse_c(ivm_vmstate_t *state,
			  const ivm_char_t *src,
			  ivm_size_t len,
			  ivm_size_t *err_pos,
			  const ivm_char_t **err_msg)
{
	ivm_size_t cur = 0;
	ivm_object_t *ret;
	ivm_char_t c;

	ret = R(value)(state, src, len, &cur, err_pos, err_msg, 0);

	if (ret) {
		for (; cur < len; cur++) {
			c = src[cur];
			if (c != '\t' && c != ' ' &&
				c != '\r' && c != '\n') {
				ERR_RET(cur, JSON_ERROR_MSG_NON_SPACE_AFTER);
			}
		}
	}

	return ret;
}

IVM_NATIVE_FUNC(_json_encode)
{
	struct string_buffer_t *buf = string_buffer_new();
	struct encode_flag_t flag = FLAG(0);
	ivm_object_t *ret;

	CHECK_ARG_COUNT(1);

	_json_encode_c(NAT_STATE(), NAT_ARG_AT(1), buf, &flag);

	ret = ivm_string_object_new_rl(NAT_STATE(), buf->buf, buf->cur);

	string_buffer_free(buf);

	return ret;
}

IVM_NATIVE_FUNC(_json_decode)
{
	const ivm_string_t *str;
	ivm_object_t *ret;
	ivm_size_t err_pos;
	const ivm_char_t *err_msg = IVM_NULL;

	MATCH_ARG("s", &str);

	ret = _json_parse_c(NAT_STATE(), ivm_string_trimHead(str), ivm_string_length(str), &err_pos, &err_msg);

	RTM_ASSERT(ret, JSON_ERROR_MSG_FAILED_PARSE(err_pos, err_msg));

	return ret;
}

ivm_object_t *
ivm_mod_main(ivm_vmstate_t *state,
			 ivm_coro_t *coro,
			 ivm_context_t *context)
{
	ivm_object_t *mod = ivm_object_new_c(state, 2);

	// ivm_object_setSlot_r(mod, state, "parse", IVM_NATIVE_WRAP(state, _json_parse));
	ivm_object_setSlot_r(mod, state, "encode", IVM_NATIVE_WRAP(state, _json_encode));
	ivm_object_setSlot_r(mod, state, "decode", IVM_NATIVE_WRAP(state, _json_decode));

	return mod;
}
