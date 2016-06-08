#include "pub/mem.h"
#include "pub/com.h"
#include "pub/vm.h"
#include "pub/err.h"

#include "inline/obj.h"
#include "context.h"

#define GET_CONTEXT(chain_sub) ((chain_sub)->ct)

typedef struct ivm_ctchain_sub_t_tag ivm_ctchain_sub_t;

ivm_ctchain_t *
ivm_ctchain_new(ivm_vmstate_t *state, ivm_int_t len)
{
	ivm_ctchain_t *ret = ivm_vmstate_allocContext(state, len);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("context chain"));

	return ret;
}

void
ivm_ctchain_free(ivm_ctchain_t *chain, ivm_vmstate_t *state)
{
	if (chain) {
		ivm_vmstate_dumpContext(state, chain);
	}

	return;
}

ivm_ctchain_t *
ivm_ctchain_appendContext(ivm_ctchain_t *chain,
						  ivm_vmstate_t *state,
						  ivm_context_t *ct)
{
	ivm_ctchain_t *ret;

	if (chain) {
		ret = ivm_vmstate_allocContext(state, chain->len + 1);
		MEM_COPY(ivm_ctchain_contextAt(ret, 1),
				 ivm_ctchain_contextStart(chain),
				 ivm_ctchain_getContextSize(chain));
	} else {
		ret = ivm_vmstate_allocContext(state, 1);
	}

	ivm_ctchain_contextStart(ret)->ct = ct;

	return ret;
}

ivm_ctchain_t *
ivm_ctchain_clone(ivm_ctchain_t *chain,
				  ivm_vmstate_t *state)
{
	ivm_ctchain_t *ret = IVM_NULL;

	if (chain) {
		ret = ivm_vmstate_allocContext(state, chain->len);
		MEM_COPY(ivm_ctchain_contextStart(ret),
				 ivm_ctchain_contextStart(chain),
				 ivm_ctchain_getContextSize(chain));
	}

	return ret;
}

ivm_object_t *
ivm_ctchain_search(ivm_ctchain_t *chain,
				   ivm_vmstate_t *state,
				   const ivm_string_t *key)
{
	ivm_object_t *ret = IVM_NULL;
	ivm_ctchain_sub_t *i;
	ivm_int_t len;

	for (i = ivm_ctchain_contextStart(chain), len = 0;
		 len < chain->len; i++, len++) {
		ret = ivm_object_getSlotValue_np(GET_CONTEXT(i), state, key);
		if (ret) break;
	}

	return ret;
}


void
ivm_ctchain_setLocalSlot(ivm_ctchain_t *chain,
						 ivm_vmstate_t *state,
						 const ivm_string_t *key,
						 ivm_object_t *val)
{
	ivm_context_t *local = ivm_ctchain_getLocal(chain);

#if 0
	if (!local) {
		local = ivm_ctchain_addContext(chain, state, ivm_context_new(state));
	}
#endif

	ivm_object_setSlot(ivm_context_toObject(local),
					   state, key, val);

	return;
}

ivm_bool_t
ivm_ctchain_setSlotIfExist(ivm_ctchain_t *chain,
						   struct ivm_vmstate_t_tag *state,
						   const ivm_string_t *key,
						   ivm_object_t *val)
{
	ivm_bool_t ret = IVM_FALSE;
	ivm_ctchain_sub_t *i;
	ivm_int_t len;

	for (i = ivm_ctchain_contextStart(chain), len = 0;
		 len < chain->len; i++, len++) {
		ret = ivm_object_setSlotIfExist(GET_CONTEXT(i),
										state, key, val);
		if (ret) break;
	}

	return ret;
}

void
ivm_ctchain_foreach(ivm_ctchain_t *chain,
					ivm_ctchain_foreach_proc_t proc,
					void *arg)
{
	ivm_ctchain_sub_t *i;
	ivm_int_t len;

	for (i = ivm_ctchain_contextStart(chain), len = 0;
		 len < chain->len; i++, len++) {
		proc(GET_CONTEXT(i), arg);
	}

	return;
}

ivm_context_pool_t *
ivm_context_pool_new(ivm_size_t ecount)
{
	ivm_context_pool_t *ret = MEM_ALLOC(sizeof(*ret),
										ivm_context_pool_t *);
	ivm_int_t i;

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("context pool"));

	for (i = 0; i <= IVM_CONTEXT_POOL_MAX_CACHE_LEN; i++) {
		ret->pools[i] = ivm_ptpool_new(ecount,
									   ivm_ctchain_getSize(i));
	}

	return ret;
}

void
ivm_context_pool_free(ivm_context_pool_t *pool)
{
	ivm_int_t i;

	if (pool) {
		for (i = 0; i <= IVM_CONTEXT_POOL_MAX_CACHE_LEN; i++) {
			ivm_ptpool_free(pool->pools[i]);
		}

		MEM_FREE(pool);
	}

	return;
}

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

	ret->len = len;

	return ret;
}

ivm_ctchain_t *
ivm_context_pool_realloc(ivm_context_pool_t *pool,
						 ivm_ctchain_t *chain,
						 ivm_int_t len)
{
	return ivm_context_pool_dump(pool, chain),
		   ivm_context_pool_alloc(pool, len);
}

void
ivm_context_pool_dump(ivm_context_pool_t *pool, ivm_ctchain_t *chain)
{
	if (pool) {
		if (chain->len <= IVM_CONTEXT_POOL_MAX_CACHE_LEN) {
			ivm_ptpool_dump(pool->pools[chain->len], chain);
		} else {
			MEM_FREE(chain);
		}
	}

	return;
}
