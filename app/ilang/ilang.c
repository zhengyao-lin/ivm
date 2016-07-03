#include "pub/type.h"
#include "pub/vm.h"

#include "std/list.h"
#include "std/io.h"

#include "util/perf.h"

#include "parser.h"

ivm_list_t *
_ilang_parser_getTokens(const ivm_char_t *src);

ivm_bool_t
_ivm_parser_tryParse(ivm_list_t *tokens);

int main(int argc, const char **argv)
{
	ivm_char_t *src = IVM_NULL;
	ivm_file_t *fp = IVM_NULL;

	if (argc == 2) {
		fp = ivm_file_new(argv[1], "rb");
		IVM_ASSERT(fp, "cannot open %s", argv[1]);
		src = ivm_file_readAll(fp);
	} else {
		src = "a(fn a, b: a + b).a.b(a + b * v);";
	}

ivm_perf_reset();
ivm_perf_startProfile();

	ivm_list_t *tokens = _ilang_parser_getTokens(src);

ivm_perf_stopProfile();
ivm_perf_printElapsed();

	IVM_TRACE("is legal: %d\n", _ivm_parser_tryParse(tokens));

	ivm_list_free(tokens);

	if (fp) {
		ivm_file_free(fp);
		MEM_FREE(src);
	}

	return 0;
}
