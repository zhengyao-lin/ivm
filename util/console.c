#include "pub/const.h"
#include "pub/err.h"

#include "std/io.h"

#include "console.h"

void
ivm_console_printHelp_c(ivm_console_help_info_t info,
						ivm_size_t err_at)
{
	ivm_size_t i;
	ivm_console_option_t *opts = info.options;
	ivm_char_t buf[128];

	IVM_TRACE("\n%s %s \n\n", info.prog, info.vers);
	IVM_TRACE("usage:\n" IVM_TAB "%s [options] <file(s), etc.>\n\n", info.path);

	if (info.ocount) {
		if (err_at != (ivm_size_t)-1) {
			IVM_TRACE("options(see '->' for the current error):\n");
		} else {
			IVM_TRACE("options:\n");
		}

#define TAB (int)sizeof(IVM_TAB) - 1, (i == err_at ? "->" : IVM_TAB)

		for (i = 0; i < info.ocount; i++) {
			if (opts[i].alias) {
				if (opts[i].value) {
					IVM_SNPRINTF(buf, sizeof(buf), "%-*s(-%s or -%s):%s",
								 TAB, opts[i].name, opts[i].alias,
								 opts[i].value);
				} else {
					IVM_SNPRINTF(buf, sizeof(buf), "%-*s-%s or -%s",
								 TAB, opts[i].name, opts[i].alias);
				}
			} else {
				if (opts[i].value) {
					IVM_SNPRINTF(buf, sizeof(buf), "%-*s-%s:-%s",
								 TAB, opts[i].name, opts[i].value);
				} else {
					IVM_SNPRINTF(buf, sizeof(buf), "%-*s-%s", TAB, opts[i].name);
				}
			}

			IVM_TRACE("%-47s%s\n", buf, opts[i].intro);
		}
	} else {
		IVM_TRACE("options: none\n");
	}

#undef TAB

	IVM_TRACE("\n" IVM_COPYRIGHT_HELP "\n");

	return;
}

ivm_console_arg_list_t *
ivm_console_arg_parse(ivm_int_t argc,
					  const ivm_char_t **argv)
{
	const ivm_char_t **i, **end, *tmp;
	ivm_console_arg_t tmp_arg = { 0 };
	ivm_console_arg_list_t *ret = ivm_console_arg_list_new();

	for (i = argv + 1, end = argv + argc;
		 i != end; i++) {
		if (**i == '-') {
			tmp_arg.name = tmp = *i + 1;
			while (*tmp != ':' && *tmp != '\0') tmp++; /* find ':' or '\0' */
			tmp_arg.nlen = (ivm_ptr_t)tmp - (ivm_ptr_t)*i - 1;

			if (*tmp != '\0') tmp_arg.value = tmp + 1;
			else tmp_arg.value = IVM_NULL;

			/* IVM_TRACE("option: name: %.*s, value: %s\n",
					  (int)tmp_arg.nlen, tmp_arg.name,
					  tmp_arg.value); */
		} else {
			tmp_arg = (ivm_console_arg_t) {
				IVM_NULL, 0,
				*i
			};
			// IVM_TRACE("string: %s\n", tmp_arg.value);
		}

		ivm_console_arg_list_push(ret, &tmp_arg);
	}

	return ret;
}

ivm_bool_t
ivm_console_arg_hasSuffix(const ivm_char_t *arg,
						  const ivm_char_t *suffix)
{
	ivm_size_t l1 = IVM_STRLEN(arg);
	ivm_size_t l2 = IVM_STRLEN(suffix);
	const ivm_char_t *i;

	if (l1 < l2) return IVM_FALSE;

	for (i = arg + l1 - l2;
		 *i != '\0'; i++, suffix++) {
		if (*i != *suffix) {
			return IVM_FALSE;
		}
	}

	return IVM_TRUE;
}
