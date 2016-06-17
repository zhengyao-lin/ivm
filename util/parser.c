#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pub/mem.h"
#include "pub/com.h"
#include "pub/type.h"
#include "pub/err.h"
#include "pub/vm.h"

#include "std/list.h"

#include "parser.h"

#if 1

enum token_id_t {
	T_NONE = 0,
	T_IGN,
	T_ID,
	T_NUM,
	T_STR
};

struct token_t {
	enum token_id_t id;
	ivm_size_t len;
	const ivm_char_t *val;
};

enum state_t {
	ST_INIT = 0,
	ST_IN_ID,
	ST_IN_STR,
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

	IVM_ASSERT(len, "illegal reg");

	// IVM_TRACE("reg: %c to %s\n", c, reg);

	switch (reg[0]) {
		case '=': return c == reg[1];
		case '-': return (c >= reg[1] && c <= reg[2]) ||
						 (c <= reg[1] && c >= reg[2]);
		case '.': return IVM_TRUE;
		default: ;
	}

	IVM_ASSERT(0, "unexpected reg");

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

								{ "=\"", ST_IN_STR, .ign = IVM_TRUE },
								{ "= ", ST_INIT, .ign = IVM_TRUE },

								{ "=\0", ST_INIT },

								{ IVM_NULL, STATE_ERR, .msg = "unexpect char '%c'" }
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
								{ ".", ST_IN_STR }
							},

		/* IN_NUM_INT */	{
								{ "-09", ST_IN_NUM_INT },
								{ "=.", ST_IN_NUM_DEC },
								{ ".", ST_INIT, T_NUM }
							},

		/* IN_NUM_DEC */	{
								{ "-09", ST_IN_NUM_DEC },
								{ ".", ST_INIT, T_NUM }
							}
	};


	#define NEXT_INIT ((struct token_t) { .len = 0, .val = c + 1 })
	#define CUR_INIT ((struct token_t) { .len = 0, .val = c-- })

	const ivm_char_t *c = src;
	struct token_t tmp_token = (struct token_t) { .len = 0, .val = c };
	struct trans_entry_t *tmp_entry;

	do {
		// IVM_TRACE("matching %c state %d\n", *c, state);
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
						MEM_COPY(tmp_buf, tmp_token.val, tmp_token.len);
						tmp_buf[tmp_token.len] = '\0';
						IVM_TRACE("t: %d -> '%s'\n", tmp_entry->save, tmp_buf);

						tmp_token.id = tmp_entry->save;
						ivm_list_push(ret, &tmp_token);

						if (tmp_entry->exc) {
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

		IVM_ASSERT(tmp_entry->match,
				   tmp_entry->msg,
				   *c);

		state = tmp_entry->to_state;
	} while (*c++ != '\0');

	IVM_ASSERT(state == ST_INIT,
			   "unexpected ending");

	return ret;
}

#endif
