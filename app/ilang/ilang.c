#include "pub/type.h"
#include "pub/vm.h"

#include "std/list.h"
#include "std/io.h"

#include "util/perf.h"

#include "parser.h"
#include "gen.h"

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
	ivm_bool_t suc;

	if (argc == 2) {
		fp = ivm_file_new(argv[1], "rb");
		IVM_ASSERT(fp, "cannot open %s", argv[1]);
		src = ivm_file_readAll(fp);
	} else {
		src = "a(fn a, b: a + b).a.b(a + b * v);";
	}

ivm_perf_reset();
ivm_perf_startProfile();

	tokens = _ilang_parser_getTokens(src);

ivm_perf_stopProfile();
ivm_perf_printElapsed();
ivm_perf_reset();
ivm_perf_startProfile();

	unit = _ivm_parser_parseToken(tokens, &suc);
	IVM_TRACE("is legal: %d\n", suc);
	
ivm_perf_stopProfile();
ivm_perf_printElapsed();

	ivm_list_free(tokens);

	if (fp) {
		ivm_file_free(fp);
		ilang_gen_trans_unit_free(unit);
		MEM_FREE(src);
	}

	return 0;
}
