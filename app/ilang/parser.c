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

	/* keywords */
	T_FN,
	T_IF,
	T_ELIF,
	T_ELSE,
	T_WHILE,
	T_RET,
	T_CONT,
	T_BREAK,

	T_CLONE,

	T_SEMIC,	// ;
	T_COMMA,	// ,
	T_COLON,	// :
	T_DOT,		// .

	T_LBRAC,	// {
	T_RBRAC,	// }

	T_LPAREN,	// (
	T_RPAREN,	// )

	T_LBRAKT,	// [
	T_RBRAKT,	// ]

	T_ASSIGN,	// =
	T_ADD,		// +
	T_SUB,		// -
	T_MUL,		// *
	T_DIV,		// /
	T_MOD,		// %

	T_NOT,		// !

	T_CGT,		// >
	T_CGE,		// >=
	T_CLT,		// <
	T_CLE,		// <=
	T_CEQ,		// ==
	T_CNE,		// !=

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
	
	"keyword `fn`",
	"keyword `if`",
	"keyword `elif`",
	"keyword `else`",
	"keyword `while`",
	"keyword `ret`",
	"keyword `cont`",
	"keyword `break`",

	"operator `clone`",

	"semicolon",
	"comma",
	"colon",
	"dot",

	"left brace",
	"right brace",
	
	"left parentheses",
	"right parenthesis",

	"left square bracket",
	"right square bracket",

	"assign operator",
	"add operator",
	"sub operator",
	"mul operator",
	"div operator",
	"mod operator",

	"not operator",

	"greater-than operator",
	"greater-than-or-equal operator",
	"less-than operator",
	"less-than-or-equal operator",
	"equal operator",
	"not equal operator",

	"new line",
	"EOF"
};

#define IVM_USE_COMMON_PARSER
#define IVM_COMMON_DEBUG_MODE 1
#define IVM_COMMON_MAX_TOKEN_RULE 40
#define IVM_COMMON_PARSER_NAME "ilang"
#include "util/parser.h"

enum state_t {
	ST_INIT = 0,
	ST_UNEXP,

	ST_TRY_GT,
	ST_TRY_LT,
	ST_TRY_NOT,
	ST_TRY_ASSIGN,

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

ivm_list_t *
_ilang_parser_getTokens(const ivm_char_t *src)
{
	ivm_list_t *ret = TOKENIZE(src,
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

			{ "=(", ST_INIT, T_LPAREN },
			{ "=)", ST_INIT, T_RPAREN },

			{ "=[", ST_INIT, T_LBRAKT },
			{ "=]", ST_INIT, T_RBRAKT },

			{ "=+", ST_INIT, T_ADD },
			{ "=-", ST_INIT, T_SUB },
			{ "=*", ST_INIT, T_MUL },
			{ "=%", ST_INIT, T_MOD },

			{ "=!", ST_TRY_NOT },

			{ "=>", ST_TRY_GT },
			{ "=<", ST_TRY_LT },
			{ "==", ST_TRY_ASSIGN },
			
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

		/* TRY_GT */
		{
			{ "==", ST_INIT, T_CGE, .ext = IVM_TRUE, .exc = IVM_TRUE },
			{ ".", ST_INIT, T_CGT },
		},

		/* TRY_LT */
		{
			{ "==", ST_INIT, T_CLE, .ext = IVM_TRUE, .exc = IVM_TRUE },
			{ ".", ST_INIT, T_CLT }
		},

		/* TRY_NOT */
		{
			{ "==", ST_INIT, T_CNE, .ext = IVM_TRUE, .exc = IVM_TRUE },
			{ ".", ST_INIT, T_NOT }
		},

		/* TRY_ASSIGN */
		{
			{ "==", ST_INIT, T_CEQ, .ext = IVM_TRUE, .exc = IVM_TRUE },
			{ ".", ST_INIT, T_ASSIGN }
		},

		/* TRY_COMMENT12 */
		{
			{ "=*", ST_IN_COMMENT1 },
			{ "=/", ST_IN_COMMENT2 },
			{ ".", ST_INIT, T_DIV, .exc = IVM_TRUE }
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
			{ "-09", ST_IN_NUM_DEC },
			{ ".", ST_INIT, T_DOT }
		},

		/* IN_NUM_DEC */
		{
			{ "-09", ST_IN_NUM_DEC },
			{ ".", ST_INIT, T_FLOAT }
		}
	);

	ivm_int_t i, j, size = ivm_list_size(ret);
	struct token_t *tmp_token;
	struct {
		const ivm_char_t *name;
		ivm_size_t len;
		enum token_id_t id;
	} keywords[] = {
#define KEYWORD(name, id) { (name), sizeof(name) - 1, (id) },
		KEYWORD("fn", T_FN)
		KEYWORD("if", T_IF)
		KEYWORD("elif", T_ELIF)
		KEYWORD("else", T_ELSE)
		KEYWORD("while", T_WHILE)
		KEYWORD("ret", T_RET)
		KEYWORD("cont", T_CONT)
		KEYWORD("break", T_BREAK)

		KEYWORD("clone", T_CLONE)
#undef KEYWORD
	};

	for (i = 0; i < size; i++) {
		tmp_token = (struct token_t *)ivm_list_at(ret, i);
		if (tmp_token->id == T_ID) {
			for (j = 0; j < IVM_ARRLEN(keywords); j++) {
				if (!IVM_STRNCMP(tmp_token->val, tmp_token->len,
								 keywords[j].name,
								 keywords[j].len)) {
					tmp_token->id = keywords[j].id;
				}
			}
		}
	}

	_ivm_parser_dumpToken(ret);

	return ret;
}

struct rule_val_t {
	union {
		ilang_gen_expr_t *expr;
		ilang_gen_expr_list_t *expr_list;
		ilang_gen_table_entry_t slot;
		ilang_gen_table_entry_list_t *slot_list;
		ilang_gen_param_list_t *param_list;
		ilang_gen_branch_t branch;
		ilang_gen_branch_list_t *branch_list;
		ilang_gen_trans_unit_t *unit;
	} u;
};
struct env_t {
	ilang_gen_trans_unit_t *unit;
};

#define R EXPECT_RULE
#define T EXPECT_TOKEN

#define DBB(...) { __VA_ARGS__; } // debug block

/*
	sep : newl
		| ';'
 */
RULE(sep)
{
	SUB_RULE_SET(
		SUB_RULE(T(T_NEWL))
		SUB_RULE(T(T_SEMIC))
	);

	FAILED({})
	MATCHED({})
}

/*
	sep_list_sub
		: sep sep_list_sub
		| %empty
 */
RULE(sep_list_sub)
{
	SUB_RULE_SET(
		SUB_RULE(R(sep) R(sep_list_sub))
		SUB_RULE()
	);

	FAILED({})
	MATCHED({})
}

/*
	sep_list
		: sep sep_list_sub
 */
RULE(sep_list)
{
	SUB_RULE_SET(
		SUB_RULE(R(sep) R(sep_list_sub))
	);

	FAILED({})
	MATCHED({})
}

/*
	sep_list_opt
		: sep sep_list_sub
		| %empty
 */
RULE(sep_list_opt)
{
	SUB_RULE_SET(
		SUB_RULE(R(sep) R(sep_list_sub))
		SUB_RULE()
	);

	FAILED({})
	MATCHED({})
}

/*
	newl_list_sub
		: newl newl_list_sub
		| %empty
 */
RULE(newl_list_sub)
{
	SUB_RULE_SET(
		SUB_RULE(T(T_NEWL) R(newl_list_sub))
		SUB_RULE()
	);

	FAILED({})
	MATCHED({})
}

/*
	nllo
		: newl newl_list_sub
		| %empty
 */
RULE(nllo)
{
	SUB_RULE_SET(
		SUB_RULE(T(T_NEWL) R(newl_list_sub))
		SUB_RULE()
	);

	FAILED({})
	MATCHED({})
}

#define TOKEN_VAL(token) (ilang_gen_token_value_build((token)->val, (token)->len))
#define TOKEN_POS(token) (ilang_gen_pos_build((token)->line, (token)->pos))

/*
	slot
		: id ':' prefix_expr
 */

RULE(prefix_expr);
RULE(slot)
{
	struct token_t *tmp_token;

	SUB_RULE_SET(
		SUB_RULE(T(T_ID) T(T_COLON) R(prefix_expr) R(nllo)
		{
			tmp_token = TOKEN_AT(0);

			_RETVAL.slot = ilang_gen_table_entry_build(
				TOKEN_POS(tmp_token),
				TOKEN_VAL(tmp_token),
				RULE_RET_AT(0).u.expr
			);
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	slot_list_sub
		: ',' slot slot_list_sub
		| %empty
 */
RULE(slot_list_sub)
{
	ilang_gen_table_entry_t tmp_entry;
	ilang_gen_table_entry_list_t *tmp_list;

	SUB_RULE_SET(
		SUB_RULE(T(T_COMMA) R(nllo) R(slot) R(slot_list_sub)
		{
			tmp_list
			= _RETVAL.slot_list
			= RULE_RET_AT(2).u.slot_list;
			tmp_entry = RULE_RET_AT(1).u.slot;

			ilang_gen_table_entry_list_push(tmp_list, &tmp_entry);
		})
		SUB_RULE({
			_RETVAL.slot_list = ilang_gen_table_entry_list_new(_ENV->unit);
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	slot_list_opt
		: slot slot_list_sub
		| %empty	
 */
RULE(slot_list_opt)
{
	ilang_gen_table_entry_t tmp_entry;
	ilang_gen_table_entry_list_t *tmp_list;

	SUB_RULE_SET(
		SUB_RULE(R(slot) R(slot_list_sub)
		{
			tmp_list
			= _RETVAL.slot_list
			= RULE_RET_AT(1).u.slot_list;
			tmp_entry = RULE_RET_AT(0).u.slot;

			ilang_gen_table_entry_list_push(tmp_list, &tmp_entry);
		})
		SUB_RULE({
			_RETVAL.slot_list = ilang_gen_table_entry_list_new(_ENV->unit);
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	primary_expr
		: string
		| int
		| float
		| id
		| '(' expr ')'
		| '{' expr_list '}'
		| '{' nllo slot_list_opt nllo '}'
 */
RULE(expr);
RULE(expr_list);
RULE(expr_list_opt);
RULE(primary_expr)
{
	ilang_gen_expr_list_t *tmp_expr_list = IVM_NULL;
	struct token_t *tmp_token;

	SUB_RULE_SET(
		SUB_RULE(T(T_STR) DBB(PRINT_MATCH_TOKEN("str expr"))
		{
			tmp_token = TOKEN_AT(0);
			_RETVAL.expr = ilang_gen_string_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token), TOKEN_VAL(tmp_token)
			);
		})

		SUB_RULE(T(T_INT) DBB(PRINT_MATCH_TOKEN("int expr"))
		{
			tmp_token = TOKEN_AT(0);
			_RETVAL.expr = ilang_gen_int_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token), TOKEN_VAL(tmp_token)
			);
		})

		SUB_RULE(T(T_FLOAT) DBB(PRINT_MATCH_TOKEN("float expr"))
		{
			tmp_token = TOKEN_AT(0);
			_RETVAL.expr = ilang_gen_float_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token), TOKEN_VAL(tmp_token)
			);
		})

		SUB_RULE(T(T_ID) DBB(PRINT_MATCH_TOKEN("id expr"))
		{
			tmp_token = TOKEN_AT(0);
			_RETVAL.expr = ilang_gen_id_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token), TOKEN_VAL(tmp_token)
			);
		})

		SUB_RULE(T(T_LPAREN) R(nllo)
				 R(expr)
				 R(nllo) T(T_RPAREN) DBB(PRINT_MATCH_TOKEN("paren expr"))
		{
			_RETVAL.expr = RULE_RET_AT(1).u.expr;
		})

		SUB_RULE(T(T_LBRAC)
				 R(expr_list)
				 T(T_RBRAC) DBB(PRINT_MATCH_TOKEN("expr block"))
		{
			tmp_expr_list = RULE_RET_AT(0).u.expr_list;
			tmp_token = TOKEN_AT(0);
			_RETVAL.expr = ilang_gen_expr_block_new(
				_ENV->unit,
				TOKEN_POS(tmp_token), tmp_expr_list
			);
		})

		SUB_RULE(T(T_LBRAC) R(nllo)
				 R(slot_list_opt)
				 R(nllo) T(T_RBRAC)
		{
			tmp_token = TOKEN_AT(0);
			_RETVAL.expr = ilang_gen_table_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token),
				RULE_RET_AT(1).u.slot_list
			);
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	arg_list_sub
		: ',' prefix_expr arg_list_sub
		| %empty
 */
RULE(prefix_expr);
RULE(arg_list_sub)
{
	ilang_gen_expr_list_t *tmp_list;
	struct rule_val_t tmp_ret;

	SUB_RULE_SET(
		SUB_RULE(T(T_COMMA) R(prefix_expr) R(arg_list_sub)
		{
			tmp_ret = RULE_RET_AT(0);
			tmp_list = _RETVAL.expr_list = RULE_RET_AT(1).u.expr_list;
			ilang_gen_expr_list_push(tmp_list, tmp_ret.u.expr);
		})

		SUB_RULE({
			_RETVAL.expr_list = ilang_gen_expr_list_new(_ENV->unit);
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	arg_list_opt
		: prefix_expr arg_list_sub
		| %empty
 */
RULE(arg_list_opt)
{
	ilang_gen_expr_list_t *tmp_list;
	struct rule_val_t tmp_ret;

	SUB_RULE_SET(
		SUB_RULE(R(prefix_expr) R(arg_list_sub)
		{
			tmp_ret = RULE_RET_AT(0);
			tmp_list = _RETVAL.expr_list = RULE_RET_AT(1).u.expr_list;
			ilang_gen_expr_list_push(tmp_list, tmp_ret.u.expr);
		})

		SUB_RULE({
			_RETVAL.expr_list = ilang_gen_expr_list_new(_ENV->unit);
		})
	);

	FAILED({})
	MATCHED({})
}

#define GET_OPERAND(expr, i) (((struct { ILANG_GEN_EXPR_HEADER ilang_gen_expr_t *tmp[i]; } *)(expr))->tmp[(i) - 1])
#define SET_OPERAND(expr, i, val) (GET_OPERAND((expr), (i)) = (val))

/*
	postfix_expr_sub
		: '(' arg_list_opt ')' postfix_expr_sub
		| '.' id postfix_expr_sub
		| %empty
 */
RULE(postfix_expr_sub)
{
	struct token_t *tmp_token, *id;
	ilang_gen_expr_t *tmp_expr = IVM_NULL;
	ilang_gen_expr_list_t *tmp_expr_list = IVM_NULL;

	SUB_RULE_SET(
		SUB_RULE(T(T_LPAREN) R(nllo)
				 R(arg_list_opt)
				 R(nllo) T(T_RPAREN) R(postfix_expr_sub) DBB(PRINT_MATCH_TOKEN("call expr"))
		{
			tmp_expr_list = RULE_RET_AT(1).u.expr_list;
			tmp_token = TOKEN_AT(0);
			tmp_expr = RULE_RET_AT(3).u.expr;

			if (tmp_expr) {
				_RETVAL.expr = tmp_expr;
				// find the innermost expression
				while (GET_OPERAND(tmp_expr, 1))
					tmp_expr = GET_OPERAND(tmp_expr, 1);

				SET_OPERAND(tmp_expr, 1, ilang_gen_call_expr_new(
					_ENV->unit,
					TOKEN_POS(tmp_token), IVM_NULL, tmp_expr_list
				));
			} else {
				_RETVAL.expr = ilang_gen_call_expr_new(
					_ENV->unit,
					TOKEN_POS(tmp_token), IVM_NULL, tmp_expr_list
				);
			}
		})

		SUB_RULE(T(T_DOT) R(nllo) T(T_ID) R(postfix_expr_sub) DBB(PRINT_MATCH_TOKEN("slot expr"))
		{
			tmp_token = TOKEN_AT(0);
			id = TOKEN_AT(1);
			tmp_expr = RULE_RET_AT(1).u.expr;

			if (tmp_expr) {
				_RETVAL.expr = tmp_expr;
				// find the innermost expression
				while (GET_OPERAND(tmp_expr, 1))
					tmp_expr = GET_OPERAND(tmp_expr, 1);

				SET_OPERAND(tmp_expr, 1, ilang_gen_slot_expr_new(
					_ENV->unit,
					TOKEN_POS(tmp_token), IVM_NULL, TOKEN_VAL(id)
				));
			} else {
				_RETVAL.expr = ilang_gen_slot_expr_new(
					_ENV->unit,
					TOKEN_POS(tmp_token), IVM_NULL, TOKEN_VAL(id)
				);
			}
		})

		SUB_RULE(T(T_ID) R(postfix_expr_sub) DBB(PRINT_MATCH_TOKEN("slot expr"))
		{
			id = tmp_token = TOKEN_AT(0);
			tmp_expr = RULE_RET_AT(0).u.expr;

			if (tmp_expr) {
				_RETVAL.expr = tmp_expr;
				// find the innermost expression
				while (GET_OPERAND(tmp_expr, 1))
					tmp_expr = GET_OPERAND(tmp_expr, 1);

				SET_OPERAND(tmp_expr, 1, ilang_gen_slot_expr_new(
					_ENV->unit,
					TOKEN_POS(tmp_token), IVM_NULL, TOKEN_VAL(id)
				));
			} else {
				_RETVAL.expr = ilang_gen_slot_expr_new(
					_ENV->unit,
					TOKEN_POS(tmp_token), IVM_NULL, TOKEN_VAL(id)
				);
			}
		})

		SUB_RULE(
		{
			_RETVAL.expr = IVM_NULL;
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	postfix_expr
		: primary_expr postfix_expr_sub
 */
RULE(postfix_expr)
{
	ilang_gen_expr_t *tmp_expr;

	SUB_RULE_SET(
		SUB_RULE(R(primary_expr) R(postfix_expr_sub)
		DBB(
			if (RULE_RET_AT(1).u.expr)
				PRINT_MATCH_TOKEN("postfix expr");
		)
		{
			tmp_expr = RULE_RET_AT(1).u.expr;
			// find the innermost expression
			if (tmp_expr) {
				_RETVAL.expr = tmp_expr;
				while (GET_OPERAND(tmp_expr, 1))
					tmp_expr = GET_OPERAND(tmp_expr, 1);

				SET_OPERAND(tmp_expr, 1, RULE_RET_AT(0).u.expr);
			} else {
				_RETVAL.expr = RULE_RET_AT(0).u.expr;
			}
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	unary_expr
		: postfix_expr
		| '!' unary_expr
		| 'clone' unary_expr
 */
RULE(unary_expr)
{
	struct token_t *tmp_token;

	SUB_RULE_SET(
		SUB_RULE(T(T_NOT) R(nllo) R(unary_expr)
		DBB(PRINT_MATCH_TOKEN("not expr"))
		{
			tmp_token = TOKEN_AT(0);
			_RETVAL.expr = ilang_gen_unary_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token),
				RULE_RET_AT(1).u.expr,
				IVM_UNIOP_ID(NOT)
			);
		})
		SUB_RULE(T(T_CLONE) R(nllo) R(unary_expr)
		DBB(PRINT_MATCH_TOKEN("clone expr"))
		{
			tmp_token = TOKEN_AT(0);
			_RETVAL.expr = ilang_gen_unary_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token),
				RULE_RET_AT(1).u.expr,
				IVM_UNIOP_ID(CLONE)
			);
		})
		SUB_RULE(R(postfix_expr)
		{
			_RETVAL.expr = RULE_RET_AT(0).u.expr;
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	mul_div_expr_sub
		: '*' unary_expr mul_div_expr_sub
		| '/' unary_expr mul_div_expr_sub
		| '%' unary_expr mul_div_expr_sub
		| %empty
 */

#define SUB1(op) \
	{                                                                   \
		struct token_t *tmp_token = TOKEN_AT(0);                        \
		ilang_gen_expr_t *tmp_expr = RULE_RET_AT(2).u.expr;             \
                                                                        \
		if (tmp_expr) {                                                 \
			_RETVAL.expr = tmp_expr;                                    \
			/* find the innermost expression */                         \
			while (GET_OPERAND(tmp_expr, 1))                            \
				tmp_expr = GET_OPERAND(tmp_expr, 1);                    \
                                                                        \
			SET_OPERAND(tmp_expr, 1, ilang_gen_binary_expr_new(         \
				_ENV->unit,                                             \
				TOKEN_POS(tmp_token), IVM_NULL,                         \
				RULE_RET_AT(1).u.expr, IVM_BINOP_ID(op)                 \
			));                                                         \
		} else {                                                        \
			_RETVAL.expr = ilang_gen_binary_expr_new(                   \
				_ENV->unit,                                             \
				TOKEN_POS(tmp_token), IVM_NULL,                         \
				RULE_RET_AT(1).u.expr, IVM_BINOP_ID(op)                 \
			);                                                          \
		}                                                               \
	}

#define SUB2() \
	{                                                                   \
		ilang_gen_expr_t *tmp_expr = RULE_RET_AT(1).u.expr;             \
                                                                        \
		if (tmp_expr) {                                                 \
			_RETVAL.expr = tmp_expr;                                    \
			/* find the innermost expression */                         \
			while (GET_OPERAND(tmp_expr, 1))                            \
				tmp_expr = GET_OPERAND(tmp_expr, 1);                    \
                                                                        \
			SET_OPERAND(tmp_expr, 1, RULE_RET_AT(0).u.expr);            \
		} else {                                                        \
			_RETVAL.expr = RULE_RET_AT(0).u.expr;                       \
		}                                                               \
	}

RULE(mul_div_expr_sub)
{
	SUB_RULE_SET(
		SUB_RULE(T(T_MUL) R(nllo) R(unary_expr) R(mul_div_expr_sub) SUB1(MUL))
		SUB_RULE(T(T_DIV) R(nllo) R(unary_expr) R(mul_div_expr_sub) SUB1(DIV))
		SUB_RULE(T(T_MOD) R(nllo) R(unary_expr) R(mul_div_expr_sub) SUB1(MOD))
		SUB_RULE({ _RETVAL.expr = IVM_NULL; })
	);

	FAILED({})
	MATCHED({})
}

/*
	mul_div_expr
		: unary_expr mul_div_expr_sub
 */
RULE(mul_div_expr)
{
	SUB_RULE_SET(
		SUB_RULE(R(unary_expr) R(mul_div_expr_sub)
		DBB(
			if (RULE_RET_AT(1).u.expr)
				PRINT_MATCH_TOKEN("mul/div/mod expr");
		) SUB2())
	);

	FAILED({})
	MATCHED({})
}

/*
	add_sub_expr_sub
		: '+' mul_div_expr add_sub_expr_sub
		| '-' mul_div_expr add_sub_expr_sub
		| %empty
 */
RULE(add_sub_expr_sub)
{
	SUB_RULE_SET(
		SUB_RULE(T(T_ADD) R(nllo) R(mul_div_expr) R(add_sub_expr_sub) SUB1(ADD))
		SUB_RULE(T(T_SUB) R(nllo) R(mul_div_expr) R(add_sub_expr_sub) SUB1(SUB))
		SUB_RULE({ _RETVAL.expr = IVM_NULL; })
	);

	FAILED({})
	MATCHED({})
}

/*
	add_sub_expr
		: mul_div_expr add_sub_expr_sub
 */
RULE(add_sub_expr)
{
	SUB_RULE_SET(
		SUB_RULE(R(mul_div_expr) R(add_sub_expr_sub)
		DBB(
			if (RULE_RET_AT(1).u.expr)
				PRINT_MATCH_TOKEN("add/sub expr");
		) SUB2())
	);

	FAILED({})
	MATCHED({})
}

#undef SUB1
#define SUB1(cmp_type) \
	{                                                                   \
		struct token_t *tmp_token = TOKEN_AT(0);                        \
		ilang_gen_expr_t *tmp_expr = RULE_RET_AT(2).u.expr;             \
                                                                        \
		if (tmp_expr) {                                                 \
			_RETVAL.expr = tmp_expr;                                    \
			/* find the innermost expression */                         \
			while (GET_OPERAND(tmp_expr, 1))                            \
				tmp_expr = GET_OPERAND(tmp_expr, 1);                    \
                                                                        \
			SET_OPERAND(tmp_expr, 1, ilang_gen_cmp_expr_new(            \
				_ENV->unit,                                             \
				TOKEN_POS(tmp_token), IVM_NULL,                         \
				RULE_RET_AT(1).u.expr, ILANG_GEN_CMP_##cmp_type         \
			));                                                         \
		} else {                                                        \
			_RETVAL.expr = ilang_gen_cmp_expr_new(                      \
				_ENV->unit,                                             \
				TOKEN_POS(tmp_token), IVM_NULL,                         \
				RULE_RET_AT(1).u.expr, ILANG_GEN_CMP_##cmp_type         \
			);                                                          \
		}                                                               \
	}

/*
	cmp_expr_sub
		: '>' add_sub_expr cmp_expr_sub
		| '>=' add_sub_expr cmp_expr_sub
		| '<' add_sub_expr cmp_expr_sub
		| '<=' add_sub_expr cmp_expr_sub
		| %empty
 */
RULE(cmp_expr_sub)
{
	SUB_RULE_SET(
		SUB_RULE(T(T_CGT) R(nllo) R(add_sub_expr) R(cmp_expr_sub) SUB1(GT))
		SUB_RULE(T(T_CGE) R(nllo) R(add_sub_expr) R(cmp_expr_sub) SUB1(GE))
		SUB_RULE(T(T_CLT) R(nllo) R(add_sub_expr) R(cmp_expr_sub) SUB1(LT))
		SUB_RULE(T(T_CLE) R(nllo) R(add_sub_expr) R(cmp_expr_sub) SUB1(LE))
		SUB_RULE({ _RETVAL.expr = IVM_NULL; })
	);

	FAILED({})
	MATCHED({})
}

/*
	cmp_expr
		: add_sub_expr cmp_expr_sub
 */
RULE(cmp_expr)
{
	SUB_RULE_SET(
		SUB_RULE(R(add_sub_expr) R(cmp_expr_sub)
		DBB(
			if (RULE_RET_AT(1).u.expr)
				PRINT_MATCH_TOKEN("cmp expr");
		) SUB2())
	);

	FAILED({})
	MATCHED({})
}

/*
	eq_expr_sub
		: '==' cmp_expr eq_expr_sub
		| '!=' cmp_expr eq_expr_sub
		| %empty
 */
RULE(eq_expr_sub)
{
	SUB_RULE_SET(
		SUB_RULE(T(T_CEQ) R(nllo) R(cmp_expr) R(eq_expr_sub) SUB1(EQ))
		SUB_RULE(T(T_CNE) R(nllo) R(cmp_expr) R(eq_expr_sub) SUB1(NE))
		SUB_RULE({ _RETVAL.expr = IVM_NULL; })
	);

	FAILED({})
	MATCHED({})
}

/*
	eq_expr
		: cmp_expr eq_expr_sub
 */
RULE(eq_expr)
{
	SUB_RULE_SET(
		SUB_RULE(R(cmp_expr) R(eq_expr_sub)
		DBB(
			if (RULE_RET_AT(1).u.expr)
				PRINT_MATCH_TOKEN("eq/ne expr");
		) SUB2())
	);

	FAILED({})
	MATCHED({})
}

#undef SUB1
#undef SUB2

RULE(prefix_expr);

/*
	param_list_sub
		: ',' id param_list_sub
		| %empty
 */
RULE(param_list_sub)
{
	ilang_gen_param_list_t *tmp_list;
	ilang_gen_token_value_t tmp_value;
	struct token_t *tmp_token;

	SUB_RULE_SET(
		SUB_RULE(T(T_COMMA) R(nllo) T(T_ID) R(param_list_sub)
		{
			tmp_token = TOKEN_AT(1);
			tmp_list = _RETVAL.param_list = RULE_RET_AT(1).u.param_list;
			tmp_value = TOKEN_VAL(tmp_token);
			ilang_gen_param_list_push(tmp_list, &tmp_value);
		})
		SUB_RULE({
			_RETVAL.param_list = ilang_gen_param_list_new(_ENV->unit);
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	param_list_opt
		: id param_list_sub
		| %empty
 */
RULE(param_list_opt)
{
	ilang_gen_param_list_t *tmp_list;
	ilang_gen_token_value_t tmp_value;
	struct token_t *tmp_token;

	SUB_RULE_SET(
		SUB_RULE(T(T_ID) R(param_list_sub)
		{
			tmp_token = TOKEN_AT(0);
			tmp_list = _RETVAL.param_list = RULE_RET_AT(0).u.param_list;
			tmp_value = TOKEN_VAL(tmp_token);
			ilang_gen_param_list_push(tmp_list, &tmp_value);
		})
		SUB_RULE({
			_RETVAL.param_list = ilang_gen_param_list_new(_ENV->unit);
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	elif_branch
		: 'elif' nllo expr nllo ':' nllo prefix_expr
 */
RULE(elif_branch)
{
	SUB_RULE_SET(
		SUB_RULE(R(nllo) T(T_ELIF) R(nllo)
				 R(expr)
				 R(nllo) T(T_COLON) R(nllo) R(prefix_expr)
				 DBB(PRINT_MATCH_TOKEN("elif branch"))
		{
			_RETVAL.branch = ilang_gen_branch_build(
				RULE_RET_AT(2).u.expr, RULE_RET_AT(5).u.expr
			);
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	elif_list_opt
		: elif_branch nllo elif_list_opt
		| %empty
 */
RULE(elif_list_opt)
{
	ilang_gen_branch_t tmp_br;
	ilang_gen_branch_list_t *tmp_list;

	SUB_RULE_SET(
		SUB_RULE(R(elif_branch) R(elif_list_opt)
		{
			tmp_list
			= _RETVAL.branch_list
			= RULE_RET_AT(1).u.branch_list;
			tmp_br = RULE_RET_AT(0).u.branch;

			ilang_gen_branch_list_push(tmp_list, &tmp_br);
		})
		SUB_RULE({
			_RETVAL.branch_list = ilang_gen_branch_list_new(_ENV->unit);
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	else_branch_opt
		: 'else' nllo ':' nllo prefix_expr
		| %empty
 */
RULE(else_branch_opt)
{
	SUB_RULE_SET(
		SUB_RULE(R(nllo) T(T_ELSE) R(nllo)
				 T(T_COLON) R(nllo) R(prefix_expr)
				 DBB(PRINT_MATCH_TOKEN("else branch"))
		{
			_RETVAL.branch = ilang_gen_branch_build(
				IVM_NULL, RULE_RET_AT(3).u.expr
			);
		})
		SUB_RULE()
	);

	FAILED({})
	MATCHED({})
}

/*
	if_expr
		: 'if' nllo expr nllo ':' nllo prefix_expr elif_list_opt else_branch_opt
 */
RULE(if_expr)
{
	struct token_t *tmp_token;
	ilang_gen_expr_t *tmp_expr = IVM_NULL;

	SUB_RULE_SET(
		SUB_RULE(T(T_IF) R(nllo) R(expr)
				 R(nllo) T(T_COLON) R(nllo)
				 R(prefix_expr) DBB(PRINT_MATCH_TOKEN("if branch"))
				 R(elif_list_opt)
				 R(else_branch_opt)
		{
			tmp_expr = RULE_RET_AT(1).u.expr;
			tmp_token = TOKEN_AT(0);
			_RETVAL.expr = ilang_gen_if_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token),
				ilang_gen_branch_build(tmp_expr, RULE_RET_AT(4).u.expr), /* main branch */
				RULE_RET_AT(5).u.branch_list,
				RULE_RET_AT(6).u.branch
			);
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	while_expr
		: 'while' nllo expr nllo ':' nllo preifx_expr
 */
RULE(while_expr)
{
	struct token_t *tmp_token;

	SUB_RULE_SET(
		SUB_RULE(T(T_WHILE) R(nllo)
				 R(expr) R(nllo)
				 T(T_COLON) R(nllo)
				 R(prefix_expr)
		{
			tmp_token = TOKEN_AT(0);
			_RETVAL.expr = ilang_gen_while_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token),
				RULE_RET_AT(1).u.expr,
				RULE_RET_AT(4).u.expr
			);
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	fn_expr
		: fn param_list_opt ':' prefix_expr
 */
RULE(fn_expr)
{
	struct token_t *tmp_token;
	ilang_gen_param_list_t *tmp_list = IVM_NULL;

	SUB_RULE_SET(
		SUB_RULE(T(T_FN) R(nllo)
				 R(param_list_opt)
				 R(nllo) T(T_COLON) R(nllo) R(prefix_expr) DBB(PRINT_MATCH_TOKEN("fn expr"))
		{
			tmp_list = RULE_RET_AT(1).u.param_list;
			tmp_token = TOKEN_AT(0);
			_RETVAL.expr = ilang_gen_fn_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token),
				tmp_list, RULE_RET_AT(4).u.expr
			);
		})

		SUB_RULE(R(if_expr) DBB(PRINT_MATCH_TOKEN("if expr"))
		{
			_RETVAL.expr = RULE_RET_AT(0).u.expr;
		})

		SUB_RULE(R(while_expr) DBB(PRINT_MATCH_TOKEN("while expr"))
		{
			_RETVAL.expr = RULE_RET_AT(0).u.expr;
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	ret_expr:
		: 'ret' prefix_expr
		| 'ret'
 */
RULE(ret_expr)
{
	struct token_t *tmp_token;

	SUB_RULE_SET(
		SUB_RULE(T(T_RET) R(prefix_expr)
		{
			tmp_token = TOKEN_AT(0);
			_RETVAL.expr = ilang_gen_intr_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token), ILANG_GEN_INTR_RET, RULE_RET_AT(0).u.expr
			);
		})
		SUB_RULE(T(T_RET)
		{
			tmp_token = TOKEN_AT(0);
			_RETVAL.expr = ilang_gen_intr_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token), ILANG_GEN_INTR_RET, IVM_NULL
			);
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	cont_expr:
		: 'cont' prefix_expr
		| 'cont'
 */
RULE(cont_expr)
{
	struct token_t *tmp_token;

	SUB_RULE_SET(
		SUB_RULE(T(T_CONT) R(prefix_expr)
		{
			tmp_token = TOKEN_AT(0);
			_RETVAL.expr = ilang_gen_intr_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token), ILANG_GEN_INTR_CONT, RULE_RET_AT(0).u.expr
			);
		})
		SUB_RULE(T(T_CONT)
		{
			tmp_token = TOKEN_AT(0);
			_RETVAL.expr = ilang_gen_intr_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token), ILANG_GEN_INTR_CONT, IVM_NULL
			);
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	break_expr:
		: 'break' prefix_expr
		| 'break'
 */
RULE(break_expr)
{
	struct token_t *tmp_token;

	SUB_RULE_SET(
		SUB_RULE(T(T_BREAK) R(prefix_expr)
		{
			tmp_token = TOKEN_AT(0);
			_RETVAL.expr = ilang_gen_intr_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token), ILANG_GEN_INTR_BREAK, RULE_RET_AT(0).u.expr
			);
		})
		SUB_RULE(T(T_BREAK)
		{
			tmp_token = TOKEN_AT(0);
			_RETVAL.expr = ilang_gen_intr_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token), ILANG_GEN_INTR_BREAK, IVM_NULL
			);
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	intr_expr:
		: ret_expr
		| cont_expr
		| break_expr
 */
RULE(intr_expr)
{
	SUB_RULE_SET(
		SUB_RULE(R(ret_expr))
		SUB_RULE(R(cont_expr))
		SUB_RULE(R(break_expr))
	);

	FAILED({})
	MATCHED({
		_RETVAL.expr = RULE_RET_AT(0).u.expr;
	})
}

/*
	assign_expr
		: eq_expr '=' prefix_expr
 */
RULE(assign_expr)
{
	struct token_t *tmp_token;

	SUB_RULE_SET(
		SUB_RULE(R(eq_expr)
				 T(T_ASSIGN) R(nllo) R(prefix_expr)
		{
			tmp_token = TOKEN_AT(0);
			_RETVAL.expr = ilang_gen_assign_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token),
				RULE_RET_AT(0).u.expr,
				RULE_RET_AT(2).u.expr
			);
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	prefix_expr
		: assign_expr
		| intr_expr
		| fn_expr
		| eq_expr
 */

RULE(prefix_expr)
{
	SUB_RULE_SET(
		SUB_RULE(R(assign_expr) DBB(PRINT_MATCH_TOKEN("assign expr")))
		SUB_RULE(R(intr_expr) DBB(PRINT_MATCH_TOKEN("intr expr")))
		SUB_RULE(R(fn_expr))
		SUB_RULE(R(eq_expr))
	);

	FAILED({})
	MATCHED({
		_RETVAL.expr = RULE_RET_AT(0).u.expr;
	})
}

/*
	comma_expr_sub
		: ',' prefix_expr comma_expr_sub
		| %empty
 */
RULE(comma_expr_sub)
{
	ilang_gen_expr_list_t *tmp_expr_list;

	SUB_RULE_SET(
		SUB_RULE(T(T_COMMA) R(nllo) R(prefix_expr) R(comma_expr_sub)
		{
			tmp_expr_list
			= RULE_RET_AT(2).u.expr_list;

			if (tmp_expr_list) {
				_RETVAL.expr_list = tmp_expr_list;
				ilang_gen_expr_list_push(tmp_expr_list, RULE_RET_AT(1).u.expr);
			} else {
				tmp_expr_list = _RETVAL.expr_list = ilang_gen_expr_list_new(_ENV->unit);
				ilang_gen_expr_list_push(tmp_expr_list, RULE_RET_AT(1).u.expr);
			}
		})

		SUB_RULE({
			_RETVAL.expr_list = IVM_NULL;
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	comma_expr
		: prefix_expr comma_expr_sub
 */
RULE(comma_expr)
{
	ilang_gen_expr_list_t *tmp_expr_list;
	ilang_gen_expr_t *tmp_expr;

	SUB_RULE_SET(
		SUB_RULE(R(prefix_expr) R(comma_expr_sub)
		DBB(
			if (RULE_RET_AT(1).u.expr_list)
				DBB(PRINT_MATCH_TOKEN("comma expr"))
		)
		{
			tmp_expr_list = RULE_RET_AT(1).u.expr_list;
			tmp_expr = RULE_RET_AT(0).u.expr;

			if (tmp_expr_list) {
				ilang_gen_expr_list_push(tmp_expr_list, tmp_expr);

				_RETVAL.expr = ilang_gen_expr_block_new(
					_ENV->unit,
					tmp_expr->pos, tmp_expr_list
				);
			} else {
				_RETVAL.expr = tmp_expr;
			}
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	expr
		: intr_expr
 */
RULE(expr)
{
	SUB_RULE_SET(
		SUB_RULE(R(comma_expr))
	);

	FAILED({})
	MATCHED({
		_RETVAL.expr = RULE_RET_AT(0).u.expr;
	})
}

/*
	expr_list_sub
		: sep_list expr expr_list_sub
		| %empty
 */
RULE(expr_list_sub)
{
	ilang_gen_expr_list_t *tmp_expr_list;

	SUB_RULE_SET(
		SUB_RULE(R(sep_list) CLEAR_ERR() R(expr)
				 DBB(IVM_TRACE("********* expr matched *********\n"))
				 R(expr_list_sub)
		{
			tmp_expr_list
			= _RETVAL.expr_list
			= RULE_RET_AT(2).u.expr_list;

			ilang_gen_expr_list_push(tmp_expr_list, RULE_RET_AT(1).u.expr);
		})
		
		SUB_RULE(R(sep_list)
		{
			_RETVAL.expr_list = ilang_gen_expr_list_new(_ENV->unit);
		})

		SUB_RULE({
			_RETVAL.expr_list = ilang_gen_expr_list_new(_ENV->unit);
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	expr_list
		: expr expr_list_sub
 */
RULE(expr_list)
{
	ilang_gen_expr_list_t *tmp_expr_list;
	ilang_gen_expr_t *tmp_expr;

	SUB_RULE_SET(
		SUB_RULE(R(sep_list_opt) CLEAR_ERR() R(expr)
				 DBB(IVM_TRACE("********* expr matched *********\n"))
				 R(expr_list_sub)
		{
			tmp_expr_list = _RETVAL.expr_list = RULE_RET_AT(2).u.expr_list;
			tmp_expr = RULE_RET_AT(1).u.expr;
			ilang_gen_expr_list_push(tmp_expr_list, tmp_expr);
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	expr_list_opt
		: expr expr_list_sub
		| %empty
 */
RULE(expr_list_opt)
{
	ilang_gen_expr_list_t *tmp_expr_list;
	ilang_gen_expr_t *tmp_expr;

	SUB_RULE_SET(
		SUB_RULE(R(sep_list_opt) CLEAR_ERR() R(expr)
				 DBB(IVM_TRACE("********* expr matched *********\n"))
				 R(expr_list_sub)
		{
			tmp_expr_list = _RETVAL.expr_list = RULE_RET_AT(2).u.expr_list;
			tmp_expr = RULE_RET_AT(1).u.expr;
			ilang_gen_expr_list_push(tmp_expr_list, tmp_expr);
		})

		SUB_RULE(R(sep_list_opt)
		{
			_RETVAL.expr_list = ilang_gen_expr_list_new(_ENV->unit);
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	eof_opt
		: EOF
		| %empty
 */
RULE(eof_opt)
{
	SUB_RULE_SET(
		SUB_RULE(T(T_EOF))
		SUB_RULE()
	);

	FAILED({})
	MATCHED({})
}

/*
	trans_unit
		: expr_list_opt
 */

RULE(trans_unit)
{
	SUB_RULE_SET(
		SUB_RULE(R(expr_list_opt) R(eof_opt)
		{
			ilang_gen_trans_unit_setTopLevel(
				_ENV->unit,
				RULE_RET_AT(0).u.expr_list
			);
		})
	);

	FAILED({
		POP_ERR();
	});

	MATCHED({
		if (HAS_NEXT_TOKEN()) {
			POP_ERR();
		}
	});
}

ilang_gen_trans_unit_t *
_ivm_parser_parseToken(ivm_list_t *tokens,
					   ivm_bool_t *suc)
{
	struct rule_val_t rule_ret;
	ilang_gen_trans_unit_t *ret = ilang_gen_trans_unit_new();
	struct env_t env = { ret };

	RULE_START(trans_unit, &env, &rule_ret, tokens, *suc);

	return ret;
}
