#ifndef _IVM_VM_INLINE_CONTEXT_H_
#define _IVM_VM_INLINE_CONTEXT_H_

#include "pub/com.h"
#include "pub/vm.h"

#include "std/string.h"

#include "../inline/vm.h"
#include "../obj.h"
#include "../context.h"

IVM_COM_HEADER

#define ivm_ctchain_free(chain, state) (ivm_vmstate_dumpContext((state), (chain)))

IVM_INLINE
ivm_ctchain_t *
ivm_context_pool_alloc(ivm_context_pool_t *pool, ivm_int_t len)
{
	ivm_ctchain_t *ret;

	if (len <= IVM_CONTEXT_POOL_MAX_CACHE_LEN) {
		ret = (ivm_ctchain_t *)
			  ivm_ptpool_alloc(pool->pools[len]);
	} else {
		ret = MEM_ALLOC(ivm_ctchain_getSize(len),
						ivm_ctchain_t *);
		IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("context chain"));
	}

	ret->ref = 0;
	ret->len = len;

	return ret;
}

IVM_INLINE
void
ivm_context_pool_dump(ivm_context_pool_t *pool,
					  ivm_ctchain_t *chain)
{
	if (pool && chain) {
		if (!--chain->ref) {
			if (chain->len <= IVM_CONTEXT_POOL_MAX_CACHE_LEN) {
				ivm_ptpool_dump(pool->pools[chain->len], chain);
			} else {
				MEM_FREE(chain);
			}
		}
	}

	return;
}

IVM_INLINE
ivm_ctchain_t *
ivm_context_pool_realloc(ivm_context_pool_t *pool,
						 ivm_ctchain_t *chain,
						 ivm_int_t len)
{
	return ivm_context_pool_dump(pool, chain),
		   ivm_context_pool_alloc(pool, len);
}

IVM_COM_END

#endif
