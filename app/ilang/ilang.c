#include "pub/type.h"
#include "pub/vm.h"
#include "pub/obj.h"
#include "pub/inlines.h"

#include "std/list.h"
#include "std/io.h"

#include "util/perf.h"
#include "util/console.h"

#include "vm/native/native.h"
#include "vm/native/priv.h"
#include "vm/mod/mod.h"
#include "vm/dbg.h"
#include "vm/env.h"
#include "vm/serial.h"

#include "gen/gen.h"
#include "parser.h"

IVM_NATIVE_FUNC(print)
{
	ivm_object_t *obj;

	CHECK_ARG_COUNT("print", 1);

	obj = NAT_ARG_AT(1);

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

	return IVM_NULL_OBJ(NAT_STATE());
}

IVM_NATIVE_FUNC(call)
{
	ivm_function_object_t *func;

	CHECK_ARG_COUNT("call", 1);

	func = IVM_AS(NAT_ARG_AT(1), ivm_function_object_t);

	IVM_ASSERT(IVM_IS_TYPE(func, IVM_FUNCTION_OBJECT_T),
			   IVM_ERROR_MSG_NOT_TYPE("function", IVM_OBJECT_GET(func, TYPE_NAME)));

	ivm_function_object_invoke(func, NAT_STATE(), NAT_CORO());

	return ivm_coro_resume(NAT_CORO(), NAT_STATE(), IVM_NULL);
}

IVM_NATIVE_FUNC(eval)
{
	const ivm_string_t *str;
	ilang_gen_trans_unit_t *unit = IVM_NULL;
	ivm_exec_unit_t *exec_unit = IVM_NULL;
	ivm_function_t *root;
	ivm_object_t *ret = IVM_NULL;

	MATCH_ARG("s", &str);

	unit = ilang_parser_parseSource("<eval>", ivm_string_trimHead(str), IVM_FALSE);

	if (!unit) goto FAILED;

	exec_unit = ilang_gen_generateExecUnit_c(unit, ivm_vmstate_getLinkOffset(NAT_STATE()));

	if (!exec_unit) goto FAILED;
	// ivm_dbg_printExecUnit(exec_unit, stderr);

	root = ivm_exec_unit_mergeToVM(exec_unit, NAT_STATE());
	ilang_gen_trans_unit_free(unit); unit = IVM_NULL;
	ivm_exec_unit_free(exec_unit); exec_unit = IVM_NULL;

	ivm_function_invoke_r(root, NAT_STATE(), NAT_CORO(), NAT_CONTEXT());
	// remove local context
	ivm_runtime_removeContextNode(IVM_CORO_GET(NAT_CORO(), RUNTIME), NAT_STATE());
	ret = ivm_coro_resume(NAT_CORO(), NAT_STATE(), IVM_NULL);

FAILED:
	ilang_gen_trans_unit_free(unit);
	ivm_exec_unit_free(exec_unit);

	return ret ? ret : IVM_NULL_OBJ(NAT_STATE());
}

IVM_PRIVATE
IVM_INLINE
ivm_exec_unit_t *
_parse_source(const ivm_char_t *path)
{
	ivm_file_t *file;
	ivm_char_t *src = IVM_NULL;
	ilang_gen_trans_unit_t *t_unit = IVM_NULL;
	ivm_exec_unit_t *ret = IVM_NULL;

	file = ivm_file_new(path, IVM_FMODE_READ_BINARY);

	if (!file) goto FAILED;
	
	src = ivm_file_readAll(file);
	ivm_file_free(file);

	if (!src) goto FAILED;

	t_unit = ilang_parser_parseSource(path, src, IVM_FALSE);

	if (!t_unit) goto FAILED;

	ret = ilang_gen_generateExecUnit_c(t_unit, 0);

FAILED:
	MEM_FREE(src);
	ilang_gen_trans_unit_free(t_unit);

	return ret;
}

IVM_PRIVATE
ivm_object_t *
ilang_mod_loadSource(const ivm_char_t *path,
					 const ivm_char_t **err,
					 ivm_vmstate_t *state,
					 ivm_coro_t *coro,
					 ivm_context_t *context)
{
	const ivm_char_t *tmp_err = IVM_NULL;
	ivm_exec_unit_t *unit = _parse_source(path);
	ivm_object_t *ret = IVM_NULL;
	ivm_function_t *root;
	ivm_context_t *dest;

	if (!unit) {
		tmp_err = IVM_ERROR_MSG_FAILED_TO_PARSE_SOURCE;
		goto FAILED;
	}

	// set load offset
	ivm_exec_unit_setOffset(unit, ivm_vmstate_getLinkOffset(state));
	root = ivm_exec_unit_mergeToVM(unit, state);
	ivm_exec_unit_free(unit);

	IVM_ASSERT(root, "impossible");

	ivm_function_invoke_r(root, state, coro, IVM_NULL);
	dest = ivm_context_addRef(ivm_coro_getRuntimeGlobal(coro));
	ivm_coro_resume(coro, state, IVM_NULL);

	ret = ivm_object_new_t(state, ivm_context_getSlotTable(dest));

	ivm_context_free(dest, state);

FAILED:

	if (err) {
		*err = tmp_err;
	}

	return ret;
}

ivm_list_t *
_ilang_parser_getTokens(const ivm_char_t *src);

ilang_gen_trans_unit_t *
_ivm_parser_parseToken(ivm_list_t *tokens, ivm_bool_t *suc);

int main(int argc, const char **argv)
{
	ivm_env_init();
	ivm_mod_addModSuffix(".ink", ilang_mod_loadSource);

	ivm_char_t *src = IVM_NULL;
	ilang_gen_trans_unit_t *unit = IVM_NULL;
	ivm_exec_unit_t *exec_unit;
	ivm_vmstate_t *state;
	ivm_context_t *ctx;
	
	ivm_serial_exec_unit_t *s_unit;

	const ivm_char_t *tmp_str, *src_path = IVM_NULL;
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
				src_path = tmp_str;
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
	unit = ilang_parser_parseSource(src_path, src, cfg_debug);
	PROF_END();

	if (!unit) goto CLEAN;

	PROF_START();
	exec_unit = ilang_gen_generateExecUnit(unit);
	PROF_END();

	if (!exec_unit) goto CLEAN;
	if (cfg_debug)
		ivm_dbg_printExecUnit(exec_unit, stderr);

	ilang_gen_trans_unit_free(unit);
	unit = IVM_NULL;

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

		ctx = ivm_coro_getRuntimeGlobal(ivm_vmstate_curCoro(state));

		ivm_context_setSlot_r(ctx, state, "print", IVM_NATIVE_WRAP(state, print));
		ivm_context_setSlot_r(ctx, state, "call", IVM_NATIVE_WRAP(state, call));
		ivm_context_setSlot_r(ctx, state, "eval", IVM_NATIVE_WRAP(state, eval));
		ivm_context_setSlot_r(ctx, state, "null", IVM_NULL_OBJ(state));
		ivm_context_setSlot_r(ctx, state, "undefined", IVM_UNDEFINED(state));

		ivm_vmstate_unlockGCFlag(state);

		// execute
		PROF_START();
		ivm_vmstate_schedule(state);
		PROF_END();

		ivm_vmstate_free(state);
	}

CLEAN:
	ilang_gen_trans_unit_free(unit);
	MEM_FREE(src);
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

	ivm_env_clean();

	return 0;
}
