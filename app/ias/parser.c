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

#define IVM_USE_COMMON_PARSER
#define IVM_COMMON_PARSER_NAME "ias"
#include "util/parser.h"

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

IVM_PRIVATE
ivm_list_t *
_ias_parser_getTokens(const ivm_char_t *src)
{
	return TOKENIZE(src,
		/* INIT */
		{
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

		/* UNEXP */
		{
			{ ".", ST_INIT, .ign = IVM_TRUE }
		},

		/* TRY_COMMENT12 */
		{
			{ "=*", ST_IN_COMMENT1 },
			{ "=/", ST_IN_COMMENT2 }
		},

		/* IN_COMMENT1 */
		{
			{ "=*", ST_OUT_COMMENT1 },
			{ ".", ST_IN_COMMENT1 }
		},

		/* OUT_COMMENT1 */
		{
			{ "=/", ST_INIT, .ign = IVM_TRUE },
			{ ".", ST_IN_COMMENT1 }
		},

		/* IN_COMMENT2 */
		{
			{ "=\n", ST_INIT, T_NEWL },
			{ "=\r", ST_INIT, T_NEWL },
			{ ".", ST_IN_COMMENT2 }
		},

		/* IN_ID */
		{
			{ "-az", ST_IN_ID },
			{ "-AZ", ST_IN_ID },
			{ "=_", ST_IN_ID },
			{ "-09", ST_IN_ID },
			{ ".", ST_INIT, T_ID } /* revert one char */
		},

		/* IN_STR_ID */
		{
			{ "=`", ST_INIT, T_ID, .exc = IVM_TRUE },
			{ "=\\", ST_IN_ID_STR_ESC },
			{ ".", ST_IN_STR_ID }
		},

		/* IN_ID_STR_ESC */
		{
			{ ".", ST_IN_STR_ID }
		},

		/* IN_STR */
		{
			{ "=\"", ST_INIT, T_STR, .exc = IVM_TRUE },
			{ "=\\", ST_IN_STR_ESC },
			{ ".", ST_IN_STR }
		},

		/* IN_STR_ESC */
		{
			{ ".", ST_IN_STR }
		},

		/* IN_NUM_INT */
		{
			{ "-09", ST_IN_NUM_INT },
			{ "=.", ST_IN_NUM_DEC },
			{ ".", ST_INIT, T_INT }
		},

		/* IN_NUM_DOT */
		{
			{ "-09", ST_IN_NUM_DEC }
		},

		/* IN_NUM_DEC */
		{
			{ "-09", ST_IN_NUM_DEC },
			{ ".", ST_INIT, T_FLOAT }
		}
	);
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
				CLEAR_ERR();
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

ias_gen_env_t *
_ias_parser_tokenToEnv(ivm_list_t *tokens)
{
	struct env_t env = { 0 };
	struct rule_val_t ret;
	ivm_bool_t suc;

	RULE_START(trans_unit, &env, &ret, tokens, suc);

	suc = suc;

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
