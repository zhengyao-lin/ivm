#ifndef _IVM_UTIL_CONSOLE_H_
#define _IVM_UTIL_CONSOLE_H_

#include "pub/com.h"
#include "pub/type.h"

#include "std/list.h"
#include "std/string.h"

typedef struct {
	const ivm_char_t *name;
	const ivm_char_t *alias;
	const ivm_char_t *value;
	const ivm_char_t *intro;
} ivm_console_option_t;

#define ivm_console_option_build(...) \
	((ivm_console_option_t) { __VA_ARGS__ })

typedef struct {
	const ivm_char_t *prog;
	const ivm_char_t *vers;
	const ivm_char_t *path;
	ivm_size_t ocount;
	ivm_console_option_t *options;
} ivm_console_help_info_t;

#define ivm_console_help_info_build(...) \
	((ivm_console_help_info_t) { __VA_ARGS__ })

void
ivm_console_printHelp_c(ivm_console_help_info_t info,
						ivm_size_t err_at);

/*
 * arg types:
 * 1. option:
 * 		-name(:<value>)?
 * 2. string:
 * 		[^-]*
 */
typedef struct {
	const ivm_char_t *name;
	ivm_size_t nlen;
	const ivm_char_t *value;
} ivm_console_arg_t;

typedef ivm_list_t ivm_console_arg_list_t;
typedef IVM_LIST_ITER_TYPE(ivm_console_arg_t) ivm_console_arg_list_iterator_t;

#define ivm_console_arg_list_new() (ivm_list_new(sizeof(ivm_console_arg_t)))
#define ivm_console_arg_list_free ivm_list_free
#define ivm_console_arg_list_push ivm_list_push

#define IVM_CONSOLE_ARG_LIST_ITER_SET(iter, val) IVM_LIST_ITER_SET((iter), (val), ivm_console_arg_t)
#define IVM_CONSOLE_ARG_LIST_ITER_GET(iter) IVM_LIST_ITER_GET((iter), ivm_console_arg_t)
#define IVM_CONSOLE_ARG_LIST_ITER_GET_PTR(iter) IVM_LIST_ITER_GET_PTR((iter), ivm_console_arg_t)
#define IVM_CONSOLE_ARG_LIST_EACHPTR(list, iter) IVM_LIST_EACHPTR((list), iter, ivm_console_arg_t)

ivm_console_arg_list_t *
ivm_console_arg_parse(ivm_int_t argc,
					  const ivm_char_t **argv);

#define IVM_CONSOLE_ARG_ERROR(...) \
	IVM_TRACE("error: ");          \
	IVM_TRACE(__VA_ARGS__);        \
	IVM_TRACE("\n");

#define IVM_CONSOLE_ARG_FATAL(...) \
	IVM_TRACE("fatal error: ");    \
	IVM_TRACE(__VA_ARGS__);        \
	IVM_TRACE("\n");

#define IVM_CONSOLE_ARG_DIRECT(prog, vers, argc, argv, def, end_chk, res) \
	do {                                                                                     \
		ivm_console_arg_list_t *__ca_arg_list__ = ivm_console_arg_parse((argc), (argv));     \
		ivm_console_arg_list_iterator_t __ca_aiter__;                                        \
		ivm_list_t *__ca_opt_list__ = ivm_list_new(sizeof(ivm_console_option_t));            \
		ivm_bool_t __ca_is_def__ = IVM_TRUE;                                                 \
		ivm_console_option_t __ca_tmp_opt__;                                                 \
		ivm_console_arg_t *__ca_cur_arg__ = IVM_NULL;                                        \
		ivm_size_t __ca_cur_opt__ = -1;                                                      \
		ivm_bool_t __ca_is_failed__ = IVM_FALSE;                                             \
		const ivm_char_t *__cs_prog__ = (prog);                                              \
		const ivm_char_t *__cs_vers__ = (vers);                                              \
		const ivm_char_t *__ca_path__ = *(argv);                                             \
		__ca_tmp_opt__ = __ca_tmp_opt__;                                                     \
		__ca_is_def__ = __ca_is_def__;                                                       \
		def; /* add definitions */                                                           \
		__ca_is_def__ = IVM_FALSE;                                                           \
		IVM_CONSOLE_ARG_LIST_EACHPTR(__ca_arg_list__, __ca_aiter__) {                        \
			__ca_cur_opt__ = -1;                                                             \
			__ca_cur_arg__ = IVM_CONSOLE_ARG_LIST_ITER_GET_PTR(__ca_aiter__);                \
			do {                                                                             \
				def;                                                                         \
				IVM_CONSOLE_ARG_ERROR("unknown argument '%s'",                               \
									  __ca_cur_arg__->name                                   \
									  ? __ca_cur_arg__->name                                 \
									  : __ca_cur_arg__->value);                              \
			} while (0);                                                                     \
			if (__ca_is_failed__) break;                                                     \
		}                                                                                    \
		if (!__ca_is_failed__) {                                                             \
			__ca_cur_opt__ = -1;                                                             \
			do {                                                                             \
				end_chk;                                                                     \
			} while (0);                                                                     \
		}                                                                                    \
		if (__ca_is_failed__) {                                                              \
			IVM_CONSOLE_ARG_DIRECT_PRINT_HELP();                                             \
		}                                                                                    \
		ivm_console_arg_list_free(__ca_arg_list__);                                          \
		ivm_list_free(__ca_opt_list__);                                                      \
		(res) = __ca_is_failed__;                                                            \
	} while (0)

/* current argument(pointer) */
#define IVM_CONSOLE_ARG_DIRECT_CUR() (__ca_cur_arg__)
#define IVM_CONSOLE_ARG_DIRECT_MATCH_OPTION(opt_name, alias, value, intro, ...) \
	__ca_cur_opt__++;                                                                      \
	if (__ca_is_def__) {                                                                   \
		__ca_tmp_opt__ = ivm_console_option_build((opt_name), (alias), (value), (intro));  \
		ivm_list_push(__ca_opt_list__, &__ca_tmp_opt__);                                   \
	} else if (__ca_cur_arg__ &&                                                           \
			   (!IVM_STRNCMP((opt_name), IVM_STRLEN(opt_name),                             \
							__ca_cur_arg__->name, __ca_cur_arg__->nlen) ||                 \
			    ((alias) &&                                                                \
			   	 !IVM_STRNCMP((alias), IVM_STRLEN(alias),                                  \
							  __ca_cur_arg__->name, __ca_cur_arg__->nlen)))) {             \
		__VA_ARGS__;                                                                       \
		break;                                                                             \
	}

#define IVM_CONSOLE_ARG_DIRECT_MATCH_STRING(...) \
	if (!__ca_is_def__ && !__ca_cur_arg__->name) {   \
		__VA_ARGS__;                                 \
		break;                                       \
	}

#define IVM_CONSOLE_ARG_DIRECT_DEFAULT(...) \
	if (!__ca_is_def__) {          \
		__VA_ARGS__;               \
		break;                     \
	}

#define IVM_CONSOLE_ARG_DIRECT_PRINT_HELP() \
	ivm_console_printHelp_c(                                             \
		ivm_console_help_info_build(                                     \
			__cs_prog__, __cs_vers__, __ca_path__,                       \
			ivm_list_size(__ca_opt_list__),                              \
			(ivm_console_option_t *)ivm_list_core(__ca_opt_list__)       \
		),                                                               \
		__ca_cur_opt__                                                   \
	)

#define IVM_CONSOLE_ARG_DIRECT_ERROR IVM_CONSOLE_ARG_ERROR

#define IVM_CONSOLE_ARG_DIRECT_FAILED(is_opt, ...) \
	IVM_CONSOLE_ARG_FATAL(__VA_ARGS__);                                  \
	__ca_is_failed__ = IVM_TRUE;                                         \
	break;

/* use only under option matching */
#define IVM_CONSOLE_ARG_DIRECT_ILLEGAL_ARG(opt) \
	IVM_CONSOLE_ARG_DIRECT_ERROR(                     \
		"illegal argument for option -%.*s: '%s'",    \
		(int)__ca_cur_arg__->nlen,                    \
		__ca_cur_arg__->name,                         \
		__ca_cur_arg__->value                         \
	)

ivm_bool_t
ivm_console_arg_hasSuffix(const ivm_char_t *arg,
						  const ivm_char_t *suffix);

#endif
