#include "pub/type.h"
#include "pub/vm.h"

#include "std/list.h"

#include "parser.h"

ivm_list_t *
_ilang_parser_getTokens(const ivm_char_t *src);

int main()
{
	ivm_list_t *tokens = _ilang_parser_getTokens(" \
		for (let i = 0, i < 1000, i++) { \
			print((fn (a): a * 2)(i)); \
		} \
	");

	ivm_list_free(tokens);

	return 0;
}
