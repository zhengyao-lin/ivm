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

#include "gen/gen.h"
#include "parser.h"

enum token_id_t {
	T_NONE = 0,
	T_UNKNOWN,
	T_ID,
	T_STR_ID,
	T_INT,
	T_FLOAT,
	T_STR,

	/* keywords */
	T_MISSING,
	T_FN,
	T_IF,
	T_ELIF,
	T_ELSE,
	T_WHILE,
	T_FOR,
	T_IN,
	T_TRY,
	T_CATCH,
	T_FINAL,
	T_RET,
	T_CONT,
	T_BREAK,
	T_RAISE,
	T_YIELD,
	T_RESUME,
	T_EXPAND,

	T_WITH,
	T_TO,

	T_FORK,

	T_DEL,
	T_REF,
	T_DEREF,

	T_IS,

	T_IMPORT,

	T_SEMIC,	// ;
	T_COMMA,	// ,
	T_COLON,	// :
	T_ELLIP,	// ...
	T_DOT,		// .
	T_AT,		// @
	T_QM,		// ?

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

	T_BAND,		// &
	T_BIOR,		// |
	T_BEOR,		// ^
	T_SHL,		// <<
	T_SHAR,		// >>
	T_SHLR,		// >>>

	T_NOT,		// !

	T_CGT,		// >
	T_CGE,		// >=
	T_CLT,		// <
	T_CLE,		// <=
	T_CEQ,		// ==
	T_CNE,		// !=

	T_CAND,		// &&
	T_COR,		// ||

	T_NEWL,
	T_EOF
};

IVM_PRIVATE
const ivm_char_t *
token_name_table[] = {
	IVM_NULL,
	"unknown character",
	"identifier",
	"string identifier",
	"integer",
	"float",
	"string",
	
	"partial applied mark",
	"keyword `fn`",
	"keyword `if`",
	"keyword `elif`",
	"keyword `else`",
	"keyword `while`",
	"keyword `for`",
	"keyword `in`",
	"keyword `try`",
	"keyword `catch`",
	"keyword `final`",
	"keyword `ret`",
	"keyword `cont`",
	"keyword `break`",
	"keyword `raise`",
	"keyword `yield`",
	"keyword `resume`",
	"keyword `expand`",

	"keyword `with`",
	"keyword `to`",

	"keyword `fork`",

	"operator `del`",
	"operator `ref`",
	"operator `deref`",

	"operator `is`",

	"keyword `import`",

	"semicolon",
	"comma",
	"colon",
	"ellipsis",
	"dot",
	"at",
	"question mark",

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

	"bit and operator",
	"bit inclusive or operator",
	"bit exclusive or operator",
	"shift left",
	"shift arithmetic right",
	"shift logic right",

	"not operator",

	"greater-than operator",
	"greater-than-or-equal operator",
	"less-than operator",
	"less-than-or-equal operator",
	"equal operator",
	"not equal operator",

	"logic and operator",
	"logic or operator",

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

	ST_TRY_NEWL,

	ST_TRY_GT,
	ST_TRY_LT,
	ST_TRY_NOT,
	ST_TRY_ASSIGN,
	ST_TRY_AND,
	ST_TRY_OR,
	ST_TRY_SHR,

	ST_TRY_DOT,
	ST_TRY_ELLIP,

	ST_TRY_COMMENT12,

	ST_IN_COMMENT1,
	ST_OUT_COMMENT1,

	ST_IN_COMMENT2,

	ST_IN_ID,
	
	ST_IN_STR_ID,
	ST_IN_STR_ID_ESC,
	
	ST_IN_STR,
	ST_IN_STR_ESC,
	
	ST_IN_NUM_INT,
	ST_IN_NUM_FLOAT,

	ST_IN_NUM_HEX,
	ST_IN_NUM_OCT,
	ST_IN_NUM_BIN,

	STATE_COUNT,
	STATE_ERR
};

ivm_list_t *
_ilang_parser_getTokens(const ivm_char_t *src,
						ivm_bool_t debug)
{
	ivm_list_t *ret = TOKENIZE(src,
		/* INIT */
		{
			{ "-az", ST_IN_ID },
			{ "-AZ", ST_IN_ID },
			{ "=_", ST_IN_ID },
			{ "=$", ST_IN_ID },
			{ "$", ST_IN_ID },

			{ "-09", ST_IN_NUM_INT },
			{ "=.", ST_TRY_DOT },

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

			{ "=^", ST_INIT, T_BEOR },
			{ "=@", ST_INIT, T_AT },
			{ "=?", ST_INIT, T_QM },

			{ "=!", ST_TRY_NOT },

			{ "=>", ST_TRY_GT },
			{ "=<", ST_TRY_LT },
			{ "==", ST_TRY_ASSIGN },

			{ "=&", ST_TRY_AND },
			{ "=|", ST_TRY_OR },
			
			{ "=\n", ST_INIT, T_NEWL },
			{ "=\r", ST_TRY_NEWL },

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

		/* TRY_NEWL */
		{
			/* check \r\n */
			{ "=\n", ST_INIT, T_NEWL, .ext = IVM_TRUE, .exc = IVM_TRUE },
			{ ".", ST_INIT, T_NEWL },
		},

		/* TRY_GT */
		{
			{ "==", ST_INIT, T_CGE, .ext = IVM_TRUE, .exc = IVM_TRUE },
			{ "=>", ST_TRY_SHR },
			{ ".", ST_INIT, T_CGT },
		},

		/* TRY_LT */
		{
			{ "==", ST_INIT, T_CLE, .ext = IVM_TRUE, .exc = IVM_TRUE },
			{ "=<", ST_INIT, T_SHL, .ext = IVM_TRUE, .exc = IVM_TRUE },
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

		/* TRY_AND */
		{
			{ "=&", ST_INIT, T_CAND, .ext = IVM_TRUE, .exc = IVM_TRUE },
			{ ".", ST_INIT, T_BAND }
		},

		/* TRY_OR */
		{
			{ "=|", ST_INIT, T_COR, .ext = IVM_TRUE, .exc = IVM_TRUE },
			{ ".", ST_INIT, T_BIOR }
		},

		/* TRY_SHR */
		{
			{ "=>", ST_INIT, T_SHLR, .ext = IVM_TRUE, .exc = IVM_TRUE },
			{ ".", ST_INIT, T_SHAR }
		},

		/* TRY_DOT */
		{
			{ "-09", ST_IN_NUM_FLOAT },
			{ "=.", ST_TRY_ELLIP },
			{ ".", ST_INIT, T_DOT }
		},

		/* TRY_ELLIP */
		{
			{ "=.", ST_INIT, T_ELLIP, .ext = IVM_TRUE, .exc = IVM_TRUE }
		},

		/* TRY_COMMENT12 */
		{
			{ "=*", ST_IN_COMMENT1 },
			{ "=/", ST_IN_COMMENT2 },
			{ ".", ST_INIT, T_DIV }
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
			{ "=$", ST_IN_ID },
			{ "-09", ST_IN_ID },
			{ "$", ST_IN_ID },
			{ ".", ST_INIT, T_ID } /* revert one char */
		},

		/* IN_STR_ID */
		{
			{ "=`", ST_INIT, T_STR_ID, .exc = IVM_TRUE },
			{ "=\\", ST_IN_STR_ID_ESC },
			{ ".", ST_IN_STR_ID }
		},

		/* IN_STR_ID_ESC */
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
			{ "|xX", ST_IN_NUM_HEX },
			{ "|oO", ST_IN_NUM_OCT },
			{ "|bB", ST_IN_NUM_BIN },
			{ "=.", ST_IN_NUM_FLOAT },
			{ ".", ST_INIT, T_INT }
		},

		/* IN_NUM_FLOAT */
		{
			{ "-09", ST_IN_NUM_FLOAT },
			{ ".", ST_INIT, T_FLOAT }
		},

		/* IN_NUM_HEX */
		{
			{ "-09", ST_IN_NUM_HEX },
			{ "-af", ST_IN_NUM_HEX },
			{ "-AF", ST_IN_NUM_HEX },
			{ ".", ST_INIT, T_INT }
		},

		/* IN_NUM_OCT */
		{
			{ "-07", ST_IN_NUM_OCT },
			{ "|89", ST_INIT, .ign = IVM_TRUE, .msg = "illegal digit for octal literal" },
			{ ".", ST_INIT, T_INT }
		},

		/* IN_NUM_BIN */
		{
			{ "|01", ST_IN_NUM_BIN },
			{ "-29", ST_INIT, .ign = IVM_TRUE, .msg = "illegal digit for binary literal" },
			{ ".", ST_INIT, T_INT }
		}
	);

	if (!ret) return IVM_NULL;

	ivm_int_t i, j, size = ivm_list_size(ret);
	struct token_t *tmp_token;
	struct {
		const ivm_char_t *name;
		ivm_size_t len;
		enum token_id_t id;
	} keywords[] = {
#define KEYWORD(name, id) { (name), sizeof(name) - 1, (id) },
		KEYWORD("_", T_MISSING)

		KEYWORD("fn", T_FN)
		KEYWORD("if", T_IF)
		KEYWORD("elif", T_ELIF)
		KEYWORD("else", T_ELSE)
		KEYWORD("while", T_WHILE)
		KEYWORD("for", T_FOR)
		KEYWORD("in", T_IN)
		KEYWORD("try", T_TRY)
		KEYWORD("catch", T_CATCH)
		KEYWORD("final", T_FINAL)
		KEYWORD("ret", T_RET)
		KEYWORD("cont", T_CONT)
		KEYWORD("break", T_BREAK)
		KEYWORD("raise", T_RAISE)
		KEYWORD("yield", T_YIELD)
		KEYWORD("resume", T_RESUME)
		KEYWORD("expand", T_EXPAND)

		KEYWORD("with", T_WITH)
		KEYWORD("to", T_TO)

		KEYWORD("fork", T_FORK)

		KEYWORD("del", T_DEL)
		KEYWORD("ref", T_REF)
		KEYWORD("deref", T_DEREF)

		KEYWORD("is", T_IS)

		KEYWORD("import", T_IMPORT)
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
		} else if (tmp_token->id == T_STR_ID) {
			tmp_token->id = T_ID;
		}
	}

	if (debug)
		_ivm_parser_dumpToken(ret);

	return ret;
}

struct rule_val_t {
	union {
		ilang_gen_expr_t *expr;
		ilang_gen_expr_list_t *expr_list;
		ilang_gen_token_value_list_t *token_list;
		ilang_gen_token_value_t token;
		ilang_gen_table_entry_t slot;
		ilang_gen_table_entry_list_t *slot_list;
		ilang_gen_param_t param;
		ilang_gen_param_list_t *param_list;
		ilang_gen_branch_t branch;
		ilang_gen_branch_list_t *branch_list;
		ilang_gen_catch_branch_t catch_branch;
		ilang_gen_trans_unit_t *unit;
		ivm_int_t oop;
	} u;
};

struct env_t {
	ilang_gen_trans_unit_t *unit;
	ivm_bool_t debug;
};

#define R EXPECT_RULE
#define R_LIST EXPECT_RULE_LIST

#define T EXPECT_TOKEN
#define T_OPT EXPECT_TOKEN_OPT

#define DBB(...) { if (_ENV->debug) { __VA_ARGS__; } } // debug block

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
	sep_list
		: list(sep)
 */
RULE(sep_list)
{
	SUB_RULE_SET(
		SUB_RULE(R_LIST(sep, {}))
	);

	FAILED({})
	MATCHED({})
}

/*
	sep_list_opt
		: list(sep)
		| %empty
 */
RULE(sep_list_opt)
{
	SUB_RULE_SET(
		SUB_RULE(R_LIST(sep, {}))
		SUB_RULE()
	);

	FAILED({})
	MATCHED({})
}

/*
	newl_list_single
		: newl
 */
RULE(newl_list_single)
{
	SUB_RULE_SET(
		SUB_RULE(T(T_NEWL))
	);

	FAILED({})
	MATCHED({})
}

/*
	nllo
		: list(newl_list_single)
		| %empty
 */
RULE(nllo)
{
	SUB_RULE_SET(
		SUB_RULE(R_LIST(newl_list_single, {}))
		SUB_RULE()
	);

	FAILED({})
	MATCHED({})
}

#define TOKEN_VAL(token) (ilang_gen_token_value_build((token)->val, (token)->len))
#define TOKEN_VAL_EMPTY() (ilang_gen_token_value_build(IVM_NULL, 0))
#define TOKEN_POS(token) (ilang_gen_pos_build((token)->line, (token)->pos))

/*
	slot
		: id nllo ':' nllo prefix_expr
		| oop nllo ':' nllo prefix_expr
		| '.' nllo id nllo ':' nllo prefix_expr
		| '.' nllo oop nllo ':' nllo prefix_expr
 */

RULE(arg_list_opt);
RULE(prefix_expr);
RULE(oop);
RULE(slot)
{
	struct token_t *tmp_token;

	SUB_RULE_SET(
		SUB_RULE(T(T_ID) R(nllo) T(T_COLON) R(nllo) R(prefix_expr)
		{
			tmp_token = TOKEN_AT(0);

			_RETVAL.slot = ilang_gen_table_entry_build(
				TOKEN_POS(tmp_token),
				TOKEN_VAL(tmp_token),
				RULE_RET_AT(2).u.expr
			);
		})

		SUB_RULE(R(oop) R(nllo) T(T_COLON) R(nllo) R(prefix_expr)
		{
			tmp_token = TOKEN_AT(0);

			_RETVAL.slot = ilang_gen_table_entry_build_oop(
				TOKEN_POS(tmp_token),
				RULE_RET_AT(0).u.oop,
				RULE_RET_AT(3).u.expr
			);
		})

		SUB_RULE(T(T_DOT) R(nllo) T(T_ID) R(nllo) T(T_COLON) R(nllo) R(prefix_expr)
		{
			tmp_token = TOKEN_AT(1);

			_RETVAL.slot = ilang_gen_table_entry_build(
				TOKEN_POS(tmp_token),
				TOKEN_VAL(tmp_token),
				RULE_RET_AT(3).u.expr
			);
		})

		SUB_RULE(T(T_DOT) R(nllo) R(oop) R(nllo) T(T_COLON) R(nllo) R(prefix_expr)
		{
			tmp_token = TOKEN_AT(0);

			_RETVAL.slot = ilang_gen_table_entry_build_oop(
				TOKEN_POS(tmp_token),
				RULE_RET_AT(1).u.oop,
				RULE_RET_AT(4).u.expr
			);
		})

		SUB_RULE(T(T_LBRAKT) R(nllo) R(arg_list_opt) R(nllo) T(T_RBRAKT)
				 R(nllo) T(T_COLON) R(nllo) R(prefix_expr)
		{
			tmp_token = TOKEN_AT(0);

			_RETVAL.slot = ilang_gen_table_entry_build_index(
				TOKEN_POS(tmp_token),
				RULE_RET_AT(1).u.expr_list,
				RULE_RET_AT(5).u.expr
			);
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	slot_list_single
		: ',' nllo slot
 */
RULE(slot_list_single)
{
	SUB_RULE_SET(
		SUB_RULE(T(T_COMMA) R(nllo) R(slot) R(nllo)
		{
			_RETVAL.slot = RULE_RET_AT(1).u.slot;
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	slot_list_sub
		: list(slot_list_single)
		| %empty
 */
RULE(slot_list_sub)
{
	ilang_gen_table_entry_list_t *tmp_list = IVM_NULL;
	ilang_gen_table_entry_t tmp_entry;

	SUB_RULE_SET(
		SUB_RULE(R_LIST(slot_list_single, {
			if (!tmp_list) {
				tmp_list = ilang_gen_table_entry_list_new(_ENV->unit);
			}

			tmp_entry = RULE_RET_AT(0).u.slot;
			ilang_gen_table_entry_list_push(tmp_list, &tmp_entry);
		}) {
			ilang_gen_table_entry_list_reverse(tmp_list);
			_RETVAL.slot_list = tmp_list;
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
	mod_name_single
		: '.' nllo id
 */
RULE(mod_name_single)
{
	SUB_RULE_SET(
		SUB_RULE(T(T_DOT) R(nllo) T(T_ID)
		{
			_RETVAL.token = TOKEN_VAL(TOKEN_AT(1));
		})
	)

	FAILED({})
	MATCHED({})
}

/*
	mod_name_sub:
		: list(mod_name_single)
		| %empty
 */
RULE(mod_name_sub)
{
	ilang_gen_token_value_list_t *tokens = IVM_NULL;
	ilang_gen_token_value_t tmp_token;

	SUB_RULE_SET(
		SUB_RULE(R_LIST(mod_name_single, {
			if (!tokens) {
				tokens = ilang_gen_token_value_list_new(_ENV->unit);
			}

			tmp_token = RULE_RET_AT(0).u.token;
			ilang_gen_token_value_list_push(tokens, &tmp_token);
		}) {
			ilang_gen_token_value_list_reverse(tokens);
			_RETVAL.token_list = tokens;
		})

		SUB_RULE({
			_RETVAL.token_list = ilang_gen_token_value_list_new(_ENV->unit);
		})
	)

	FAILED({})
	MATCHED({})
}

/*
	mod_name
		: id mod_name_sub
 */
RULE(mod_name)
{
	ilang_gen_token_value_list_t *tokens;
	ilang_gen_token_value_t tmp_token;

	SUB_RULE_SET(
		SUB_RULE(T(T_ID) R(mod_name_sub)
		{
			tokens
			= _RETVAL.token_list
			= RULE_RET_AT(0).u.token_list;
			tmp_token = TOKEN_VAL(TOKEN_AT(0));

			ilang_gen_token_value_list_push(tokens, &tmp_token);
		})
	)

	FAILED({})
	MATCHED({})
}

/*
	for_postfix
		: 'for' nllo leftval nllo 'in' nllo logic_or_expr
 */
RULE(leftval);
RULE(logic_or_expr);
RULE(for_postfix)
{
	struct token_t *tmp_token;

	SUB_RULE_SET(
		SUB_RULE(T(T_FOR) R(nllo)
				 R(leftval) R(nllo)
				 T(T_IN) R(nllo)
				 R(logic_or_expr)
		{
			tmp_token = TOKEN_AT(0);
			_RETVAL.expr = ilang_gen_for_expr_new(
				_ENV->unit, TOKEN_POS(tmp_token),
				RULE_RET_AT(1).u.expr,
				RULE_RET_AT(4).u.expr,
				IVM_NULL
			);
		})
	)

	FAILED({})
	MATCHED({})
}

/*
	for_if_postfix
		: for_postfix
		| 'if' nllo logic_or_expr
 */
RULE(for_if_postfix)
{
	struct token_t *tmp_token;

	SUB_RULE_SET(
		SUB_RULE(R(for_postfix)
		{
			_RETVAL.expr = RULE_RET_AT(0).u.expr;
		})

		SUB_RULE(T(T_IF) R(nllo) R(logic_or_expr)
		{
			tmp_token = TOKEN_AT(0);
			_RETVAL.expr = ilang_gen_if_expr_new_c(
				_ENV->unit, TOKEN_POS(tmp_token),
				RULE_RET_AT(1).u.expr, IVM_NULL
			);
		})
	)

	FAILED({})
	MATCHED({})
}

/*
	list_comp_postfix_sub
		: nllo for_if_postfix list_comp_postfix_sub
		| %empty
 */
RULE(list_comp_postfix_sub)
{
	ilang_gen_expr_t *tmp_expr;

	SUB_RULE_SET(
		SUB_RULE(R(nllo) R(for_if_postfix) R(list_comp_postfix_sub)
		{
			tmp_expr = RULE_RET_AT(1).u.expr;

			if (ilang_gen_expr_isExpr(tmp_expr, if_expr)) {
				ilang_gen_if_expr_setMainBody(tmp_expr, RULE_RET_AT(2).u.expr);
			} else {
				ilang_gen_for_expr_setBody(tmp_expr, RULE_RET_AT(2).u.expr);
			}

			_RETVAL.expr = tmp_expr;
		})

		SUB_RULE({ _RETVAL.expr = IVM_NULL; })
	)

	FAILED({})
	MATCHED({})
}

/*
	list_comp_postfix
		: nllo for_postfix list_comp_postfix_sub
 */
RULE(list_comp_postfix)
{
	ilang_gen_expr_t *tmp_expr;

	SUB_RULE_SET(
		SUB_RULE(R(nllo) R(for_postfix) R(list_comp_postfix_sub)
		{
			tmp_expr = RULE_RET_AT(1).u.expr;

			ilang_gen_for_expr_setBody(tmp_expr, RULE_RET_AT(2).u.expr);

			_RETVAL.expr = tmp_expr;
		})
	)

	FAILED({})
	MATCHED({})
}

/*
	list_comp
		: logic_or_expr list_comp_postfix
 */
RULE(list_comp)
{
	ilang_gen_expr_t *tmp_expr;
	ilang_gen_expr_t *last;
	ivm_int_t cblock;

	SUB_RULE_SET(
		SUB_RULE(R(logic_or_expr) R(list_comp_postfix)
		{
			tmp_expr = RULE_RET_AT(0).u.expr;
			last = RULE_RET_AT(1).u.expr;
			cblock = 0;

			_RETVAL.expr = ilang_gen_list_comp_expr_new(_ENV->unit, ilang_gen_expr_getPos(tmp_expr), last);

			while (1) {
				if (ilang_gen_expr_isExpr(last, if_expr)) {
					if (!ilang_gen_if_expr_getMainBody(last)) break;
					last = ilang_gen_if_expr_getMainBody(last);
				} else {
					cblock++; // only for expr generates block
					if (!ilang_gen_for_expr_getBody(last)) break;
					last = ilang_gen_for_expr_getBody(last);
				}
			}

			tmp_expr = ilang_gen_list_comp_core_expr_new(_ENV->unit, ilang_gen_expr_getPos(tmp_expr), tmp_expr, cblock);

			if (ilang_gen_expr_isExpr(last, if_expr)) {
				ilang_gen_if_expr_setMainBody(last, tmp_expr);
			} else {
				ilang_gen_for_expr_setBody(last, tmp_expr);
			}
		})
	)

	FAILED({})
	MATCHED({})
}

/*
	primary_expr
		: string
		| int
		| float
		| id
		| 'import' nllo mod_name
		| '(' nllo expr nllo ')'
		| '{' expr_list '}'
		| '{' nllo slot_list_opt nllo '}'
		| '[' nllo arg_list_opt nllo ']'
 */
RULE(expr);
RULE(expr_list);
RULE(expr_list_opt);
RULE(arg_list_opt);
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

		SUB_RULE(T(T_MISSING) DBB(PRINT_MATCH_TOKEN("partial applied expr"))
		{
			tmp_token = TOKEN_AT(0);
			_RETVAL.expr = ilang_gen_pa_expr_new(_ENV->unit, TOKEN_POS(tmp_token), 0);
			_RETVAL.expr->is_missing = IVM_TRUE;
		})

		SUB_RULE(T(T_IMPORT) R(nllo) R(mod_name) DBB(PRINT_MATCH_TOKEN("import expr"))
		{
			tmp_token = TOKEN_AT(0);
			_RETVAL.expr = ilang_gen_import_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token),
				RULE_RET_AT(1).u.token_list
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

		SUB_RULE(T(T_LBRAKT) R(nllo)
				 R(arg_list_opt)
				 R(nllo) T(T_RBRAKT)
		{
			tmp_token = TOKEN_AT(0);
			_RETVAL.expr = ilang_gen_list_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token),
				RULE_RET_AT(1).u.expr_list
			);
		})

		SUB_RULE(T(T_LBRAKT) R(nllo)
				 R(list_comp)
				 R(nllo) T(T_RBRAKT)
		{
			_RETVAL.expr = RULE_RET_AT(1).u.expr;
		})
	);

	FAILED({})
	MATCHED({})
}

#define NONE_EXPR() \
	ilang_gen_none_expr_new(_ENV->unit, ilang_gen_pos_build(-1, -1), 0)

/*
	arg
		: 'expand' prefix_expr
		| prefix_expr
 */
RULE(arg)
{
	struct token_t *tmp_token;

	SUB_RULE_SET(
		SUB_RULE(T(T_EXPAND) R(prefix_expr)
		{
			tmp_token = TOKEN_AT(0);
			_RETVAL.expr = ilang_gen_expand_expr_new(_ENV->unit, TOKEN_POS(tmp_token), RULE_RET_AT(0).u.expr);
		})

		SUB_RULE(R(prefix_expr)
		{
			_RETVAL.expr = RULE_RET_AT(0).u.expr;
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	arg_list_single
		: nllo ',' nllo arg
		| nllo ',' nllo
 */
RULE(arg_list_single)
{
	SUB_RULE_SET(
		SUB_RULE(R(nllo) T(T_COMMA) R(nllo) R(arg)
		{
			_RETVAL.expr = RULE_RET_AT(2).u.expr;
		})

		SUB_RULE(R(nllo) T(T_COMMA) R(nllo)
		{
			_RETVAL.expr = NONE_EXPR();
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	arg_list_sub
		: list(arg_list_single)
		| %empty
 */
RULE(arg_list_sub)
{
	ilang_gen_expr_list_t *tmp_list = IVM_NULL;

	SUB_RULE_SET(
		SUB_RULE(R_LIST(arg_list_single, {
			if (!tmp_list) {
				tmp_list = ilang_gen_expr_list_new(_ENV->unit);
			}

			ilang_gen_expr_list_push(tmp_list, RULE_RET_AT(0).u.expr);
		}) {
			ilang_gen_expr_list_reverse(tmp_list);
			_RETVAL.expr_list = tmp_list;
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
		: arg arg_list_sub
		| arg_list_sub
		| %empty
 */
RULE(arg_list_opt)
{
	ilang_gen_expr_list_t *tmp_list;
	struct rule_val_t tmp_ret;

	SUB_RULE_SET(
		SUB_RULE(R(arg) R(arg_list_sub)
		{
			tmp_ret = RULE_RET_AT(0);
			tmp_list = _RETVAL.expr_list = RULE_RET_AT(1).u.expr_list;
			ilang_gen_expr_list_push(tmp_list, tmp_ret.u.expr);
		})

		SUB_RULE(R(arg_list_sub)
		{
			tmp_list = _RETVAL.expr_list = RULE_RET_AT(0).u.expr_list;
			if (ilang_gen_expr_list_size(tmp_list)) {
				// not an empty list
				ilang_gen_expr_list_push(tmp_list, NONE_EXPR());
			}
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
	oop
		: '!'
		| '+' '@'
		| '-' '@'
		| '+'
		| '-'
		| '*'
		| '\'
		| '%'
		| '&'
		| '|'
		| '^'
		| '[' nllo ']'
		| '[' nllo '=' nllo ']'
		| '(' nllo ')'
		| '!='
		| '=='
		| '>'
		| '>='
		| '<'
		| '<='
		| '<<'
		| '>>'
		| '>>>'
 */
RULE(oop)
{

#define DEF_OOP(op) \
	SUB_RULE(T(T_##op)                 \
	{                                  \
		_RETVAL.oop = IVM_OOP_ID(op);  \
	})

#define DEF_OOP_B(op) \
	SUB_RULE(T(T_B##op)                 \
	{                                  \
		_RETVAL.oop = IVM_OOP_ID(op);  \
	})

#define DEF_OOP_C(op) \
	SUB_RULE(T(T_C##op)                 \
	{                                  \
		_RETVAL.oop = IVM_OOP_ID(op);  \
	})

	SUB_RULE_SET(
		DEF_OOP(NOT)

		SUB_RULE(T(T_ADD) T(T_AT)
		{
			_RETVAL.oop = IVM_OOP_ID(POS);
		})

		SUB_RULE(T(T_SUB) T(T_AT)
		{
			_RETVAL.oop = IVM_OOP_ID(NEG);
		})

		DEF_OOP(ADD)
		DEF_OOP(SUB)
		DEF_OOP(MUL)
		DEF_OOP(DIV)
		DEF_OOP(MOD)

		DEF_OOP_B(AND)
		DEF_OOP_B(IOR)
		DEF_OOP_B(EOR)

		SUB_RULE(T(T_LBRAKT) R(nllo) T(T_RBRAKT)
		{
			_RETVAL.oop = IVM_OOP_ID(IDX);
		})

		SUB_RULE(T(T_LBRAKT) R(nllo) T(T_ASSIGN) R(nllo) T(T_RBRAKT)
		{
			_RETVAL.oop = IVM_OOP_ID(IDXA);
		})

		SUB_RULE(T(T_LPAREN) R(nllo) T(T_RPAREN)
		{
			_RETVAL.oop = IVM_OOP_ID(CALL);
		})

		DEF_OOP_C(NE)
		DEF_OOP_C(EQ)
		DEF_OOP_C(LT)
		DEF_OOP_C(LE)
		DEF_OOP_C(GT)
		DEF_OOP_C(GE)

		DEF_OOP(SHL)
		DEF_OOP(SHAR)
		DEF_OOP(SHLR)
	)

#undef DEF_OOP
#undef DEF_OOP_B
#undef DEF_OOP_C

	FAILED({})
	MATCHED({})
}

/*
	block_param
		: '|' nllo param_list_opt nllo '|'
 */
RULE(param_list_opt);
RULE(block_param)
{
	SUB_RULE_SET(
		SUB_RULE(T(T_BIOR) R(nllo) R(param_list_opt) R(nllo) T(T_BIOR)
		{
			_RETVAL.param_list = RULE_RET_AT(1).u.param_list;
		})
	)

	FAILED({})
	MATCHED({})
}

/*
	block_param_opt
		: block_param
		| %empty
 */
RULE(block_param_opt)
{
	SUB_RULE_SET(
		SUB_RULE(R(block_param)
		{
			_RETVAL.param_list = RULE_RET_AT(0).u.param_list;
		})

		SUB_RULE({
			_RETVAL.param_list = ilang_gen_param_list_new(_ENV->unit);
		})
	)

	FAILED({})
	MATCHED({})
}

/*
	block
		: '{' nllo block_param_opt nllo expr_list nllo '}'
		| '{' nllo slot_list_opt nllo '}'
		| 'to' nllo block_param_opt nllo prefix_expr
 */
RULE(block)
{
	struct token_t *tmp_token;

	SUB_RULE_SET(
		SUB_RULE(T(T_LBRAC) R(nllo)
				 R(block_param_opt) R(nllo)
				 R(expr_list_opt) R(nllo)
				 T(T_RBRAC)
		{
			tmp_token = TOKEN_AT(0);

			_RETVAL.expr = ilang_gen_fn_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token),
				RULE_RET_AT(1).u.param_list,
				ilang_gen_expr_block_new(
					_ENV->unit,
					TOKEN_POS(tmp_token),
					RULE_RET_AT(3).u.expr_list
				)
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

		SUB_RULE(T(T_TO) R(nllo)
				 R(block_param_opt) R(nllo)
				 R(prefix_expr)
		{
			tmp_token = TOKEN_AT(0);

			_RETVAL.expr = ilang_gen_fn_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token),
				RULE_RET_AT(1).u.param_list,
				RULE_RET_AT(3).u.expr
			);
		})
	)

	FAILED({})
	MATCHED({})
}

/*
	block_opt
		: block
		| %empty
 */
RULE(block_opt)
{
	SUB_RULE_SET(
		SUB_RULE(R(block)
		{
			_RETVAL.expr = RULE_RET_AT(0).u.expr;
		})

		SUB_RULE({
			_RETVAL.expr = IVM_NULL;
		})
	)

	FAILED({})
	MATCHED({})
}

/*
	postfix_expr_sub
		: '(' nllo arg_list_opt nllo ')' block_opt postfix_expr_sub
		| block postfix_expr_sub
		| '[' nllo arg_list_opt nllo ']' postfix_expr_sub
		| '.' id postfix_expr_sub
		| '.' oop postfix_expr_sub
		| id postfix_expr_sub
		| %empt
 */
RULE(postfix_expr_sub)
{
	struct token_t *tmp_token, *id;
	ilang_gen_expr_t *tmp_expr = IVM_NULL, *tmp_block;
	ilang_gen_expr_list_t *tmp_expr_list = IVM_NULL;
	ivm_int_t oop;

	SUB_RULE_SET(
		SUB_RULE(T(T_LPAREN) R(nllo)
				 R(arg_list_opt)
				 R(nllo) T(T_RPAREN)
				 R(block_opt) R(postfix_expr_sub) DBB(PRINT_MATCH_TOKEN("call expr"))
		{
			tmp_expr_list = RULE_RET_AT(1).u.expr_list;
			tmp_token = TOKEN_AT(0);
			tmp_expr = RULE_RET_AT(4).u.expr;
			tmp_block = RULE_RET_AT(3).u.expr;

			if (tmp_block) {
				ilang_gen_expr_list_pushFront(tmp_expr_list, tmp_block);
			}

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

		SUB_RULE(R(block) R(postfix_expr_sub) DBB(PRINT_MATCH_TOKEN("block call expr"))
		{
			tmp_expr_list = ilang_gen_expr_list_new(_ENV->unit);
			tmp_block = RULE_RET_AT(0).u.expr;
			tmp_expr = RULE_RET_AT(1).u.expr;

			ilang_gen_expr_list_push(tmp_expr_list, tmp_block);

			if (tmp_expr) {
				_RETVAL.expr = tmp_expr;
				// find the innermost expression
				while (GET_OPERAND(tmp_expr, 1))
					tmp_expr = GET_OPERAND(tmp_expr, 1);

				SET_OPERAND(tmp_expr, 1, ilang_gen_call_expr_new(
					_ENV->unit,
					tmp_block->pos, IVM_NULL, tmp_expr_list
				));
			} else {
				_RETVAL.expr = ilang_gen_call_expr_new(
					_ENV->unit,
					tmp_block->pos, IVM_NULL, tmp_expr_list
				);
			}
		})

		SUB_RULE(T(T_LBRAKT) R(nllo) R(arg_list_opt) R(nllo) T(T_RBRAKT) R(postfix_expr_sub)
		DBB(PRINT_MATCH_TOKEN("index expr"))
		{
			tmp_token = TOKEN_AT(0);
			tmp_expr_list = RULE_RET_AT(1).u.expr_list;
			tmp_expr = RULE_RET_AT(3).u.expr;

			if (tmp_expr) {
				_RETVAL.expr = tmp_expr;
				// find the innermost expression
				while (GET_OPERAND(tmp_expr, 1))
					tmp_expr = GET_OPERAND(tmp_expr, 1);

				SET_OPERAND(tmp_expr, 1, ilang_gen_index_expr_new(
					_ENV->unit,
					TOKEN_POS(tmp_token), IVM_NULL, tmp_expr_list
				));
			} else {
				_RETVAL.expr = ilang_gen_index_expr_new(
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

		SUB_RULE(T(T_DOT) R(nllo) R(oop) R(postfix_expr_sub) DBB(PRINT_MATCH_TOKEN("oop expr"))
		{
			tmp_token = TOKEN_AT(0);
			oop = RULE_RET_AT(1).u.oop;
			tmp_expr = RULE_RET_AT(2).u.expr;

			if (tmp_expr) {
				_RETVAL.expr = tmp_expr;
				// find the innermost expression
				while (GET_OPERAND(tmp_expr, 1))
					tmp_expr = GET_OPERAND(tmp_expr, 1);

				SET_OPERAND(tmp_expr, 1, ilang_gen_oop_expr_new(
					_ENV->unit,
					TOKEN_POS(tmp_token), IVM_NULL, oop
				));
			} else {
				_RETVAL.expr = ilang_gen_oop_expr_new(
					_ENV->unit,
					TOKEN_POS(tmp_token), IVM_NULL, oop
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

/*
		SUB_RULE(T(T_ELLIP) R(postfix_expr_sub) DBB(PRINT_MATCH_TOKEN("varg expr"))
		{
			tmp_token = TOKEN_AT(0);
			tmp_expr = RULE_RET_AT(0).u.expr;

			if (tmp_expr) {
				_RETVAL.expr = tmp_expr;
				// find the innermost expression
				while (GET_OPERAND(tmp_expr, 1))
					tmp_expr = GET_OPERAND(tmp_expr, 1);

				SET_OPERAND(tmp_expr, 1, ilang_gen_varg_expr_new(_ENV->unit, TOKEN_POS(tmp_token), IVM_NULL));
			} else {
				_RETVAL.expr = ilang_gen_varg_expr_new(_ENV->unit, TOKEN_POS(tmp_token), IVM_NULL);
			}
		})
*/
		
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
		| '-' unary_expr
		| '+' unary_expr
		| 'del' unary_expr
		| 'ref' unary_expr
		| 'deref' unary_expr
 */
RULE(unary_expr)
{
	struct token_t *tmp_token;

#define UNARY_EXPR(tok, name, op)	\
	SUB_RULE(T(tok) R(nllo) R(unary_expr)             \
	DBB(PRINT_MATCH_TOKEN(name))                      \
	{                                                 \
		tmp_token = TOKEN_AT(0);                      \
		_RETVAL.expr = ilang_gen_unary_expr_new(      \
			_ENV->unit,                               \
			TOKEN_POS(tmp_token),                     \
			RULE_RET_AT(1).u.expr,                    \
			op                                        \
		);                                            \
	})

	SUB_RULE_SET(
		UNARY_EXPR(T_NOT, "not expr", IVM_UNIOP_ID(NOT))
		UNARY_EXPR(T_SUB, "neg expr", IVM_UNIOP_ID(NEG))
		UNARY_EXPR(T_ADD, "pos expr", IVM_UNIOP_ID(POS))
		// UNARY_EXPR(T_CLONE, "clone expr", IVM_UNIOP_ID(CLONE))
		UNARY_EXPR(T_DEL, "del expr", IVM_UNIOP_ID(DEL))

		UNARY_EXPR(T_REF, "ref expr", ILANG_GEN_UNIOP_REF)
		UNARY_EXPR(T_DEREF, "deref expr", ILANG_GEN_UNIOP_DEREF)

		SUB_RULE(T(T_MUL) R(nllo) R(unary_expr) DBB("varg expr")
		{
			tmp_token = TOKEN_AT(0);
			_RETVAL.expr = ilang_gen_varg_expr_new(_ENV->unit, TOKEN_POS(tmp_token), RULE_RET_AT(1).u.expr);
		})

		SUB_RULE(R(postfix_expr)
		{
			_RETVAL.expr = RULE_RET_AT(0).u.expr;
		})
	);

#undef UNARY_EXPR

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

#define SUB1_OP(op) \
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

#define SUB1_CMP(cmp_type) \
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

#define SUB1_LOGIC(type) \
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
			SET_OPERAND(tmp_expr, 1, ilang_gen_logic_expr_new(          \
				_ENV->unit,                                             \
				TOKEN_POS(tmp_token), IVM_NULL,                         \
				RULE_RET_AT(1).u.expr, ILANG_GEN_##type                 \
			));                                                         \
		} else {                                                        \
			_RETVAL.expr = ilang_gen_logic_expr_new(                    \
				_ENV->unit,                                             \
				TOKEN_POS(tmp_token), IVM_NULL,                         \
				RULE_RET_AT(1).u.expr, ILANG_GEN_##type                 \
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
		SUB_RULE(T(T_MUL) R(nllo) R(unary_expr) R(mul_div_expr_sub) SUB1_OP(MUL))
		SUB_RULE(T(T_DIV) R(nllo) R(unary_expr) R(mul_div_expr_sub) SUB1_OP(DIV))
		SUB_RULE(T(T_MOD) R(nllo) R(unary_expr) R(mul_div_expr_sub) SUB1_OP(MOD))
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
		SUB_RULE(T(T_ADD) R(nllo) R(mul_div_expr) R(add_sub_expr_sub) SUB1_OP(ADD))
		SUB_RULE(T(T_SUB) R(nllo) R(mul_div_expr) R(add_sub_expr_sub) SUB1_OP(SUB))
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

/*
	shift_expr_sub
		: '<<' add_sub_expr shift_expr_sub
		| '>>' add_sub_expr shift_expr_sub
		| '>>>' add_sub_expr shift_expr_sub
		| %empty
 */
RULE(shift_expr_sub)
{
	SUB_RULE_SET(
		SUB_RULE(T(T_SHL) R(nllo) R(add_sub_expr) R(shift_expr_sub) SUB1_OP(SHL))
		SUB_RULE(T(T_SHAR) R(nllo) R(add_sub_expr) R(shift_expr_sub) SUB1_OP(SHAR))
		SUB_RULE(T(T_SHLR) R(nllo) R(add_sub_expr) R(shift_expr_sub) SUB1_OP(SHLR))
		SUB_RULE({ _RETVAL.expr = IVM_NULL; })
	);

	FAILED({})
	MATCHED({})
}

/*
	shift_expr
		: add_sub_expr shift_expr_sub
 */
RULE(shift_expr)
{
	SUB_RULE_SET(
		SUB_RULE(R(add_sub_expr) R(shift_expr_sub)
		DBB(
			if (RULE_RET_AT(1).u.expr)
				PRINT_MATCH_TOKEN("shift expr");
		) SUB2())
	);

	FAILED({})
	MATCHED({})
}

/*
	cmp_expr_sub
		: '>' shift_expr cmp_expr_sub
		| '>=' shift_expr cmp_expr_sub
		| '<' shift_expr cmp_expr_sub
		| '<=' shift_expr cmp_expr_sub
		| %empty
 */
RULE(cmp_expr_sub)
{
	SUB_RULE_SET(
		SUB_RULE(T(T_CGT) R(nllo) R(shift_expr) R(cmp_expr_sub) SUB1_CMP(GT))
		SUB_RULE(T(T_CGE) R(nllo) R(shift_expr) R(cmp_expr_sub) SUB1_CMP(GE))
		SUB_RULE(T(T_CLT) R(nllo) R(shift_expr) R(cmp_expr_sub) SUB1_CMP(LT))
		SUB_RULE(T(T_CLE) R(nllo) R(shift_expr) R(cmp_expr_sub) SUB1_CMP(LE))
		SUB_RULE({ _RETVAL.expr = IVM_NULL; })
	);

	FAILED({})
	MATCHED({})
}

/*
	cmp_expr
		: shift_expr cmp_expr_sub
 */
RULE(cmp_expr)
{
	SUB_RULE_SET(
		SUB_RULE(R(shift_expr) R(cmp_expr_sub)
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
		| 'is' cmp_expr eq_expr_sub
		| %empty
 */
RULE(eq_expr_sub)
{
	SUB_RULE_SET(
		SUB_RULE(T(T_CEQ) R(nllo) R(cmp_expr) R(eq_expr_sub) SUB1_CMP(EQ))
		SUB_RULE(T(T_CNE) R(nllo) R(cmp_expr) R(eq_expr_sub) SUB1_CMP(NE))
		SUB_RULE(T(T_IS) R(nllo) R(cmp_expr) R(eq_expr_sub) SUB1_CMP(IS))
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

/*
	and_expr_sub
		: '&' eq_expr and_expr_sub
		| %empty
 */
RULE(and_expr_sub)
{
	SUB_RULE_SET(
		SUB_RULE(T(T_BAND) R(nllo) R(eq_expr) R(and_expr_sub) SUB1_OP(AND))
		SUB_RULE({ _RETVAL.expr = IVM_NULL; })
	);

	FAILED({})
	MATCHED({})
}

/*
	and_expr
		: eq_expr and_expr_sub
 */
RULE(and_expr)
{
	SUB_RULE_SET(
		SUB_RULE(R(eq_expr) R(and_expr_sub)
		DBB(
			if (RULE_RET_AT(1).u.expr)
				PRINT_MATCH_TOKEN("bit and expr");
		) SUB2())
	);

	FAILED({})
	MATCHED({})
}

/*
	eor_expr_sub
		: '^' and_expr eor_expr_sub
		| %empty
 */
RULE(eor_expr_sub)
{
	SUB_RULE_SET(
		SUB_RULE(T(T_BEOR) R(nllo) R(and_expr) R(eor_expr_sub) SUB1_OP(EOR))
		SUB_RULE({ _RETVAL.expr = IVM_NULL; })
	);

	FAILED({})
	MATCHED({})
}

/*
	eor_expr
		: and_expr eor_expr_sub
 */
RULE(eor_expr)
{
	SUB_RULE_SET(
		SUB_RULE(R(and_expr) R(eor_expr_sub)
		DBB(
			if (RULE_RET_AT(1).u.expr)
				PRINT_MATCH_TOKEN("eor expr");
		) SUB2())
	);

	FAILED({})
	MATCHED({})
}

/*
	ior_expr_sub
		: '|' eor_expr ior_expr_sub
		| %empty
 */
RULE(ior_expr_sub)
{
	SUB_RULE_SET(
		SUB_RULE(T(T_BIOR) R(nllo) R(eor_expr) R(ior_expr_sub) SUB1_OP(IOR))
		SUB_RULE({ _RETVAL.expr = IVM_NULL; })
	);

	FAILED({})
	MATCHED({})
}

/*
	ior_expr
		: eor_expr ior_expr_sub
 */
RULE(ior_expr)
{
	SUB_RULE_SET(
		SUB_RULE(R(eor_expr) R(ior_expr_sub)
		DBB(
			if (RULE_RET_AT(1).u.expr)
				PRINT_MATCH_TOKEN("ior expr");
		) SUB2())
	);

	FAILED({})
	MATCHED({})
}

/*
	logic_and_expr_sub
		: '&&' ior_expr logic_and_expr_sub
		| %empty
 */
RULE(logic_and_expr_sub)
{
	SUB_RULE_SET(
		SUB_RULE(T(T_CAND) R(nllo) R(ior_expr) R(logic_and_expr_sub) SUB1_LOGIC(LOGIC_AND))
		SUB_RULE({ _RETVAL.expr = IVM_NULL; })
	);

	FAILED({})
	MATCHED({})
}

/*
	logic_and_expr
		: ior_expr logic_and_expr_sub
 */
RULE(logic_and_expr)
{
	SUB_RULE_SET(
		SUB_RULE(R(ior_expr) R(logic_and_expr_sub)
		DBB(
			if (RULE_RET_AT(1).u.expr)
				PRINT_MATCH_TOKEN("logic and expr");
		) SUB2())
	);

	FAILED({})
	MATCHED({})
}

/*
	logic_or_expr_sub
		: '||' logic_and_expr logic_or_expr_sub
		| %empty
 */
RULE(logic_or_expr_sub)
{
	SUB_RULE_SET(
		SUB_RULE(T(T_COR) R(nllo) R(logic_and_expr) R(logic_or_expr_sub) SUB1_LOGIC(LOGIC_OR))
		SUB_RULE({ _RETVAL.expr = IVM_NULL; })
	);

	FAILED({})
	MATCHED({})
}

/*
	logic_or_expr
		: logic_and_expr logic_or_expr_sub
 */
RULE(logic_or_expr)
{
	SUB_RULE_SET(
		SUB_RULE(R(logic_and_expr) R(logic_or_expr_sub)
		DBB(
			if (RULE_RET_AT(1).u.expr)
				PRINT_MATCH_TOKEN("logic or expr");
		) SUB2())
	);

	FAILED({})
	MATCHED({})
}

#undef SUB1_OP
#undef SUB1_CMP
#undef SUB1_LOGIC
#undef SUB2

RULE(prefix_expr);

/*
	param
		: id nllo '...'
		| '...'
		| id '=' nllo logic_or_expr
		| id
 */
RULE(param)
{
	struct token_t *tmp_token;

	SUB_RULE_SET(
		SUB_RULE(T(T_MUL) T(T_ID)
		{
			tmp_token = TOKEN_AT(1);
			_RETVAL.param = ilang_gen_param_build(IVM_TRUE, TOKEN_VAL(tmp_token), IVM_NULL);
		})

		SUB_RULE(T(T_MUL)
		{
			_RETVAL.param = ilang_gen_param_build(IVM_TRUE, TOKEN_VAL_EMPTY(), IVM_NULL);
		})

		SUB_RULE(T(T_ID) T(T_ASSIGN) R(nllo) R(logic_or_expr)
		{
			tmp_token = TOKEN_AT(0);
			_RETVAL.param = ilang_gen_param_build(IVM_FALSE, TOKEN_VAL(tmp_token), RULE_RET_AT(1).u.expr);
		})

		SUB_RULE(T(T_ID)
		{
			tmp_token = TOKEN_AT(0);
			_RETVAL.param = ilang_gen_param_build(IVM_FALSE, TOKEN_VAL(tmp_token), IVM_NULL);
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	param_list_sub
		: ',' nllo param param_list_sub
		| %empty
 */
RULE(param_list_sub)
{
	ilang_gen_param_list_t *tmp_list;
	ilang_gen_param_t tmp_value;

	SUB_RULE_SET(
		SUB_RULE(T(T_COMMA) R(nllo) R(param) R(param_list_sub)
		{
			tmp_list = _RETVAL.param_list = RULE_RET_AT(2).u.param_list;
			tmp_value = RULE_RET_AT(1).u.param;
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
		: param param_list_sub
		| %empty
 */
RULE(param_list_opt)
{
	ilang_gen_param_list_t *tmp_list;
	ilang_gen_param_t tmp_value;

	SUB_RULE_SET(
		SUB_RULE(R(param) R(param_list_sub)
		{
			tmp_list = _RETVAL.param_list = RULE_RET_AT(1).u.param_list;
			tmp_value = RULE_RET_AT(0).u.param;
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
				TOKEN_POS(tmp_token), ILANG_GEN_INTR_RET,
				RULE_RET_AT(0).u.expr, IVM_NULL
			);
		})
		SUB_RULE(T(T_RET)
		{
			tmp_token = TOKEN_AT(0);
			_RETVAL.expr = ilang_gen_intr_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token), ILANG_GEN_INTR_RET,
				IVM_NULL, IVM_NULL
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
				TOKEN_POS(tmp_token), ILANG_GEN_INTR_CONT, 
				RULE_RET_AT(0).u.expr, IVM_NULL
			);
		})
		SUB_RULE(T(T_CONT)
		{
			tmp_token = TOKEN_AT(0);
			_RETVAL.expr = ilang_gen_intr_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token), ILANG_GEN_INTR_CONT,
				IVM_NULL, IVM_NULL
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
				TOKEN_POS(tmp_token), ILANG_GEN_INTR_BREAK,
				RULE_RET_AT(0).u.expr, IVM_NULL
			);
		})
		SUB_RULE(T(T_BREAK)
		{
			tmp_token = TOKEN_AT(0);
			_RETVAL.expr = ilang_gen_intr_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token), ILANG_GEN_INTR_BREAK,
				IVM_NULL, IVM_NULL
			);
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	raise_expr:
		: 'raise' prefix_expr
 */
RULE(raise_expr)
{
	struct token_t *tmp_token;

	SUB_RULE_SET(
		SUB_RULE(T(T_RAISE) R(prefix_expr)
		{
			tmp_token = TOKEN_AT(0);
			_RETVAL.expr = ilang_gen_intr_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token), ILANG_GEN_INTR_RAISE,
				RULE_RET_AT(0).u.expr, IVM_NULL
			);
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	yield_expr:
		: 'resume' prefix_expr 'with' prefix_expr
		| 'resume' prefix_expr
		| 'yield' prefix_expr
		| 'yield'
 */
RULE(yield_expr)
{
	struct token_t *tmp_token;

	SUB_RULE_SET(
		SUB_RULE(T(T_RESUME) R(prefix_expr) T(T_WITH) R(prefix_expr)
		{
			tmp_token = TOKEN_AT(0);
			_RETVAL.expr = ilang_gen_intr_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token), ILANG_GEN_INTR_RESUME,
				RULE_RET_AT(0).u.expr, RULE_RET_AT(1).u.expr
			);
		})

		SUB_RULE(T(T_RESUME) R(prefix_expr)
		{
			tmp_token = TOKEN_AT(0);
			_RETVAL.expr = ilang_gen_intr_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token), ILANG_GEN_INTR_RESUME,
				RULE_RET_AT(0).u.expr, IVM_NULL
			);
		})
		
		SUB_RULE(T(T_YIELD) R(prefix_expr)
		{
			tmp_token = TOKEN_AT(0);
			_RETVAL.expr = ilang_gen_intr_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token), ILANG_GEN_INTR_YIELD,
				RULE_RET_AT(0).u.expr, IVM_NULL
			);
		})

		SUB_RULE(T(T_YIELD)
		{
			tmp_token = TOKEN_AT(0);
			_RETVAL.expr = ilang_gen_intr_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token), ILANG_GEN_INTR_YIELD,
				IVM_NULL, IVM_NULL
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
		| raise_expr
 */
RULE(intr_expr)
{
	SUB_RULE_SET(
		SUB_RULE(R(ret_expr))
		SUB_RULE(R(cont_expr))
		SUB_RULE(R(break_expr))
		SUB_RULE(R(raise_expr))
		SUB_RULE(R(yield_expr))
	);

	FAILED({})
	MATCHED({
		_RETVAL.expr = RULE_RET_AT(0).u.expr;
	})
}

/*
	leftval
		: logic_or_expr
 */
RULE(leftval)
{

	SUB_RULE_SET(
		SUB_RULE(R(logic_or_expr)
		{
			_RETVAL.expr = RULE_RET_AT(0).u.expr;
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	assign_expr
		| leftval '=' nllo prefix_expr
 */
RULE(assign_expr)
{
	struct token_t *tmp_token;

	SUB_RULE_SET(
		SUB_RULE(R(leftval) T(T_ASSIGN) R(nllo) R(prefix_expr)
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
				 R(else_branch_opt) DBB(PRINT_MATCH_TOKEN("if expr"))
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
				 R(prefix_expr) DBB(PRINT_MATCH_TOKEN("while expr"))
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
	for_expr
		: 'for' nllo leftval nllo 'in' nllo expr nllo ':' preifx_expr
 */
RULE(leftval);
RULE(for_expr)
{
	struct token_t *tmp_token;

	SUB_RULE_SET(
		SUB_RULE(T(T_FOR) R(nllo)
				 R(leftval) R(nllo)
				 T(T_IN) R(nllo)
				 R(expr) R(nllo)
				 T(T_COLON) R(nllo) 
				 R(prefix_expr) DBB(PRINT_MATCH_TOKEN("for expr"))
		{
			tmp_token = TOKEN_AT(0);
			_RETVAL.expr = ilang_gen_for_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token),
				RULE_RET_AT(1).u.expr,
				RULE_RET_AT(4).u.expr,
				RULE_RET_AT(7).u.expr
			);
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	final_branch_opt
		: 'final' nllo ':' nllo prefix_expr
		| %empty
 */
RULE(final_branch_opt)
{
	SUB_RULE_SET(
		SUB_RULE(R(nllo) T(T_FINAL) R(nllo)
				 T(T_COLON) R(nllo) R(prefix_expr)
				 DBB(PRINT_MATCH_TOKEN("final branch"))
		{
			_RETVAL.expr = RULE_RET_AT(3).u.expr;
		})
		SUB_RULE()
	);

	FAILED({})
	MATCHED({})
}

/*
	catch_branch_opt
		: 'catch' nllo id nllo ':' nllo prefix_expr
		| 'catch' nllo ':' nllo prefix_expr
		| %empty
 */
RULE(catch_branch_opt)
{
	struct token_t *tmp_token;

	SUB_RULE_SET(
		SUB_RULE(R(nllo) T(T_CATCH) R(nllo)
				 T(T_ID) R(nllo)
				 T(T_COLON) R(nllo) R(prefix_expr)
				 DBB(PRINT_MATCH_TOKEN("catch branch"))
		{
			tmp_token = TOKEN_AT(1);
			_RETVAL.catch_branch = ilang_gen_catch_branch_build(
				TOKEN_VAL(tmp_token), RULE_RET_AT(4).u.expr
			);
		})
		SUB_RULE(R(nllo) T(T_CATCH) R(nllo)
				 T(T_COLON) R(nllo) R(prefix_expr)
				 DBB(PRINT_MATCH_TOKEN("catch branch(no arg)"))
		{
			tmp_token = TOKEN_AT(1);
			_RETVAL.catch_branch = ilang_gen_catch_branch_build(
				TOKEN_VAL_EMPTY(), RULE_RET_AT(3).u.expr
			);
		})
		SUB_RULE()
	);

	FAILED({})
	MATCHED({})
}

/*
	try_expr
		: 'try' nllo ':' nllo prefix_expr catch_branch_opt final_branch_opt
 */
RULE(try_expr)
{
	struct token_t *tmp_token;

	SUB_RULE_SET(
		SUB_RULE(T(T_TRY) R(nllo) T(T_COLON) R(nllo)
				 R(prefix_expr) DBB(PRINT_MATCH_TOKEN("try branch"))
				 R(catch_branch_opt)
				 R(final_branch_opt)
		{
			tmp_token = TOKEN_AT(0);
			_RETVAL.expr = ilang_gen_try_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token),
				RULE_RET_AT(2).u.expr,
				RULE_RET_AT(3).u.catch_branch,
				RULE_RET_AT(4).u.expr
			);
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	fork_expr
		: 'fork' nllo ':' nllo prefix_expr
		| 'fork' nllo prefix_expr
 */
RULE(fork_expr)
{
	struct token_t *tmp_token;

	SUB_RULE_SET(
		SUB_RULE(T(T_FORK) R(nllo) T(T_COLON) R(nllo)
				 R(prefix_expr) DBB(PRINT_MATCH_TOKEN("fork fn expr"))
		{
			tmp_token = TOKEN_AT(0);
			/*
				fork: expr
				=>
				fork fn: expr
			 */
			_RETVAL.expr = ilang_gen_fork_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token),
				ilang_gen_fn_expr_new(
					_ENV->unit,
					TOKEN_POS(tmp_token),
					IVM_NULL, RULE_RET_AT(2).u.expr
				),
				IVM_FALSE
			);
		})
		SUB_RULE(T(T_FORK) R(nllo)
				 R(prefix_expr) DBB(PRINT_MATCH_TOKEN("fork expr"))
		{
			tmp_token = TOKEN_AT(0);
			/*
				fork expr
				=>
				fork fn: expr()
			 */
			_RETVAL.expr = ilang_gen_fork_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token),
				RULE_RET_AT(1).u.expr,
				IVM_FALSE
			);
		})
	);

	FAILED({})
	MATCHED({})
}

#if 0

/*
	group_expr
		: 'group' nllo ':' nllo prefix_expr
		| 'group' nllo prefix_expr
 */
RULE(group_expr)
{
	struct token_t *tmp_token;

	SUB_RULE_SET(
		SUB_RULE(T(T_GROUP) R(nllo) T(T_COLON) R(nllo)
				 R(prefix_expr) DBB(PRINT_MATCH_TOKEN("group fn expr"))
		{
			tmp_token = TOKEN_AT(0);
			/*
				fork: expr
				=>
				fork fn: expr
			 */
			_RETVAL.expr = ilang_gen_fork_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token),
				ilang_gen_fn_expr_new(
					_ENV->unit,
					TOKEN_POS(tmp_token),
					IVM_NULL, RULE_RET_AT(2).u.expr
				),
				IVM_TRUE
			);
		})
		SUB_RULE(T(T_GROUP) R(nllo)
				 R(prefix_expr) DBB(PRINT_MATCH_TOKEN("group expr"))
		{
			tmp_token = TOKEN_AT(0);
			/*
				fork expr
				=>
				fork fn: expr()
			 */
			_RETVAL.expr = ilang_gen_fork_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token),
				RULE_RET_AT(1).u.expr,
				IVM_TRUE
			);
		})
	);

	FAILED({})
	MATCHED({})
}

#endif

/*

RULE(branch_expr)
{
	struct token_t *tmp_token;

	SUB_RULE_SET(
		SUB_RULE(R(logic_or_expr)
				 T(T_QM) R(nllo)
				 R(prefix_expr) R(nllo)
				 T(T_COLON) R(nllo)
				 R(prefix_expr) DBB(PRINT_MATCH_TOKEN("branchexpr"))
		{
			tmp_token = TOKEN_AT(0);

			_RETVAL.expr = ilang_gen_if_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token),
				ilang_gen_branch_build(RULE_RET_AT(0).u.expr, RULE_RET_AT(2).u.expr),
				IVM_NULL,
				ilang_gen_branch_build(IVM_NULL, RULE_RET_AT(5).u.expr)
			);
		})
	);

	FAILED({})
	MATCHED({})
}

*/

/*
	prefix_expr
		: assign_expr
		| intr_expr
		| fn_expr
		| try_expr
		| if_expr
		| while_expr
		| for_expr
		| fork_expr
		| logic_or_expr '?' nllo prefix_expr nllo ':' nllo prefix_expr
 */

RULE(prefix_expr)
{
	struct token_t *tmp_token;
	ivm_bool_t is_branch = IVM_FALSE;

	SUB_RULE_SET(
		SUB_RULE(R(assign_expr) DBB(PRINT_MATCH_TOKEN("assign expr")))
		SUB_RULE(R(intr_expr) DBB(PRINT_MATCH_TOKEN("intr expr")))
		SUB_RULE(R(fn_expr))
		SUB_RULE(R(try_expr))
		SUB_RULE(R(if_expr))
		SUB_RULE(R(while_expr))
		SUB_RULE(R(for_expr))
		SUB_RULE(R(fork_expr))
		// SUB_RULE(R(group_expr))
		// SUB_RULE(R(branch_expr))
		// SUB_RULE(R(logic_or_expr))

		SUB_RULE(R(logic_or_expr)
				 T_OPT(T_QM, { GOTO_MATCHED(); }) R(nllo)
				 R(prefix_expr) R(nllo)
				 T(T_COLON) R(nllo)
				 R(prefix_expr) DBB(PRINT_MATCH_TOKEN("branch expr"))
		{
			tmp_token = TOKEN_AT(0);
			is_branch = IVM_TRUE;

			_RETVAL.expr = ilang_gen_if_expr_new(
				_ENV->unit,
				TOKEN_POS(tmp_token),
				ilang_gen_branch_build(RULE_RET_AT(0).u.expr, RULE_RET_AT(2).u.expr),
				IVM_NULL,
				ilang_gen_branch_build(IVM_NULL, RULE_RET_AT(5).u.expr)
			);
		})
	);

	FAILED({})
	MATCHED({
		if (!is_branch)
			_RETVAL.expr = RULE_RET_AT(0).u.expr;
	})
}

/*
	comma_expr_single
		: ',' nllo prefix_expr
 */
RULE(comma_expr_single)
{
	SUB_RULE_SET(
		SUB_RULE(T(T_COMMA) R(nllo) R(prefix_expr)
		{
			_RETVAL.expr = RULE_RET_AT(1).u.expr;
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	comma_expr_sub
		: list(comma_expr_single)
		| %empty
 */
RULE(comma_expr_sub)
{
	ilang_gen_expr_list_t *tmp_expr_list = IVM_NULL;

	SUB_RULE_SET(
		SUB_RULE(R_LIST(comma_expr_single, {
			if (!tmp_expr_list) {
				tmp_expr_list = ilang_gen_expr_list_new(_ENV->unit);
			}

			ilang_gen_expr_list_push(tmp_expr_list, RULE_RET_AT(0).u.expr);
		}) {
			ilang_gen_expr_list_reverse(tmp_expr_list);
			_RETVAL.expr_list = tmp_expr_list;
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
		: comma_expr
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
	expr_list_single
		: sep_list expr
		| sep_list
 */
RULE(expr_list_single)
{
	SUB_RULE_SET(
		SUB_RULE(R(sep_list) CLEAR_ERR() R(expr)
		{
			_RETVAL.expr = RULE_RET_AT(1).u.expr;
		})

		SUB_RULE(R(sep_list)
		{
			_RETVAL.expr = IVM_NULL;
		})
	);

	FAILED({})
	MATCHED({})
}

/*
	expr_list_sub
		: list(expr_list_single)
		| %empty
 */
RULE(expr_list_sub)
{
	ilang_gen_expr_list_t *tmp_expr_list = IVM_NULL;
	ilang_gen_expr_t *tmp_expr;

	SUB_RULE_SET(
		SUB_RULE(R_LIST(expr_list_single, {
			if (!tmp_expr_list) {
				tmp_expr_list = ilang_gen_expr_list_new(_ENV->unit);
			}

			tmp_expr = RULE_RET_AT(0).u.expr;
			if (tmp_expr) {
				ilang_gen_expr_list_push(tmp_expr_list, tmp_expr);
			}
		}) {
			ilang_gen_expr_list_reverse(tmp_expr_list);
			_RETVAL.expr_list = tmp_expr_list;
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

/*
ilang_gen_trans_unit_t *
_ivm_parser_parseToken(ivm_list_t *tokens,
					   ivm_bool_t *suc)
{
	struct rule_val_t rule_ret;
	ilang_gen_trans_unit_t *ret = ilang_gen_trans_unit_new();
	struct env_t env = { ret, IVM_TRUE };

	RULE_START(trans_unit, &env, &rule_ret, tokens, *suc);

	return ret;
}
*/

// null for parse error
ilang_gen_trans_unit_t *
ilang_parser_parseSource(const ivm_char_t *file,
						 const ivm_char_t *src,
						 ivm_bool_t debug)
{
	ivm_list_t *tokens;
	struct rule_val_t rule_ret;
	ilang_gen_trans_unit_t *ret;
	struct env_t env;
	ivm_bool_t suc;

	tokens = _ilang_parser_getTokens(src, debug);

	if (!tokens) return IVM_NULL;

	ret = ilang_gen_trans_unit_new(file);
	env = (struct env_t) { ret, debug };

	RULE_START(trans_unit, &env, &rule_ret, tokens, suc);

	if (!suc) {
		ilang_gen_trans_unit_free(ret);
		ret = IVM_NULL;
	}

	ivm_list_free(tokens);

	return ret;
}
