#include "pub/type.h"
#include "pub/vm.h"
#include "pub/obj.h"
#include "pub/inlines.h"

#include "std/list.h"
#include "std/io.h"

#include "util/perf.h"
#include "util/console.h"
#include "util/serial.h"

#include "vm/dbg.h"
#include "vm/env.h"

#include "gen/gen.h"
#include "parser.h"

IVM_NATIVE_FUNC(print)
{
	ivm_object_t *obj;

	IVM_ASSERT(arg.argc, "print need at least 1 argument");

	obj = arg.argv[0];

	switch (IVM_TYPE_TAG_OF(obj)) {
		case IVM_NUMERIC_T:
			IVM_TRACE("num: %.3f\n", ivm_numeric_getValue(obj));
			break;
		case IVM_STRING_OBJECT_T:
			IVM_TRACE("str: %s\n", ivm_string_trimHead(ivm_string_object_getValue(obj)));
			break;
		default:
			IVM_TRACE("unable to print the object of type <%s>\n",
					  IVM_OBJECT_GET(obj, TYPE_NAME));
	}

	return IVM_NULL;
}

ivm_list_t *
_ilang_parser_getTokens(const ivm_char_t *src);

ilang_gen_trans_unit_t *
_ivm_parser_parseToken(ivm_list_t *tokens, ivm_bool_t *suc);

int main(int argc, const char **argv)
{
	ivm_env_init();

	ivm_char_t *src = IVM_NULL;
	ilang_gen_trans_unit_t *unit = IVM_NULL;
	ivm_exec_unit_t *exec_unit;
	ivm_vmstate_t *state;
	ivm_context_t *ctx;
	
	ivm_serial_exec_unit_t *s_unit;

	const ivm_char_t *tmp_str;
	ivm_bool_t is_failed = IVM_FALSE;

	/* config */
	ivm_bool_t cfg_prof = IVM_TRUE;
	ivm_bool_t cfg_debug = IVM_FALSE;
	ivm_file_t *output_cache = IVM_NULL;
	ivm_file_t *src_file = IVM_NULL;

#define OPTION IVM_CONSOLE_ARG_DIRECT_MATCH_OPTION
#define NORMAL IVM_CONSOLE_ARG_DIRECT_MATCH_STRING
#define DEFAULT IVM_CONSOLE_ARG_DIRECT_DEFAULT
#define ARG IVM_CONSOLE_ARG_DIRECT_CUR
#define HELP IVM_CONSOLE_ARG_DIRECT_PRINT_HELP
#define FAILED IVM_CONSOLE_ARG_DIRECT_FAILED
#define ERROR IVM_CONSOLE_ARG_DIRECT_ERROR
#define ILLEGAL_ARG IVM_CONSOLE_ARG_DIRECT_ILLEGAL_ARG

	IVM_CONSOLE_ARG_DIRECT("ilang", "1.0", argc, argv,
		OPTION("p", "-profile", "[enable|disable]", "enable(as default)/disable performance profile", {
			if (!(tmp_str = ARG()->value)) {
				cfg_prof = !cfg_prof;
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

		OPTION("d", "-debug", "[disable|enable]", "disable(as default)/enable debug mode", {
			if (!(tmp_str = ARG()->value)) {
				cfg_debug = !cfg_debug;
			} else {
				if (!IVM_STRCMP(tmp_str, "enable")) {
					cfg_debug = IVM_TRUE;
				} else if (!IVM_STRCMP(tmp_str, "disable")) {
					cfg_debug = IVM_FALSE;
				} else {
					ILLEGAL_ARG();
				}
			}
		})

		OPTION("c", "-cache", "<file path>", "compile and save cache file to the specified path", {
			tmp_str = ARG()->value;
			if (!tmp_str) {
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
			tmp_str = ARG()->value;
			if (src_file) {
				ERROR("too many source files given");
			} else {
				src_file = ivm_file_new(tmp_str, IVM_FMODE_READ_BINARY);
				if (!src_file) {
					ERROR("cannot open source file %s", tmp_str);
				}
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

#define PROF_START() \
	if (cfg_prof) {                \
		ivm_perf_reset();          \
		ivm_perf_startProfile();   \
	}

#define PROF_END() \
	if (cfg_prof) {                \
		ivm_perf_stopProfile();    \
		ivm_perf_printElapsed();   \
	}

	PROF_START();
	unit = ilang_parser_parseSource(src, cfg_debug);
	PROF_END();

	if (!unit) return 1;

	PROF_START();
	exec_unit = ilang_gen_generateExecUnit(unit);
	if (cfg_debug)
		ivm_dbg_printExecUnit(exec_unit, stderr);
	PROF_END();

	ilang_gen_trans_unit_free(unit);
	MEM_FREE(src);

	if (output_cache) {
		s_unit = ivm_serial_serializeExecUnit(exec_unit, IVM_NULL);
		ivm_serial_execUnitToFile(s_unit, output_cache);

		ivm_serial_exec_unit_free(s_unit);
		ivm_exec_unit_free(exec_unit);
	} else {
		state = ivm_exec_unit_generateVM(exec_unit);
		ivm_exec_unit_free(exec_unit);

		// set native
		ivm_vmstate_lockGCFlag(state);

		ctx = ivm_coro_getRuntimeGlobal(IVM_VMSTATE_GET(state, CUR_CORO));

		ivm_context_setSlot_r(
			ctx, state, "print",
			ivm_function_object_new(state, IVM_NULL, ivm_function_newNative(state, IVM_GET_NATIVE_FUNC(print)))
		);

		ivm_vmstate_unlockGCFlag(state);

		// execute
		PROF_START();
		ivm_vmstate_schedule(state);
		PROF_END();

		ivm_vmstate_free(state);
	}

	ivm_file_free(src_file);
	ivm_file_free(output_cache);

#if 0
#define PSIZE(type) IVM_TRACE(#type ": %d\n", sizeof(type))
	// IVM_TRACE("vmstate: %d\n", sizeof(ivm_vmstate_t));
	// IVM_TRACE("arg: %d\n", sizeof(ivm_opcode_arg_t));
	// IVM_TRACE("arg: %d\n", sizeof(ivm_instr_cache_t));
/*
	PSIZE(ivm_vmstate_t);
	PSIZE(ivm_heap_t);
	PSIZE(ivm_coro_list_t);
	PSIZE(ivm_type_t);
	PSIZE(ivm_func_list_t);
	PSIZE(ivm_coro_pool_t);
	PSIZE(ivm_uid_gen_t);
	PSIZE(ivm_binop_table_t);
	PSIZE(ivm_uniop_table_t);
	PSIZE(ivm_ptlist_t);
	IVM_TRACE("op count: %d\n", IVM_BINOP_COUNT);
*/
#endif

	return 0;
}
