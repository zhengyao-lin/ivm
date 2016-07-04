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

enum token_id_t {
	T_NONE = 0,
	T_UNKNOWN,
	T_ID,
	T_INT,
	T_FLOAT,
	T_STR,

	/* keywords */
	T_FN,
	T_LET,
	T_IF,
	T_ELSE,
	T_WHILE,
	T_RET,

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
	"keyword `let`",
	"keyword `if`",
	"keyword `else`",
	"keyword `while`",
	"keyword `ret`",

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
		KEYWORD("let", T_LET)
		KEYWORD("if", T_IF)
		KEYWORD("else", T_ELSE)
		KEYWORD("while", T_WHILE)
		KEYWORD("ret", T_RET)
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

struct rule_val_t { int dummy; };
struct env_t { int dummy; };

#define R EXPECT_RULE
#define T EXPECT_TOKEN

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
	newl_list_opt
		: newl newl_list_sub
		| %empty
 */
RULE(newl_list_opt)
{
	SUB_RULE_SET(
		SUB_RULE(T(T_NEWL) R(newl_list_sub))
		SUB_RULE()
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
		| '(' expression ')'
		| '{' expression_list_opt '}'
 */
RULE(expression);
RULE(expression_list);
RULE(expression_list_opt);
RULE(primary_expr)
{
	SUB_RULE_SET(
		SUB_RULE(T(T_STR))
		SUB_RULE(T(T_INT))
		SUB_RULE(T(T_FLOAT))
		SUB_RULE(T(T_ID))
		SUB_RULE(T(T_LPAREN) R(newl_list_opt) R(expression) R(newl_list_opt) T(T_RPAREN))
		SUB_RULE(T(T_LBRAC) R(expression_list_opt) T(T_RBRAC))
	);

	FAILED({})
	MATCHED({})
}

/*
	arg_list_sub
		: ',' expression arg_list_sub
		| %empty
 */
RULE(arg_list_sub)
{
	SUB_RULE_SET(
		SUB_RULE(T(T_COMMA) R(newl_list_opt) R(expression) R(arg_list_sub))
		SUB_RULE()
	);

	FAILED({})
	MATCHED({})
}

/*
	arg_list
		: expression arg_list_sub
 */
RULE(arg_list)
{
	SUB_RULE_SET(
		SUB_RULE(R(expression) R(arg_list_sub))
	);

	FAILED({})
	MATCHED({})
}

/*
	arg_list_opt
		: expression arg_list_sub
		| %empty
 */
RULE(arg_list_opt)
{
	SUB_RULE_SET(
		SUB_RULE(R(expression) R(arg_list_sub))
		SUB_RULE()
	);

	FAILED({})
	MATCHED({})
}

/*
	postfix_expr_sub
		: '(' arg_list ')' postfix_expr_sub
		| '.' id postfix_expr_sub
		| %empty
 */
RULE(postfix_expr_sub)
{
	SUB_RULE_SET(
		SUB_RULE(T(T_LPAREN) R(newl_list_opt) R(arg_list_opt) R(newl_list_opt) T(T_RPAREN) R(postfix_expr_sub))
		SUB_RULE(T(T_DOT) R(newl_list_opt) T(T_ID) R(postfix_expr_sub))
		SUB_RULE()
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
	SUB_RULE_SET(
		SUB_RULE(R(primary_expr) R(postfix_expr_sub))
	);

	FAILED({})
	MATCHED({})
}

/*
	unary_expr
		: postfix_expr
		| '!' unary_expr
 */
RULE(unary_expr)
{
	SUB_RULE_SET(
		SUB_RULE(T(T_NOT) R(newl_list_opt) R(unary_expr))
		SUB_RULE(R(postfix_expr))
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
RULE(mul_div_expr_sub)
{
	SUB_RULE_SET(
		SUB_RULE(T(T_MUL) R(newl_list_opt) R(unary_expr) R(mul_div_expr_sub))
		SUB_RULE(T(T_DIV) R(newl_list_opt) R(unary_expr) R(mul_div_expr_sub))
		SUB_RULE(T(T_MOD) R(newl_list_opt) R(unary_expr) R(mul_div_expr_sub))
		SUB_RULE()
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
		SUB_RULE(R(unary_expr) R(mul_div_expr_sub))
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
		SUB_RULE(T(T_ADD) R(newl_list_opt) R(mul_div_expr) R(add_sub_expr_sub))
		SUB_RULE(T(T_SUB) R(newl_list_opt) R(mul_div_expr) R(add_sub_expr_sub))
		SUB_RULE()
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
		SUB_RULE(R(mul_div_expr) R(add_sub_expr_sub))
	);

	FAILED({})
	MATCHED({})
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
		SUB_RULE(T(T_CGT) R(newl_list_opt) R(add_sub_expr) R(cmp_expr_sub))
		SUB_RULE(T(T_CGE) R(newl_list_opt) R(add_sub_expr) R(cmp_expr_sub))
		SUB_RULE(T(T_CLT) R(newl_list_opt) R(add_sub_expr) R(cmp_expr_sub))
		SUB_RULE(T(T_CLE) R(newl_list_opt) R(add_sub_expr) R(cmp_expr_sub))
		SUB_RULE()
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
		SUB_RULE(R(add_sub_expr) R(cmp_expr_sub))
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
		SUB_RULE(T(T_CEQ) R(newl_list_opt) R(cmp_expr) R(eq_expr_sub))
		SUB_RULE(T(T_CNE) R(newl_list_opt) R(cmp_expr) R(eq_expr_sub))
		SUB_RULE()
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
		SUB_RULE(R(cmp_expr) R(eq_expr_sub))
	);

	FAILED({})
	MATCHED({})
}

RULE(prefix_expr);

/*
	param_list_sub
		: ',' id param_list_sub
		| %empty
 */
RULE(param_list_sub)
{
	SUB_RULE_SET(
		SUB_RULE(T(T_COMMA) R(newl_list_opt) T(T_ID) R(param_list_sub))
		SUB_RULE()
	);

	FAILED({})
	MATCHED({})
}

/*
	param_list
		: id param_list_sub
 */
RULE(param_list)
{
	SUB_RULE_SET(
		SUB_RULE(T(T_ID) R(param_list_sub))
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
	SUB_RULE_SET(
		SUB_RULE(T(T_ID) R(param_list_sub))
		SUB_RULE()
	);

	FAILED({})
	MATCHED({})
}

/*
	fn_expr
		: fn param_list ':' prefix_expr
 */
RULE(fn_expr)
{
	SUB_RULE_SET(
		SUB_RULE(T(T_FN) R(param_list_opt) R(newl_list_opt) T(T_COLON) R(newl_list_opt) R(prefix_expr))
	);

	FAILED({})
	MATCHED({})
}

/*
	intr_expr:
		: 'ret' prefix_expr
		| 'ret'
 */
RULE(intr_expr)
{
	SUB_RULE_SET(
		SUB_RULE(T(T_RET) R(prefix_expr))
		SUB_RULE(T(T_RET))
	);

	FAILED({})
	MATCHED({})
}

/*
	assign_expr
		: eq_expr '=' prefix_expr
 */
RULE(assign_expr)
{
	SUB_RULE_SET(
		SUB_RULE(R(eq_expr) T(T_ASSIGN) R(newl_list_opt) R(prefix_expr))
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
		SUB_RULE(R(assign_expr) PRINT_MATCH_TOKEN("assign expr"))
		SUB_RULE(R(intr_expr) PRINT_MATCH_TOKEN("intr expr"))
		SUB_RULE(R(fn_expr) PRINT_MATCH_TOKEN("fn expr"))
		SUB_RULE(R(eq_expr) PRINT_MATCH_TOKEN("eq expr"))
	);

	FAILED({})
	MATCHED({})
}

/*
	comma_expr_sub
		: ',' prefix_expr comma_expr_sub
		| %empty
 */
RULE(comma_expr_sub)
{
	SUB_RULE_SET(
		SUB_RULE(T(T_COMMA) R(newl_list_opt) R(prefix_expr) R(comma_expr_sub))
		SUB_RULE()
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
	SUB_RULE_SET(
		SUB_RULE(R(prefix_expr) R(comma_expr_sub) PRINT_MATCH_TOKEN("comma expr"))
	);

	FAILED({})
	MATCHED({})
}

/*
	expression
		: intr_expr
 */
RULE(expression)
{
	SUB_RULE_SET(
		SUB_RULE(R(comma_expr))
	);

	FAILED({})
	MATCHED({})
}

/*
	expression_list_sub
		: sep_list expression expression_list_sub
		| %empty
 */
RULE(expression_list_sub)
{
	SUB_RULE_SET(
		SUB_RULE(R(sep_list) CLEAR_ERR() R(expression)
				 IVM_TRACE("********* expr matched *********\n");
				 R(expression_list_sub))
		SUB_RULE(R(sep_list))
		SUB_RULE()
	);

	FAILED({})
	MATCHED({})
}

/*
	expression_list
		: expression expression_list_sub
 */
RULE(expression_list)
{
	SUB_RULE_SET(
		SUB_RULE(R(sep_list_opt) CLEAR_ERR() R(expression)
				 IVM_TRACE("********* expr matched *********\n");
				 R(expression_list_sub))
	);

	FAILED({})
	MATCHED({})
}

/*
	expression_list_opt
		: expression expression_list_sub
		| %empty
 */
RULE(expression_list_opt)
{
	SUB_RULE_SET(
		SUB_RULE(R(sep_list_opt) CLEAR_ERR() R(expression)
				 IVM_TRACE("********* expr matched *********\n");
				 R(expression_list_sub))
		SUB_RULE()
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
		: expression_list
 */

RULE(trans_unit)
{
	SUB_RULE_SET(
		SUB_RULE(R(expression_list) R(eof_opt))
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

ivm_bool_t
_ivm_parser_tryParse(ivm_list_t *tokens)
{
	struct env_t env = { 0 };
	struct rule_val_t ret;
	ivm_bool_t suc;

	RULE_START(trans_unit, &env, &ret, tokens, suc);

	return suc;
}
