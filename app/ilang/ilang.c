#include "pub/type.h"
#include "pub/vm.h"
#include "pub/obj.h"

#include "std/list.h"
#include "std/io.h"

#include "util/perf.h"

#include "vm/dbg.h"
#include "vm/env.h"

#include "parser.h"
#include "gen.h"

IVM_NATIVE_FUNC(print)
{
	ivm_object_t *obj;

	IVM_ASSERT(arg.argc, "print need at least 1 argument");

	obj = arg.argv[0];

	switch (IVM_TYPE_TAG_OF(obj)) {
		case IVM_NUMERIC_T:
			IVM_TRACE("num: %f\n", ivm_numeric_getValue(obj));
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
	ivm_char_t *src = IVM_NULL;
	ivm_file_t *fp = IVM_NULL;
	ivm_list_t *tokens = IVM_NULL;
	ilang_gen_trans_unit_t *unit = IVM_NULL;
	ivm_exec_unit_t *exec_unit;
	ivm_vmstate_t *state;
	ivm_object_t *global;
	ivm_bool_t suc;

	if (argc == 2) {
		fp = ivm_file_new(argv[1], "rb");
		IVM_ASSERT(fp, "cannot open %s", argv[1]);
		src = ivm_file_readAll(fp);
	} else {
		src = "a(fn a, b: a + b).a.b(a + b * v);";
	}

	ivm_env_init();

/***********************************************/

	ivm_perf_reset();
	ivm_perf_startProfile();

	tokens = _ilang_parser_getTokens(src);

	ivm_perf_stopProfile();
	ivm_perf_printElapsed();

/***********************************************/

	ivm_perf_reset();
	ivm_perf_startProfile();

	unit = _ivm_parser_parseToken(tokens, &suc);
	IVM_TRACE("is legal: %d\n", suc);
	
	ivm_perf_stopProfile();
	ivm_perf_printElapsed();

	ivm_list_free(tokens);

/***********************************************/

	ivm_perf_reset();
	ivm_perf_startProfile();

	exec_unit = ilang_gen_generateExecUnit(unit);
	ivm_dbg_printExecUnit(exec_unit, stderr);

	ivm_perf_stopProfile();
	ivm_perf_printElapsed();

/***********************************************/

	ilang_gen_trans_unit_free(unit);
	if (fp) {
		ivm_file_free(fp);
		MEM_FREE(src);
	}

	state = ivm_exec_unit_generateVM(exec_unit);
	ivm_exec_unit_free(exec_unit);

	ivm_vmstate_lockGCFlag(state);

	global = ivm_coro_getRuntimeGlobal(IVM_VMSTATE_GET(state, CUR_CORO));

	ivm_object_setSlot_r(global, state, "print",
						 ivm_function_object_new(state, IVM_NULL, ivm_function_newNative(state, IVM_GET_NATIVE_FUNC(print))));

	ivm_vmstate_unlockGCFlag(state);

/***********************************************/

	ivm_perf_reset();
	ivm_perf_startProfile();

	ivm_vmstate_schedule(state);

	ivm_perf_stopProfile();
	ivm_perf_printElapsed();

/***********************************************/

	ivm_vmstate_free(state);

	return 0;
}
