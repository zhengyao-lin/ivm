#ifndef _IVM_VM_CONTEXT_H_
#define _IVM_VM_CONTEXT_H_

#include "pub/com.h"
#include "obj.h"
#include "slot.h"
#include "std/pool.h"

IVM_COM_HEADER

struct ivm_vmstate_t_tag;

typedef ivm_object_t ivm_context_t;

#define ivm_context_new ivm_object_new
#define ivm_context_fromObj(obj) (obj)
#define ivm_context_toObject(context) (context)

typedef void (*ivm_ctchain_foreach_proc_t)(ivm_context_t *, void *);

struct ivm_ctchain_sub_t_tag {
	ivm_context_t *ct;
};

typedef struct ivm_ctchain_t_tag {
	ivm_int_t len;
} ivm_ctchain_t;

#define ivm_ctchain_getSize(len) \
	(sizeof(ivm_ctchain_t) + (sizeof(struct ivm_ctchain_sub_t_tag) * (len)))

#define ivm_ctchain_getContextSize(chain) \
	(sizeof(struct ivm_ctchain_sub_t_tag) * (chain)->len)

#define ivm_ctchain_contextStart(chain) \
	((struct ivm_ctchain_sub_t_tag *) \
	 (&((ivm_ctchain_t *)(chain))[1]))

#define ivm_ctchain_contextEnd(chain) \
	(&((struct ivm_ctchain_sub_t_tag *) \
	   (&((ivm_ctchain_t *)(chain))[1]))[(chain)->len - 1])

#define ivm_ctchain_contextAt(chain, i) \
	(&(((struct ivm_ctchain_sub_t_tag *) \
		 (&((ivm_ctchain_t *) \
			(chain))[1]))[i]))

#define ivm_ctchain_setAt(chain, i, context) \
	(ivm_ctchain_contextAt((chain), (i))->ct = (context))

ivm_ctchain_t *
ivm_ctchain_new(struct ivm_vmstate_t_tag *state, ivm_int_t len);
void
ivm_ctchain_free(ivm_ctchain_t *chain, struct ivm_vmstate_t_tag *state);

#if 0

ivm_context_t *
ivm_ctchain_addContext(ivm_ctchain_t *chain,
					   struct ivm_vmstate_t_tag *state,
					   ivm_context_t *ct);
void
ivm_ctchain_removeContext(ivm_ctchain_t *chain,
						  struct ivm_vmstate_t_tag *state,
						  ivm_context_t *ct);
#endif

ivm_ctchain_t *
ivm_ctchain_appendContext(ivm_ctchain_t *chain,
						  struct ivm_vmstate_t_tag *state,
						  ivm_context_t *ct);

ivm_object_t *
ivm_ctchain_search(ivm_ctchain_t *chain,
				   struct ivm_vmstate_t_tag *state,
				   const ivm_char_t *key);

ivm_ctchain_t *
ivm_ctchain_clone(ivm_ctchain_t *chain,
				  struct ivm_vmstate_t_tag *state);

#define ivm_ctchain_getLocal(chain) (ivm_ctchain_contextStart(chain)->ct)
#define ivm_ctchain_getGlobal(chain) (ivm_ctchain_contextEnd(chain)->ct)

void
ivm_ctchain_setLocalSlot(ivm_ctchain_t *chain,
						 struct ivm_vmstate_t_tag *state,
						 const ivm_char_t *key,
						 ivm_object_t *val);

ivm_bool_t
ivm_ctchain_setSlotIfExist(ivm_ctchain_t *chain,
						   struct ivm_vmstate_t_tag *state,
						   const ivm_char_t *key,
						   ivm_object_t *val);

void
ivm_ctchain_foreach(ivm_ctchain_t *chain,
					ivm_ctchain_foreach_proc_t proc,
					void *arg);

typedef struct ivm_ctchain_sub_t_tag ivm_ctchain_iterator_t;

#define IVM_CTCHAIN_ITER_SET(iter, val) ((iter)->ct = val)
#define IVM_CTCHAIN_ITER_GET(iter) ((iter)->ct)
#define IVM_CTCHAIN_EACHPTR(chain, ptr) \
	for ((ptr) = ivm_ctchain_contextStart(chain); \
		 (ptr) <= ivm_ctchain_contextEnd(chain); (ptr)++)

#define IVM_CONTEXT_POOL_MAX_CACHE_LEN 10

typedef struct {
	ivm_ptpool_t *pools[IVM_CONTEXT_POOL_MAX_CACHE_LEN + 1];
} ivm_context_pool_t;

ivm_context_pool_t *
ivm_context_pool_new(ivm_size_t ecount);

void
ivm_context_pool_free(ivm_context_pool_t *pool);

ivm_ctchain_t *
ivm_context_pool_alloc(ivm_context_pool_t *pool, ivm_int_t len);

ivm_ctchain_t *
ivm_context_pool_realloc(ivm_context_pool_t *pool,
						 ivm_ctchain_t *chain,
						 ivm_int_t len);

void
ivm_context_pool_dump(ivm_context_pool_t *pool, ivm_ctchain_t *chain);

#if 0
typedef ivm_ptpool_t ivm_context_pool_t;

#define ivm_context_pool_new(count) (ivm_ptpool_new((count), sizeof(ivm_ctchain_t)))
#define ivm_context_pool_free ivm_ptpool_free
#define ivm_context_pool_alloc(pool) ((ivm_ctchain_t *)ivm_ptpool_alloc(pool))
#define ivm_context_pool_dump ivm_ptpool_dump

#endif

IVM_COM_END

#endif
