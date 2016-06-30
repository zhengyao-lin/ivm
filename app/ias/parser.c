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

#include "util/parser.h"

#include "parser.h"
#include "gen.h"

enum token_id_t {
	T_NONE = 0,
	T_UNKNOWN,
	T_ID,
	T_INT,
	T_FLOAT,
	T_STR,

	T_SEMIC,
	T_COMMA,
	T_COLON,
	T_LBRAC,
	T_RBRAC,

	T_NEWL,
	T_EOF
};

IVM_PRIVATE
const ivm_char_t *
token_name_table[] = {
	IVM_NULL,
	"unknown character",
	"identifier",
	"integer",
	"float",
	"string",
	"semicolon",
	"comma",
	"colon",
	"left brace",
	"right brace",
	"new line",
	"EOF"
};

#define TOKEN_NAME(tid) (token_name_table[tid])

struct err_msg_t {
	ivm_size_t line;
	ivm_size_t pos;

	const ivm_char_t *expect;
	const ivm_char_t *given;
};

#define ERR_MSG(l, p, e, g) ((struct err_msg_t) { (l), (p), (e), (g) })

struct token_t {
	enum token_id_t id;
	ivm_size_t len;
	const ivm_char_t *val;
	
	ivm_size_t line;
	ivm_size_t pos;
};

#define PARSER_ERR_LP(l, p, ...) \
	IVM_TRACE("parser: at line %zd pos %zd: ", (l), (p)); \
	IVM_TRACE(__VA_ARGS__); \
	IVM_TRACE("\n");

#define PARSER_ERR_P(p, ...) \
	IVM_TRACE("parser: at pos %zd: ", (p)); \
	IVM_TRACE(__VA_ARGS__); \
	IVM_TRACE("\n");

#define PARSER_ERR_L(l, ...) \
	IVM_TRACE("parser: at line %zd: ", (l)); \
	IVM_TRACE(__VA_ARGS__); \
	IVM_TRACE("\n");

#define PARSER_ERR_EM(err) \
	(IVM_TRACE("parser: at line %zd pos %zd: unexpected %s, expecting %s \n", \
			   (err)->line, (err)->pos, (err)->given, (err)->expect))

#define PARSER_ERR_MSG_ILLEGAL_REG									("illegal regular")
#define PARSER_ERR_MSG_UNEXPECTED_CHAR(c, state)					"unexpected character '%c' in state %d", (c), (state)
#define PARSER_ERR_MSG_UNEXPECTED_ENDING(state)						"unexpected ending(still in state %d)", (state)
#define PARSER_ERR_MSG_ILLEGAL_CHAR_RADIX(c, rdx)					"illegal character '%c' in radix %d", (c), (rdx)

enum state_t {
	ST_INIT = 0,
	ST_UNEXP,

	ST_TRY_COMMENT12,

	ST_IN_COMMENT1,
	ST_OUT_COMMENT1,

	ST_IN_COMMENT2,

	ST_IN_ID,
	
	ST_IN_STR_ID,
	ST_IN_ID_STR_ESC,
	
	ST_IN_STR,
	ST_IN_STR_ESC,
	
	ST_IN_NUM_INT,
	ST_IN_NUM_DOT,
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
_ias_parser_getTokens(const ivm_char_t *src)
{
	ivm_list_t *ret = ivm_list_new(sizeof(struct token_t));
	enum state_t state = ST_INIT;

	struct trans_entry_t
	trans_map[STATE_COUNT][20] = {
		/* INIT */			{
								{ "-az", ST_IN_ID },
								{ "-AZ", ST_IN_ID },
								{ "=_", ST_IN_ID },

								{ "-09", ST_IN_NUM_INT },
								{ "=.", ST_IN_NUM_DOT },

								{ "=,", ST_INIT, T_COMMA },
								{ "=;", ST_INIT, T_SEMIC },
								{ "=:", ST_INIT, T_COLON },
								{ "={", ST_INIT, T_LBRAC },
								{ "=}", ST_INIT, T_RBRAC },
								
								{ "=\n", ST_INIT, T_NEWL },
								{ "=\r", ST_INIT, T_NEWL },

								{ "=\"", ST_IN_STR, .ign = IVM_TRUE },
								{ "=`", ST_IN_STR_ID, .ign = IVM_TRUE },
								{ "= ", ST_INIT, .ign = IVM_TRUE },
								{ "=\t", ST_INIT, .ign = IVM_TRUE },

								{ "=/", ST_TRY_COMMENT12 },

								{ "=\0", ST_INIT, T_EOF }
							},

		/* UNEXP */			{
								{ ".", ST_INIT, .ign = IVM_TRUE }
							},

		/* TRY_COMMENT12 */	{
								{ "=*", ST_IN_COMMENT1 },
								{ "=/", ST_IN_COMMENT2 }
							},

		/* IN_COMMENT1 */	{
								{ "=*", ST_OUT_COMMENT1 },
								{ ".", ST_IN_COMMENT1 }
							},

		/* OUT_COMMENT1 */	{
								{ "=/", ST_INIT, .ign = IVM_TRUE },
								{ ".", ST_IN_COMMENT1 }
							},

		/* IN_COMMENT2 */	{
								{ "=\n", ST_INIT, T_NEWL },
								{ "=\r", ST_INIT, T_NEWL },
								{ ".", ST_IN_COMMENT2 }
							},

		/* IN_ID */			{
								{ "-az", ST_IN_ID },
								{ "-AZ", ST_IN_ID },
								{ "=_", ST_IN_ID },
								{ "-09", ST_IN_ID },
								{ ".", ST_INIT, T_ID } /* revert one char */
							},

		/* IN_STR_ID */		{
								{ "=`", ST_INIT, T_ID, .exc = IVM_TRUE },
								{ "=\\", ST_IN_ID_STR_ESC },
								{ ".", ST_IN_STR_ID }
							},

		/* IN_ID_STR_ESC */	{
								{ ".", ST_IN_STR_ID }
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

		/* IN_NUM_DOT */	{
								{ "-09", ST_IN_NUM_DEC }
							},

		/* IN_NUM_DEC */	{
								{ "-09", ST_IN_NUM_DEC },
								{ ".", ST_INIT, T_FLOAT }
							}
	};


	#define NEXT_INIT ((struct token_t) { .len = 0, .val = c + 1, .line = LINE, .pos = POS })
	#define CUR_INIT (c--, has_exc = IVM_TRUE, (struct token_t) { .len = 0, .val = c + 1, .line = LINE, .pos = POS + 1 })
	#define LINE (line)
	#define POS ((ivm_ptr_t)c - col + 1)

	const ivm_char_t *c = src;
	ivm_char_t cur_c;
	struct trans_entry_t *tmp_entry;
	ivm_size_t line = 1;
	ivm_ptr_t col = (ivm_ptr_t)c;
	ivm_bool_t has_exc = IVM_FALSE;
	struct token_t tmp_token = (struct token_t) { .len = 0, .val = c, .line = LINE, .pos = POS };

	if (c && *c != '\0') {
		do {
			cur_c = *c;
			// IVM_TRACE("matching %c state %d\n", *c, state);
			if (!has_exc) {
				if (*c == '\n') {
					line++;
					col = (ivm_ptr_t)c;
				}
			} else {
				has_exc = IVM_FALSE;
			}

			for (tmp_entry = trans_map[state];
				 tmp_entry->match;
				 tmp_entry++) {
				if (_is_match(*c, tmp_entry->match)) {
					// tmp_token.val += tmp_entry->s_ofs;
					// tmp_token.len += tmp_entry->ofs;

					if (tmp_entry->ign) {
						tmp_token = NEXT_INIT;
					} else if (tmp_entry->save != T_NONE) { // save to token stack
						if (tmp_entry->to_state == state)
							tmp_token.len++;

						// IVM_TRACE("token %d, value '%.*s'(len %zd)\n",
						// 		  tmp_entry->save, (int)tmp_token.len, tmp_token.val, tmp_token.len);

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

					// c += tmp_entry->c_ofs;

					break;
				}
			}

			if (tmp_entry->match) {
				state = tmp_entry->to_state;
			} else {
				PARSER_ERR_LP(LINE, POS, PARSER_ERR_MSG_UNEXPECTED_CHAR(*c, state));
				state = ST_UNEXP;
				c--;
			}

			c++;
		} while (cur_c != '\0');
	}

	#undef NEXT_INIT
	#undef CUR_INIT

	if (state != ST_INIT) {
		PARSER_ERR_LP(LINE, POS, PARSER_ERR_MSG_UNEXPECTED_ENDING(state));
	}

	#undef LINE
	#undef POS

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
	union {
		ias_gen_opcode_arg_t arg;
		ias_gen_instr_t instr;
		ias_gen_instr_list_t *instr_list;
		ias_gen_block_t block;
		ias_gen_block_list_t *block_list;
		ias_gen_env_t *env;
	} u;
};

struct env_t { int dummy; };

#define RULE_ARG \
	struct env_t *__env__, struct rule_val_t *__ret__, ivm_list_t *__tokens__, ivm_size_t *__i__, struct err_msg_t *__last_err__

#define MAX_RULE_COUNT 10
#define MAX_TOK_COUNT 10

#define _ENV (__env__)
#define _RET (__ret__)
#define _RETVAL (__ret__->u)
#define _RETLINE (__ret__->line)
#define _RETPOS (__ret__->pos)
#define _TOKEN (__tokens__)

#define CUR_TOKEN() ((struct token_t *)ivm_list_at(_TOKEN, *__i__))
#define NEXT_TOKEN() (++*__i__)
#define HAS_NEXT_TOKEN() (*__i__ < ivm_list_size(_TOKEN))

#define RULE_NAME(name) _ias_parser_rule_##name

#define RULE(name) \
	ivm_bool_t RULE_NAME(name)(RULE_ARG)

#define IS_SUC(call) (call)
#define IS_FAILED(call) (!IS_SUC(call))

#define SET_ERR(msg) (*__last_err__ = (msg))
#define POP_ERR() (PARSER_ERR_EM(__last_err__))

#define SHIFT(name, ret, ofs) \
	(*__i__ += (ofs), RULE_NAME(name)(_ENV, (ret), _TOKEN, __i__))

#define RESTORE_RULE() \
	__reti__ = __rets__; \
	__toki__ = __toks__; \
	*__i__ = __i_back__;

#define EXPECT_TOKEN(tid) \
	if (HAS_NEXT_TOKEN() && (*__toki__++ = CUR_TOKEN())->id == (tid)) { \
		NEXT_TOKEN(); \
	} else { \
		if (HAS_NEXT_TOKEN() && !__has_err__) { \
			__has_err__ = IVM_TRUE; \
			__tmp_token__ = CUR_TOKEN(); \
			__tmp_err__= ERR_MSG( \
				__tmp_token__->line, __tmp_token__->pos, \
				TOKEN_NAME(tid), TOKEN_NAME(__tmp_token__->id) \
			); \
		} \
		RESTORE_RULE(); \
		break; \
	}

#define EXPECT_RULE(name) \
	if (IS_FAILED(RULE_NAME(name)(_ENV, __reti__++, _TOKEN, __i__, __last_err__))) { \
		RESTORE_RULE(); \
		break; \
	}

#define EXPECT_RULE_NORET(name) \
	if (IS_FAILED(RULE_NAME(name)(_ENV, __reti__, _TOKEN, __i__, __last_err__))) { \
		break; \
	}

#define EXPECT_RULE_LIST(name, ...) \
	struct rule_val_t *__reti_back__ =  __reti__; \
	EXPECT_RULE(name); \
	do { \
		__VA_ARGS__; \
		while (IS_SUC(RULE_NAME(name)(_ENV, __reti_back__, _TOKEN, __i__, __last_err__))) { \
			__VA_ARGS__; \
		} \
	} while (0);

#define RULE_RET_AT(i) (__rets__[i])
#define TOKEN_AT(i) (__toks__[i]) // ((struct token_t *)ivm_list_at(_TOKEN, __i_back__ + (i)))

#define SUB_RULE_SET(...) \
	ivm_size_t __i_back__ = *__i__; \
	struct rule_val_t __rets__[MAX_RULE_COUNT]; \
	struct rule_val_t *__reti__ = __rets__; \
	struct token_t *__toks__[MAX_TOK_COUNT]; \
	struct token_t **__toki__ = __toks__; \
	struct token_t *__tmp_token__ = IVM_NULL; \
	ivm_bool_t __has_err__ = IVM_FALSE; \
	struct err_msg_t __tmp_err__; \
	__reti__ == __reti__; /* reduce unused variable warning */ \
	__toki__ == __toki__; \
	__tmp_token__ == __tmp_token__; \
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
		if (__has_err__) { \
			SET_ERR(__tmp_err__); \
		} \
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
	ivm_char_t type;

	/*
	 * arg : int
	 * 	   | float
	 * 	   | str
	 */
	SUB_RULE_SET(
		SUB_RULE(EXPECT_TOKEN(T_INT)
		{ type = 'I'; })
		
		SUB_RULE(EXPECT_TOKEN(T_FLOAT)
		{ type = 'F'; })
		
		SUB_RULE(EXPECT_TOKEN(T_STR)
		{ type = 'S'; })

		SUB_RULE(EXPECT_TOKEN(T_ID)
		{ type = 'D'; })
	);

	FAILED(0);
	MATCHED({
		tmp_token = TOKEN_AT(0);
		_RETVAL.arg = ias_gen_opcode_arg_build(
			tmp_token->line, tmp_token->pos,
			type, tmp_token->val, tmp_token->len
		);
	});
}

RULE(nl)
{
	/*
	 * nl : newl
	 */
	SUB_RULE_SET(
		SUB_RULE(EXPECT_TOKEN(T_NEWL))
	);

	FAILED(0);
	MATCHED(0);
}

RULE(nl_list)
{
	/*
	 * nl_list : list(nl)
	 */
	SUB_RULE_SET(
		SUB_RULE(EXPECT_RULE_LIST(nl, 0))
	);

	FAILED(0);
	MATCHED(0);
}

RULE(nl_list_opt)
{
	/*
	 * nl_list_opt : nl_list | %empty
	 */
	SUB_RULE_SET(
		SUB_RULE(EXPECT_RULE(nl_list))
		SUB_RULE()
	);

	FAILED(0);
	MATCHED(0);
}


RULE(instr)
{
	// ivm_opcode_t opc;
	// const ivm_char_t *param;
	struct token_t *label;
	struct token_t *tmp_token;
	struct rule_val_t ret;

	/*
	 * instr : id colon id arg
	 * 		 | id colon id
	 * 		 | id arg
	 * 		 | id
	 */
	SUB_RULE_SET(
		SUB_RULE(
			EXPECT_TOKEN(T_ID) EXPECT_TOKEN(T_COLON) EXPECT_RULE(nl_list_opt)
			EXPECT_TOKEN(T_ID) EXPECT_RULE(arg)
			{
				label = TOKEN_AT(0);
				tmp_token = TOKEN_AT(2);
				ret = RULE_RET_AT(1);

				_RETVAL.instr = ias_gen_instr_build(
					tmp_token->line, tmp_token->pos,
					label->val, label->len,
					tmp_token->val, tmp_token->len, ret.u.arg
				);
			}
		)

		SUB_RULE(
			EXPECT_TOKEN(T_ID) EXPECT_TOKEN(T_COLON) EXPECT_RULE(nl_list_opt)
			EXPECT_TOKEN(T_ID)
			{
				label = TOKEN_AT(0);
				tmp_token = TOKEN_AT(2);

				_RETVAL.instr = ias_gen_instr_build(
					tmp_token->line, tmp_token->pos,
					label->val, label->len,
					tmp_token->val, tmp_token->len,
					ias_gen_opcode_arg_build(tmp_token->line, tmp_token->pos, 'N')
				);
			}
		)

		SUB_RULE(
			EXPECT_TOKEN(T_ID) EXPECT_TOKEN(T_COLON)
			{
				label = TOKEN_AT(0);

				_RETVAL.instr = ias_gen_instr_build(
					label->line, label->pos,
					label->val, label->len, 0
				);
			}
		)

		SUB_RULE(
			EXPECT_TOKEN(T_ID) EXPECT_RULE(arg)
			{
				tmp_token = TOKEN_AT(0);
				ret = RULE_RET_AT(0);

				_RETVAL.instr = ias_gen_instr_build(
					tmp_token->line, tmp_token->pos,
					IVM_NULL, 0,
					tmp_token->val, tmp_token->len, ret.u.arg
				);
			}
		)

		SUB_RULE(
			EXPECT_TOKEN(T_ID)
			{
				tmp_token = TOKEN_AT(0);

				_RETVAL.instr = ias_gen_instr_build(
					tmp_token->line, tmp_token->pos,
					IVM_NULL, 0,
					tmp_token->val, tmp_token->len,
					ias_gen_opcode_arg_build(tmp_token->line, tmp_token->pos, 'N')
				);
			}
		)
	);

	FAILED(0);
	MATCHED(0);
}

RULE(instr_end_single)
{
	/*
	 * instr_end_single : newl | semic
	 */
	SUB_RULE_SET(
		SUB_RULE(EXPECT_TOKEN(T_NEWL))
		SUB_RULE(EXPECT_TOKEN(T_SEMIC))
	);

	FAILED(0);
	MATCHED(0);
}

RULE(instr_end)
{
	/*
	 * instr_end : list(instr_end_single)
	 */
	SUB_RULE_SET(
		SUB_RULE(EXPECT_RULE_LIST(instr_end_single, 0))
	);

	FAILED(0);
	MATCHED(0);
}

RULE(instr_end_opt)
{
	/*
	 * instr_end_opt : instr_end | %empty
	 */
	SUB_RULE_SET(
		SUB_RULE(EXPECT_RULE(instr_end))
		SUB_RULE()
	);

	FAILED(0);
	MATCHED(0);
}

RULE(instr_list)
{
	ias_gen_instr_list_t *list = ias_gen_instr_list_new();

	/*
	 * instr_list : instr_end_opt list(instr, instr_end)
	 */
	SUB_RULE_SET(
		SUB_RULE(
			EXPECT_RULE(instr_end_opt)
			EXPECT_RULE_LIST(instr, {
				ias_gen_instr_list_add(list, &RULE_RET_AT(1).u.instr);
			} EXPECT_RULE_NORET(instr_end))

			{
				_RETVAL.instr_list = list;
			}
		)
	);

	FAILED({
		ias_gen_instr_list_free(list);
	});

	MATCHED(0);
}

RULE(block)
{
	struct token_t *tmp_token;
	ias_gen_instr_list_t *list = IVM_NULL;

	/*
	 * block : id { instr_list }
	 * 		 | id { instr_end_opt }
	 */
	SUB_RULE_SET(
		SUB_RULE(
			EXPECT_TOKEN(T_ID) EXPECT_RULE(nl_list_opt)
			EXPECT_TOKEN(T_LBRAC)
				EXPECT_RULE(instr_list) {
					list = RULE_RET_AT(1).u.instr_list;
				}
			EXPECT_TOKEN(T_RBRAC)
		)
		SUB_RULE(
			EXPECT_TOKEN(T_ID) EXPECT_RULE(nl_list_opt)
			EXPECT_TOKEN(T_LBRAC)
				EXPECT_RULE(instr_end_opt)
			EXPECT_TOKEN(T_RBRAC)
		)
	);

	FAILED({
		ias_gen_instr_list_free(list);
	});

	MATCHED({
		tmp_token = TOKEN_AT(0);
		_RETVAL.block = ias_gen_block_build(
			tmp_token->line, tmp_token->pos,
			tmp_token->val, tmp_token->len, list
		);
	});
}

RULE(block_end_single)
{
	/*
	 * block_end_single : newl | eof
	 */
	SUB_RULE_SET(
		SUB_RULE(EXPECT_TOKEN(T_NEWL))
		SUB_RULE(EXPECT_TOKEN(T_EOF))
	);

	FAILED(0);
	MATCHED(0);
}

RULE(block_end)
{
	/*
	 * block_end : list(block_end_single)
	 */
	SUB_RULE_SET(
		SUB_RULE(EXPECT_RULE_LIST(block_end_single, 0))
	);

	FAILED(0);
	MATCHED(0);
}

RULE(block_end_opt)
{
	/*
	 * block_end_opt : block_end
	 * 				 | %empty
	 */
	SUB_RULE_SET(
		SUB_RULE(EXPECT_RULE(block_end))
		SUB_RULE()
	);

	FAILED(0);
	MATCHED(0);
}

RULE(block_list)
{
	ias_gen_block_list_t *list = ias_gen_block_list_new();

	/*
	 * block_list : block_end_opt list(block, block_end_opt)
	 */
	SUB_RULE_SET(
		SUB_RULE(
			EXPECT_RULE(block_end_opt)
			EXPECT_RULE_LIST(block, {
				ias_gen_block_list_push(list, &RULE_RET_AT(1).u.block);
			} EXPECT_RULE_NORET(block_end_opt))
		)
	);

	FAILED({
		ias_gen_block_list_free(list);
		_RETVAL.block_list = IVM_NULL;
	});

	MATCHED({
		_RETVAL.block_list = list;
	});
}

RULE(trans_unit)
{
	/*
	 * trans_unit : block_list
	 * 			  | block_end_opt
	 */
	SUB_RULE_SET(
		SUB_RULE(EXPECT_RULE(block_list))
		SUB_RULE(EXPECT_RULE(block_end_opt))
	);

	FAILED({
		_RETVAL.env = ias_gen_env_new(IVM_NULL);
		POP_ERR();
	});

	MATCHED({
		if (HAS_NEXT_TOKEN()) {
			POP_ERR();
		}
		_RETVAL.env = ias_gen_env_new(RULE_RET_AT(0).u.block_list);
	});
}

#define RULE_START(name, env, ret, tokens) \
	ivm_size_t __i__ = 0; \
	struct err_msg_t __last_err__; \
	RULE_NAME(name)((env), (ret), (tokens), &__i__, &__last_err__);

ias_gen_env_t *
_ias_parser_tokenToEnv(ivm_list_t *tokens)
{
	struct env_t env = { 0 };
	struct rule_val_t ret;

	RULE_START(trans_unit, &env, &ret, tokens);

	return ret.u.env;
}

ias_gen_env_t *
ias_parser_parseSource(const ivm_char_t *src)
{
	ivm_list_t *tokens = _ias_parser_getTokens(src);
	ias_gen_env_t *ret = _ias_parser_tokenToEnv(tokens);

	ivm_list_free(tokens);

	return ret;
}