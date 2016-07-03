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

#include "util/perf.h"
#include "util/console.h"
#include "util/serial.h"

#include "parser.h"
#include "gen.h"

int main(int argc, const char **argv)
{
	ivm_env_init();

	const ivm_char_t *tmp_str;

	ivm_file_t *src_file = IVM_NULL;
	ivm_file_t *cache_file = IVM_NULL;
	ivm_file_t *output_cache = IVM_NULL;

	enum {
		FORMAT_AUTO,
		FORMAT_IASM,
		FORMAT_IOBJ
	} file_format = FORMAT_AUTO; // auto

	ivm_char_t *src;
	const ivm_char_t *cache_file_path;
	ias_gen_env_t *env;
	ivm_exec_unit_t *unit = IVM_NULL;
	ivm_vmstate_t *state = IVM_NULL;
	ivm_serial_exec_unit_t *s_unit;
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

		OPTION("f", "-format", "[auto|iasm|iobj]",
			   "specify input file format(source file(iasm) or cache file(iobj), use BEFORE file argument)", {
			if ((tmp_str = ARG()->value) != IVM_NULL) {
				if (!IVM_STRCMP(tmp_str, "auto")) {
					file_format = FORMAT_AUTO;
				} else if (!IVM_STRCMP(tmp_str, "iasm")) {
					file_format = FORMAT_IASM;
				} else if (!IVM_STRCMP(tmp_str, "iobj")) {
					file_format = FORMAT_IOBJ;
				} else {
					ILLEGAL_ARG();
				}
			} else {
				ILLEGAL_ARG();
			}
		})

		OPTION("c", "-cache", "<file path>", "compile and save cache file to the specified path", {
			if (!(tmp_str = ARG()->value)) {
				ILLEGAL_ARG();
			} else {
				if (output_cache) {
					ERROR("too many cache file outputs given");
				} else if (!(output_cache = ivm_file_new(tmp_str, IVM_FMODE_WRITE_BINARY))) {
					ERROR("cannot open cache file %s", tmp_str);
				}
			}
		})

		NORMAL({
			if (src_file || cache_file) {
				ERROR("too many source/cache files given");
			} else {
				tmp_str = ARG()->value;

				if (file_format == FORMAT_AUTO) {
					if (ivm_console_arg_hasSuffix(tmp_str, IVM_IASM_FILE_SUFFIX)) {
						file_format = FORMAT_IASM;
					} else if (ivm_console_arg_hasSuffix(tmp_str, IVM_IOBJ_FILE_SUFFIX)) {
						file_format = FORMAT_IOBJ;
					} else {
						FAILED(IVM_FALSE, "unrecognizable file format of file %s", tmp_str);
					}
				}

				switch (file_format) {
					case FORMAT_IASM:
						src_file = ivm_file_new(tmp_str, IVM_FMODE_READ_BINARY);
						if (!src_file) {
							ERROR("cannot open source file %s", tmp_str);
						}
						break;
					case FORMAT_IOBJ:
						cache_file_path = tmp_str;
						cache_file = ivm_file_new(tmp_str, IVM_FMODE_READ_BINARY);
						if (!cache_file) {
							ERROR("cannot open cache file %s", tmp_str);
						}
						break;
					default: IVM_FATAL("unexpected file format");
				}
			}
		}),

		if (!(src_file || cache_file)) {
			FAILED(IVM_FALSE, "no available source/cache file given");
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

	if (src_file) {
		src = ivm_file_readAll(src_file);
		env = ias_parser_parseSource(src);
		unit = ias_gen_env_generateExecUnit(env);

		if (output_cache) {
			s_unit = ivm_serial_serializeExecUnit(unit);
			ivm_serial_execUnitToFile(s_unit, output_cache);

			ivm_serial_exec_unit_free(s_unit);
			ivm_exec_unit_free(unit);
			unit = IVM_NULL;
		}

		ias_gen_env_free(env);
		MEM_FREE(src);
	} else if (cache_file) {
		s_unit = ivm_serial_execUnitFromFile(cache_file);
		if (s_unit) {
			unit = ivm_serial_unserializeExecUnit(s_unit);
			ivm_serial_exec_unit_free(s_unit);
		} else {
			IVM_FATAL(IVM_ERROR_MSG_FILE_FORMAT_ERR(cache_file_path, "iobj"));
		}
	}

	if (unit) {
		state = ivm_exec_unit_generateVM(unit);

		if (cfg_prof) {
			ivm_perf_reset();
			ivm_perf_startProfile();
		}

		ivm_vmstate_schedule(state);

		if (cfg_prof) {
			ivm_perf_stopProfile();
			ivm_perf_printElapsed();
		}
	}

	ivm_exec_unit_free(unit);
	ivm_vmstate_free(state);
	ivm_file_free(src_file);
	ivm_file_free(cache_file);
	ivm_file_free(output_cache);

	return 0;
}
