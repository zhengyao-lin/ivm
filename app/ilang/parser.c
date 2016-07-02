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

	T_SEMIC,	// ;
	T_COMMA,	// ,
	T_COLON,	// :

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

	"semicolon",
	"comma",
	"colon",

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
#define IVM_COMMON_MAX_TOKEN_RULE 30
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
			{ "==", ST_INIT, T_CGE },
			{ ".", ST_INIT, T_CGT, .exc = IVM_TRUE },
		},

		/* TRY_LT */
		{
			{ "==", ST_INIT, T_CLE },
			{ ".", ST_INIT, T_CLT, .exc = IVM_TRUE }
		},

		/* TRY_NOT */
		{
			{ "==", ST_INIT, T_CNE },
			{ ".", ST_INIT, T_NOT, .exc = IVM_TRUE }
		},

		/* TRY_ASSIGN */
		{
			{ "==", ST_INIT, T_CEQ },
			{ ".", ST_INIT, T_ASSIGN, .exc = IVM_TRUE }
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
			{ "-09", ST_IN_NUM_DEC }
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
#undef KEYWORD
	};

#define KEYWORD_COUNT (sizeof(keywords) / sizeof(*keywords))

	for (i = 0; i < size; i++) {
		tmp_token = (struct token_t *)ivm_list_at(ret, i);
		if (tmp_token->id == T_ID) {
			for (j = 0; j < KEYWORD_COUNT; j++) {
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
