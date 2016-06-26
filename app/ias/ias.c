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
#include "util/serial.h"

int main(int argc, const char **argv)
{
	ivm_env_init();

	const ivm_char_t *tmp_str;
	ivm_file_t *src_file = IVM_NULL;
	ivm_file_t *cache_file = IVM_NULL;

	ivm_char_t *src;
	ivm_gen_env_t *env;
	ivm_vmstate_t *state;
	ivm_serial_exec_list_t *s_list;
	ivm_bool_t is_failed = IVM_FALSE;

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

		OPTION("c", "-cache", "<file path>", "compile and save cache file to the specified path", {
			if (!(tmp_str = ARG()->value)) {
				ILLEGAL_ARG();
			} else {
				if (cache_file) {
					ERROR("too many cache file outputs given");
				} else if (!(cache_file = ivm_file_new(tmp_str, IVM_FMODE_WRITE_BINARY))) {
					ERROR("cannot open cache file %s", tmp_str);
				}
			}
		})

		NORMAL({
			if (src_file) {
				ERROR("too many source files given");
			} else if (!(src_file = ivm_file_new(ARG()->value, IVM_FMODE_READ_BINARY))) {
				ERROR("cannot open source file %s", ARG()->value);
			}
		}),

		if (!src_file) {
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

	src = ivm_file_readAll(src_file);
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

	if (cache_file) {
		s_list = ivm_serial_serializeExecList(env->exec_list);
		ivm_serial_execListToFile(s_list, cache_file);
		ivm_serial_exec_list_free(s_list);

		// fflush(cache_file->fp);

		// ivm_file_t *tmp_file = ivm_file_new("test.iobj", "rb");
		// s_list = ivm_serial_execListFromFile(tmp_file);
		// ivm_serial_exec_list_free(s_list);
		// ivm_file_free(tmp_file);
	}

	ivm_vmstate_free(state);
	ivm_gen_env_free(env);
	ivm_file_free(src_file);
	ivm_file_free(cache_file);
	MEM_FREE(src);

	return 0;
}
