#include "pub/mem.h"
#include "str.h"
#include "type.h"
#include "err.h"

ivm_char_t *
ivm_strdup(const ivm_char_t *src)
{
	ivm_size_t len = IVM_STRLEN(src);
	ivm_char_t *ret = MEM_ALLOC(sizeof(ivm_char_t) * (len + 1));
	ivm_char_t *tmp = ret;

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("new string"));

	while (len && *src) {
		*(tmp++) = *(src++);
	}

	*tmp = '\0';

	return ret;
}
