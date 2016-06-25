#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "pub/const.h"
#include "pub/inlines.h"

#include "std/pool.h"
#include "std/chain.h"
#include "std/hash.h"
#include "std/heap.h"

#include "vm/vm.h"
#include "vm/dbg.h"
#include "vm/err.h"
#include "vm/env.h"
#include "vm/opcode.h"
#include "vm/gc/gc.h"

#include "std/io.h"

#include "util/parser.h"
#include "util/perf.h"
#include "util/gen.h"
#include "util/console.h"

int main(int argc, const char **argv)
{
	ivm_env_init();

	const ivm_char_t *tmp_str;
	ivm_file_t *file = IVM_NULL;
	ivm_char_t *src;
	ivm_gen_env_t *env;
	ivm_vmstate_t *state;
	ivm_bool_t is_failed;

	ivm_bool_t cfg_prof = IVM_TRUE;

#define OPTION IVM_CONSOLE_ARG_DIRECT_MATCH_OPTION
#define NORMAL IVM_CONSOLE_ARG_DIRECT_MATCH_STRING
#define DEFAULT IVM_CONSOLE_ARG_DIRECT_DEFAULT
#define ARG IVM_CONSOLE_ARG_DIRECT_CUR
#define HELP IVM_CONSOLE_ARG_DIRECT_PRINT_HELP
#define FAILED IVM_CONSOLE_ARG_DIRECT_FAILED
#define ERROR IVM_CONSOLE_ARG_DIRECT_ERROR
#define ILLEGAL_ARG IVM_CONSOLE_ARG_DIRECT_ILLEGAL_ARG

	IVM_CONSOLE_ARG_DIRECT("ias", "1.0", argc, argv,
		OPTION("e", "-example", IVM_NULL, "just an example(don't use me)", {
			FAILED(IVM_TRUE, "unexpected option");
		})

		OPTION("p", "-profile", "[enable|disable]", "enable(as default)/disable performance profile", {
			if (!(tmp_str = ARG()->value)) {
				cfg_prof = IVM_TRUE;
			} else {
				if (!IVM_STRCMP(tmp_str, "enable")) {
					cfg_prof = IVM_TRUE;
				} else if (!IVM_STRCMP(tmp_str, "disable")) {
					cfg_prof = IVM_FALSE;
				} else {
					ILLEGAL_ARG();
				}
			}
		})

		NORMAL({
			if (file) {
				ERROR("too many files given");
			} else {
				if (!(file = ivm_file_new(ARG()->value, "r"))) {
					ERROR("cannot open file %s", ARG()->value);
				}
			}
		}),

		if (!file) {
			FAILED(IVM_FALSE, "no available source file given");
		},

		is_failed
	);

#undef OPTION
#undef NORMAL
#undef DEFAULT
#undef ARG
#undef HELP
#undef FAILED
#undef ERROR
#undef ILLEGAL_ARG

	if (is_failed) return 1;

	src = ivm_file_readAll(file);
	env = ivm_parser_parseSource(src);
	state = ivm_gen_env_generateVM(env);

if (cfg_prof) {
	ivm_perf_reset();
	ivm_perf_startProfile();
}

	ivm_vmstate_schedule(state);

if (cfg_prof) {
	ivm_perf_stopProfile();
	ivm_perf_printElapsed();
}

	ivm_vmstate_free(state);
	ivm_gen_env_free(env);
	ivm_file_free(file);
	MEM_FREE(src);

	return 0;
}
