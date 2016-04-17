#include "pub/mem.h"
#include "str.h"
#include "type.h"
#include "err.h"

ivm_char_t *
ivm_strdup(const ivm_char_t *src)
{
	ivm_size_t size = sizeof(ivm_char_t) * (IVM_STRLEN(src) + 1);
	ivm_char_t *ret = MEM_ALLOC(size);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("new string"));

	MEM_COPY(ret, src, size);

	return ret;
}
