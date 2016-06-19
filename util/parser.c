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

#define PARSER_ERR_LP(l, p, ...) \
	IVM_TRACE("parser: at line %ld pos %ld: ", (l), (p)); \
	IVM_TRACE(__VA_ARGS__);

#define PARSER_ERR_P(p, ...) \
	IVM_TRACE("parser: at pos %ld: ", (p)); \
	IVM_TRACE(__VA_ARGS__);

#define PARSER_ERR_L(l, ...) \
	IVM_TRACE("parser: at line %ld: ", (l)); \
	IVM_TRACE(__VA_ARGS__);

#define PARSER_ERR_MSG_ILLEGAL_REG									("illegal regular\n")
#define PARSER_ERR_MSG_UNEXPECTED_CHAR(c, state)					"unexpected character '%c' in state %d\n", (c), (state)
#define PARSER_ERR_MSG_UNEXPECTED_ENDING(state)						"unexpected ending(still in state %d)\n", (state)
#define PARSER_ERR_MSG_ILLEGAL_CHAR_RADIX(c, rdx)					"illegal character '%c' in radix %d\n", (c), (rdx)
#define PARSER_ERR_MSG_UNMATCHED_ARGUMENT(instr, arg, exp)			"unmatched argument type for instruction '%s'(expecting %c, %c given)\n", (instr), (arg), (exp)

enum token_id_t {
	T_NONE = 0,
	T_ID,
	T_INT,
	T_FLOAT,
	T_STR,

	T_SEMIC,
	T_COMMA,

	T_NEWL,
	T_EOF
};

struct token_t {
	enum token_id_t id;
	ivm_size_t len;
	const ivm_char_t *val;
	
	ivm_size_t line;
	ivm_size_t pos;
};

enum state_t {
	ST_INIT = 0,
	ST_IN_ID,
	ST_IN_STR,
	ST_IN_STR_ESC,
	ST_IN_NUM_INT,
	ST_IN_NUM_DEC,
	STATE_COUNT,
	STATE_ERR
};

struct trans_entry_t {
	const ivm_char_t *match;
	enum state_t to_state;
	enum token_id_t save;
	ivm_bool_t ign;
	ivm_bool_t exc; // exclude current char
	const ivm_char_t *msg;
};

/*
	"=a": a
	"-ab": a-b or b-a
	".": .
	NULL: ->err
*/

IVM_PRIVATE
ivm_bool_t
_is_match(const char c,
		  const char *reg)
{
	ivm_size_t len = strlen(reg);

	IVM_ASSERT(len, PARSER_ERR_MSG_ILLEGAL_REG);

	// IVM_TRACE("reg: %c to %s\n", c, reg);

	switch (reg[0]) {
		case '=': return c == reg[1];
		case '-': return (c >= reg[1] && c <= reg[2]) ||
						 (c <= reg[1] && c >= reg[2]);
		case '.': return IVM_TRUE;
		default: ;
	}

	IVM_ASSERT(0, PARSER_ERR_MSG_ILLEGAL_REG);

	return IVM_FALSE;
}

ivm_list_t *
_ivm_parser_getTokens(const ivm_char_t *src)
{
	ivm_list_t *ret = ivm_list_new(sizeof(struct token_t));
	enum state_t state = ST_INIT;
	char tmp_buf[100];

	struct trans_entry_t
	trans_map[STATE_COUNT][20] = {
		/* INIT */			{
								{ "-az", ST_IN_ID },
								{ "-AZ", ST_IN_ID },
								{ "=_", ST_IN_ID },

								{ "-09", ST_IN_NUM_INT },
								{ "=.", ST_IN_NUM_DEC },

								{ "=,", ST_INIT, T_COMMA },
								{ "=;", ST_INIT, T_SEMIC },
								
								{ "=\n", ST_INIT, T_NEWL },
								{ "=\r", ST_INIT, T_NEWL },

								{ "=\"", ST_IN_STR, .ign = IVM_TRUE },
								{ "= ", ST_INIT, .ign = IVM_TRUE },

								{ "=\0", ST_INIT, T_EOF }
							},

		/* IN_ID */			{
								{ "-az", ST_IN_ID },
								{ "-AZ", ST_IN_ID },
								{ "=_", ST_IN_ID },
								{ "-09", ST_IN_ID },
								{ ".", ST_INIT, T_ID } /* revert one char */
							},

		/* IN_STR */		{
								{ "=\"", ST_INIT, T_STR, .exc = IVM_TRUE },
								{ "=\\", ST_IN_STR_ESC },
								{ ".", ST_IN_STR }
							},

		/* IN_STR_ESC */	{
								{ ".", ST_IN_STR }
							},

		/* IN_NUM_INT */	{
								{ "-09", ST_IN_NUM_INT },
								{ "=.", ST_IN_NUM_DEC },
								{ ".", ST_INIT, T_INT }
							},

		/* IN_NUM_DEC */	{
								{ "-09", ST_IN_NUM_DEC },
								{ ".", ST_INIT, T_FLOAT }
							}
	};


	#define NEXT_INIT ((struct token_t) { .len = 0, .val = c + 1, .line = LINE, .pos = POS })
	#define CUR_INIT (c--, (struct token_t) { .len = 0, .val = c + 1, .line = LINE, .pos = POS + 1 })
	#define LINE (line)
	#define POS ((ivm_ptr_t)c - col)

	const ivm_char_t *c = src;
	struct token_t tmp_token = (struct token_t) { .len = 0, .val = c };
	struct trans_entry_t *tmp_entry;
	ivm_size_t line = 1;
	ivm_ptr_t col = 0;

	do {
		// IVM_TRACE("matching %c state %d\n", *c, state);
		if (*c == '\n' || *c == '\r') {
			line++;
			col = (ivm_ptr_t)c;
		}

		for (tmp_entry = trans_map[state];
			 tmp_entry->match;
			 tmp_entry++) {
			if (_is_match(*c, tmp_entry->match)) {
				// tmp_token.val += tmp_entry->s_ofs;
				// tmp_token.len += tmp_entry->ofs;

				if (tmp_entry->ign) {
					tmp_token = NEXT_INIT;
				} else {
					if (tmp_entry->save != T_NONE) { // save to token stack
						if (tmp_entry->to_state == state)
							tmp_token.len++;

						MEM_COPY(tmp_buf, tmp_token.val, tmp_token.len);
						tmp_buf[tmp_token.len] = '\0';
						IVM_TRACE("token %d, value '%s'(len %ld)\n", tmp_entry->save, tmp_buf, tmp_token.len);

						tmp_token.id = tmp_entry->save;
						ivm_list_push(ret, &tmp_token);

						if (tmp_entry->exc || tmp_entry->to_state == state) {
							tmp_token = NEXT_INIT;
						} else {
							tmp_token = CUR_INIT;
						}
					} else {
						tmp_token.len++;
					}
				}

				// c += tmp_entry->c_ofs;

				break;
			}
		}

		if (tmp_entry->match)
			state = tmp_entry->to_state;
		else {
			PARSER_ERR_LP(LINE, POS, PARSER_ERR_MSG_UNEXPECTED_CHAR(*c, state));
		}
	} while (*c++ != '\0');

	#undef NEXT_INIT
	#undef CUR_INIT

	if (state != ST_INIT)
		PARSER_ERR_LP(LINE, POS, PARSER_ERR_MSG_UNEXPECTED_ENDING(state));

	#undef LINE
	#undef POS

	return ret;
}

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
_ivm_parser_parseNum(const ivm_char_t *src,
					 ivm_size_t len,
					 ivm_bool_t *err)
{
	ivm_int_t sign = 1;
	const ivm_char_t *beg = src, *end;
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
			PARSER_ERR_P((ivm_ptr_t)src - (ivm_ptr_t)beg,
						 PARSER_ERR_MSG_ILLEGAL_CHAR_RADIX(*src, radix));
			if (err) *err = IVM_TRUE;
		}
	}

	return ret * sign;
}

ivm_char_t *
_ivm_parser_parseStr(const ivm_char_t *str,
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

/*

trans_unit : instr_list

instr_list
	: instr
	| instr_list instr

instr : T_ID arg

argt
	: T_INT
	| T_FLOAT
	| T_STR

*/

struct arg_t {
	ivm_char_t type;
	ivm_opcode_arg_t val;
};

struct rule_val_t {
	ivm_size_t line;
	ivm_size_t pos;
	union {
		struct arg_t arg;
	} u;
};

struct env_t {
	ivm_exec_t *exec;
	ivm_string_pool_t *pool;
};

#define RULE_ARG \
	struct env_t *__env__, struct rule_val_t *__ret__, ivm_list_t *__tokens__, ivm_size_t *__i__

#define _ENV (__env__)
#define _RET (__ret__)
#define _RETVAL (__ret__->u)
#define _RETLINE (__ret__->line)
#define _RETPOS (__ret__->pos)
#define _TOKEN (__tokens__)

#define CUR_TOKEN() ((struct token_t *)ivm_list_at(_TOKEN, *__i__))
#define NEXT_TOKEN() (++*__i__)
#define HAS_NEXT_TOKEN() (*__i__ < ivm_list_size(_TOKEN))

#define RULE_NAME(name) _ivm_parser_rule_##name

#define RULE(name) \
	ivm_bool_t RULE_NAME(name)(RULE_ARG)

#define SHIFT(name, ret, ofs) \
	(*__i__ += (ofs), RULE_NAME(name)(_ENV, (ret), _TOKEN, __i__))

#define RESTORE_RULE() \
	__reti__ = __rets__; \
	*__i__ = __i_back__;

#define EXPECT_TOKEN(tid) \
	if (HAS_NEXT_TOKEN() && CUR_TOKEN()->id == (tid)) { \
		NEXT_TOKEN(); \
	} else { \
		RESTORE_RULE(); \
		break; \
	}

#define EXPECT_RULE(name) \
	if (!RULE_NAME(name)(_ENV, __reti__++, _TOKEN, __i__)) { \
		RESTORE_RULE(); \
		break; \
	}

#define EXPECT_RULE_LIST(name) \
	EXPECT_RULE(name); \
	while (RULE_NAME(name)(_ENV, __reti__++, _TOKEN, __i__)) ; \
	__reti__--;

#define RULE_RET_AT(i) (__rets__[(i)])
#define TOKEN_AT(i) ((struct token_t *)ivm_list_at(_TOKEN, __i_back__ + (i)))

#define SUB_RULE_SET(count, ...) \
	ivm_size_t __i_back__ = *__i__; \
	struct rule_val_t __rets__[count]; \
	struct rule_val_t *__reti__ = __rets__; \
	__reti__ == __reti__; \
	__VA_ARGS__; \
	goto RULE_FAILED;

#define SUB_RULE(...) \
	do { \
		__VA_ARGS__; \
		goto RULE_MATCHED; \
	} while (0);

#define FAILED(...) \
	goto RULE_FAILED_END; \
	RULE_FAILED: \
		__VA_ARGS__; \
		return IVM_FALSE;\
	RULE_FAILED_END: ;

#define MATCHED(...) \
	goto RULE_MATCHED_END; \
	RULE_MATCHED: \
		__VA_ARGS__; \
		return IVM_TRUE; \
	RULE_MATCHED_END: ;

RULE(arg)
{
	struct token_t *tmp_token;
	ivm_char_t *tmp_str;

#define ARG_VAL(type, val) ((struct arg_t) { (type), (val) })

	SUB_RULE_SET(
		3,
		
		SUB_RULE(EXPECT_TOKEN(T_INT)
		{
			tmp_token = TOKEN_AT(0);
			_RETLINE = tmp_token->line;
			_RETPOS = tmp_token->pos;

			_RETVAL.arg = ARG_VAL('I',
								  ivm_opcode_arg_fromInt(
									_ivm_parser_parseNum(tmp_token->val,
														 tmp_token->len, IVM_NULL)));
		})
		
		SUB_RULE(EXPECT_TOKEN(T_FLOAT)
		{
			tmp_token = TOKEN_AT(0);
			_RETLINE = tmp_token->line;
			_RETPOS = tmp_token->pos;

			_RETVAL.arg = ARG_VAL('F',
								  ivm_opcode_arg_fromFloat(
								  	_ivm_parser_parseNum(tmp_token->val,
														 tmp_token->len, IVM_NULL)));
		})
		
		SUB_RULE(EXPECT_TOKEN(T_STR)
		{
			tmp_token = TOKEN_AT(0);
			_RETLINE = tmp_token->line;
			_RETPOS = tmp_token->pos;

			tmp_str = _ivm_parser_parseStr(tmp_token->val, tmp_token->len);
			_RETVAL.arg = ARG_VAL('S', ivm_opcode_arg_fromInt(ivm_string_pool_registerRaw(_ENV->pool, tmp_str)));
			MEM_FREE(tmp_str);
		})
	);

	FAILED({ });
	MATCHED({ });

#undef ARG_VAL
}

RULE(instr_end)
{
	SUB_RULE_SET(
		3,
		SUB_RULE(EXPECT_TOKEN(T_NEWL) { })
		SUB_RULE(EXPECT_TOKEN(T_EOF) { })
		SUB_RULE(EXPECT_TOKEN(T_SEMIC) { })
	);

	FAILED({ });
	MATCHED({ });
}

RULE(instr)
{
	ivm_opcode_t opc;
	const ivm_char_t *param;
	struct token_t *tmp_token;
	struct rule_val_t ret;

	SUB_RULE_SET(
		2,

		SUB_RULE(EXPECT_TOKEN(T_ID) EXPECT_RULE(arg) EXPECT_RULE(instr_end)
		{
			tmp_token = TOKEN_AT(0);
			ret = RULE_RET_AT(0);

			ivm_char_t tmp_str[tmp_token->len + 1];
			MEM_COPY(tmp_str, tmp_token->val, tmp_token->len);
			tmp_str[tmp_token->len] = '\0';

			opc = ivm_opcode_searchOp(tmp_str);
			param = ivm_opcode_table_getParam(opc);

			if (param[0] != ret.u.arg.type) {
				PARSER_ERR_LP(ret.line, ret.pos,
							  PARSER_ERR_MSG_UNMATCHED_ARGUMENT(tmp_str, ret.u.arg.type, param[0]));
			}

			ivm_exec_addInstr_c(_ENV->exec, ivm_instr_build(opc, ret.u.arg.val));
		})

		SUB_RULE(EXPECT_TOKEN(T_ID) EXPECT_RULE(instr_end)
		{
			tmp_token = TOKEN_AT(0);
			
			ivm_char_t tmp_str[tmp_token->len + 1];
			MEM_COPY(tmp_str, tmp_token->val, tmp_token->len);
			tmp_str[tmp_token->len] = '\0';

			opc = ivm_opcode_searchOp(tmp_str);
			param = ivm_opcode_table_getParam(opc);

			if (param[0] != 'N') {
				PARSER_ERR_LP(tmp_token->line, tmp_token->pos,
							  PARSER_ERR_MSG_UNMATCHED_ARGUMENT(tmp_str, 'N', param[0]));
			}

			ivm_exec_addInstr_c(_ENV->exec, ivm_instr_build(opc, ivm_opcode_arg_fromInt(0)));
		})
	);

	FAILED({ });
	MATCHED({ });
}

RULE(instr_list)
{
	SUB_RULE_SET(
		1,
		SUB_RULE(EXPECT_RULE_LIST(instr))
	);

	FAILED({ });
	MATCHED({ });
}

RULE(trans_unit)
{
	SUB_RULE_SET(
		1,
		SUB_RULE(EXPECT_RULE(instr_list))
	);

	FAILED({ });

	MATCHED({ });
}

#define RULE_START(name, env, tokens) \
	ivm_size_t __i__ = 0; \
	struct rule_val_t __ret__; \
	RULE_NAME(name)((env), &__ret__, (tokens), &__i__);

ivm_exec_t *
ivm_parser_tokenToExec(ivm_list_t *tokens)
{
	ivm_string_pool_t *pool = ivm_string_pool_new(IVM_TRUE);
	struct env_t env = {
		ivm_exec_new(pool),
		pool
	};

	RULE_START(trans_unit, &env, tokens);

	return env.exec;
}
