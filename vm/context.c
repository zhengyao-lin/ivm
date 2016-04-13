#include "pub/mem.h"
#include "context.h"

ivm_ctchain_t *
ivm_ctchain_new()
{
	ivm_ctchain_t *ret = MEM_ALLOC_INIT(sizeof(*ret));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("context chain"));

	ret->head = ret->tail = IVM_NULL;

	return ret;
}

void
ivm_ctchain_free(ivm_ctchain_t *chain)
{
	MEM_FREE(chain);
	return;
}
