#ifndef _IVM_UTIL_PARSER_H_
#define _IVM_UTIL_PARSER_H_

#include "pub/com.h"
#include "pub/type.h"
#include "pub/vm.h"

#include "std/list.h"

IVM_COM_HEADER

ivm_double_t
ivm_parser_parseNum(const ivm_char_t *src,
					ivm_size_t len,
					ivm_bool_t *err);

ivm_char_t *
ivm_parser_parseStr(const ivm_char_t *str,
					ivm_size_t len);

/*
 * common parser: define IVM_USE_COMMON_PARSER to use(it MUST NOT be used in header file)
 * 
 * need to define BEFORE including:
 *    1. const ivm_char_t *token_name_table[] (name of each token)
 *    2. struct rule_val_t (return value of every rule)
 *    3. struct env_t (possessed by every rule handler)
 *
 * default settings:
 *    1. default max transition rule count: IVM_COMMON_MAX_TOKEN_RULE = 20
 *    2. default none token value: IVM_COMMON_TOKEN_NONE = 0(don't set any token to 0)
 *    3. default init state: IVM_COMMON_TOKEN_STATE_INIT = 0
 *    4. default unexpected state: IVM_COMMON_TOKEN_STATE_UNEXP = 1
 *    5. debug mode is closed in default: IVM_COMMON_DEBUG_MODE(not defined)
 *
 * transition rule(only apply to single character):
 *    1. "=a": the char is 'a'
 *    2. "-ab": the char could be between a and b(in ASCII code)
 *    3. ".": any char can be applied
 *    4. NULL: end of rule, unexpected char
 *
 * note:
 *    1. it's just a simple recursive descent parser -- DO NOT include any left-recursive rules
 *    2. it's quite inefficient
 */

#if defined(IVM_USE_COMMON_PARSER)
#undef IVM_USE_COMMON_PARSER

#ifndef IVM_COMMON_PARSER_NAME
	#error no parser name given
#endif

#ifndef IVM_COMMON_MAX_TOKEN_RULE
	#define IVM_COMMON_MAX_TOKEN_RULE 20
#endif

#ifndef IVM_COMMON_TOKEN_NONE
	#define IVM_COMMON_TOKEN_NONE 0
#endif

#ifndef IVM_COMMON_TOKEN_STATE_INIT
	#define IVM_COMMON_TOKEN_STATE_INIT 0
#endif

#ifndef IVM_COMMON_TOKEN_STATE_UNEXP
	#define IVM_COMMON_TOKEN_STATE_UNEXP 1
#endif

struct err_msg_t {
	ivm_size_t line;
	ivm_size_t pos;

	const ivm_char_t *expect;
	const ivm_char_t *given;
};

#define ERR_MSG(l, p, e, g) ((struct err_msg_t) { (l), (p), (e), (g) })

struct token_t {
	ivm_int_t id;
	ivm_size_t len;
	const ivm_char_t *val;
	
	ivm_size_t line;
	ivm_size_t pos;
};

struct trans_entry_t {
	const ivm_char_t *match;
	ivm_int_t to_state;
	ivm_int_t save;
	ivm_bool_t ign;
	ivm_bool_t exc; // exclude current char
	ivm_bool_t ext; // extend the current token len
	const ivm_char_t *msg;
};

#define TOKEN_NAME(tid) (token_name_table[tid])

/* error */
#define PARSER_ERR_LP(l, p, ...) \
	IVM_TRACE(IVM_COMMON_PARSER_NAME " parser: at line %zd pos %zd: ", (l), (p)); \
	IVM_TRACE(__VA_ARGS__); \
	IVM_TRACE("\n");

#define PARSER_ERR_P(p, ...) \
	IVM_TRACE(IVM_COMMON_PARSER_NAME " parser: at pos %zd: ", (p)); \
	IVM_TRACE(__VA_ARGS__); \
	IVM_TRACE("\n");

#define PARSER_ERR_L(l, ...) \
	IVM_TRACE(IVM_COMMON_PARSER_NAME " parser: at line %zd: ", (l)); \
	IVM_TRACE(__VA_ARGS__); \
	IVM_TRACE("\n");

#define PARSER_ERR_EM(err) \
	(IVM_TRACE(IVM_COMMON_PARSER_NAME " parser: at line %zd pos %zd: unexpected %s, expecting %s \n", \
			   (err)->line, (err)->pos, (err)->given, (err)->expect))

#define PARSER_ERR_MSG_ILLEGAL_REG									("illegal regular")
#define PARSER_ERR_MSG_UNEXPECTED_CHAR(c, state)					"unexpected character '%c' in state %d", (c), (state)
#define PARSER_ERR_MSG_UNEXPECTED_ENDING(state)						"unexpected ending(still in state %d)", (state)

IVM_PRIVATE
IVM_INLINE
ivm_bool_t
_is_match(const char c,
		  const char *reg)
{
	ivm_size_t len = strlen(reg);

	IVM_ASSERT(len, PARSER_ERR_MSG_ILLEGAL_REG);

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

IVM_PRIVATE
IVM_INLINE
void
_ivm_parser_dumpToken(ivm_list_t *tokens)
{
	ivm_int_t i, size = ivm_list_size(tokens);
	struct token_t *tmp;

	for (i = 0; i < size; i++) {
		tmp = (struct token_t *)ivm_list_at(tokens, i);

		IVM_TRACE("token %-25svalue '%.*s'(len %zd)\n",
				  token_name_table[tmp->id],
				  (int)tmp->len, tmp->val, tmp->len);
	}

	return;
}

IVM_PRIVATE
IVM_INLINE
ivm_list_t *
_ivm_parser_tokenizer(const ivm_char_t *src, struct trans_entry_t trans_map[][IVM_COMMON_MAX_TOKEN_RULE])
{
	ivm_list_t *ret = ivm_list_new(sizeof(struct token_t));
	ivm_int_t state = IVM_COMMON_TOKEN_STATE_INIT;

	const ivm_char_t *c = src;
	ivm_char_t cur_c;
	struct trans_entry_t *tmp_entry;
	ivm_size_t line = 1;
	ivm_ptr_t col = (ivm_ptr_t)c;
	ivm_bool_t has_exc = IVM_FALSE;
	struct token_t tmp_token = (struct token_t) { .len = 0, .val = c, .line = line, .pos = (ivm_ptr_t)c - col + 1 };

	if (c && *c != '\0') {
		do {
			cur_c = *c;
			// IVM_TRACE("matching %c state %d\n", *c, state);

			// if used exc before, skip the current char
			if (!has_exc) {
				if (cur_c == '\n') {
					line++;
					col = (ivm_ptr_t)c;
				}
			} else {
				has_exc = IVM_FALSE;
			}

			for (tmp_entry = trans_map[state];
				 tmp_entry->match;
				 tmp_entry++) {
				if (_is_match(cur_c, tmp_entry->match)) {
					// tmp_token.val += tmp_entry->s_ofs;
					// tmp_token.len += tmp_entry->ofs;

					if (tmp_entry->ign) {
						tmp_token = ((struct token_t) { .len = 0, .val = c + 1, .line = line, .pos = (ivm_ptr_t)c - col + 1 });
					} else if (tmp_entry->save != IVM_COMMON_TOKEN_NONE) { // save to token stack
						if (tmp_entry->ext || tmp_entry->to_state == state)
							tmp_token.len++;

						tmp_token.id = tmp_entry->save;
						ivm_list_push(ret, &tmp_token);

						if (tmp_entry->exc || tmp_entry->to_state == state) {
							tmp_token = ((struct token_t) { .len = 0, .val = c + 1, .line = line, .pos = (ivm_ptr_t)c - col + 1 });
						} else {
							c--;
							has_exc = IVM_TRUE;
							tmp_token = ((struct token_t) { .len = 0, .val = c + 1, .line = line, .pos = (ivm_ptr_t)c - col + 2 });
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
				PARSER_ERR_LP(line, (ivm_ptr_t)c - col + 1, PARSER_ERR_MSG_UNEXPECTED_CHAR(*c, state));
				state = IVM_COMMON_TOKEN_STATE_UNEXP;
				c--;
			}

			c++;
		} while (cur_c != '\0' || has_exc);
	}

	if (state != IVM_COMMON_TOKEN_STATE_INIT) {
		PARSER_ERR_LP(line, (ivm_ptr_t)c - col + 1, PARSER_ERR_MSG_UNEXPECTED_ENDING(state));
	}

	return ret;
}

/* tokenizer */
#define TOKENIZE(src, ...) (_ivm_parser_tokenizer(src, (struct trans_entry_t [][IVM_COMMON_MAX_TOKEN_RULE]){ __VA_ARGS__ }))

/* syntax parser */
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

#define RULE_NAME(name) _ivm_parser_rule_##name

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
#define TOKEN_AT(i) (__toks__[i])

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

#define RULE_START(name, env, ret, tokens, suc) \
	ivm_size_t __i__ = 0; \
	struct err_msg_t __last_err__; \
	(suc) = RULE_NAME(name)((env), (ret), (tokens), &__i__, &__last_err__) && __i__ == ivm_list_size(tokens);

#endif

IVM_COM_END

#endif
