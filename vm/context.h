#ifndef _IVM_VM_CONTEXT_H_
#define _IVM_VM_CONTEXT_H_

#include "pub/com.h"
#include "pub/const.h"

#include "std/pool.h"
#include "std/ref.h"

#include "obj.h"
#include "slot.h"

IVM_COM_HEADER

struct ivm_vmstate_t_tag;

typedef ivm_object_t ivm_context_t;

#define ivm_context_new ivm_object_new
#define ivm_context_fromObj(obj) (IVM_AS((obj), ivm_context_t))
#define ivm_context_toObject(context) (IVM_AS_OBJ(context))

typedef void (*ivm_ctchain_foreach_proc_t)(ivm_context_t *, void *);

struct ivm_ctchain_sub_t_tag {
	ivm_context_t *ct;
};

typedef struct ivm_ctchain_t_tag {
	IVM_REF_HEADER
	ivm_uint_t len;
} ivm_ctchain_t;

#define ivm_ctchain_getSize(len) \
	(sizeof(ivm_ctchain_t) + (sizeof(struct ivm_ctchain_sub_t_tag) * (len)))

#define ivm_ctchain_getContextSize(chain) \
	(sizeof(struct ivm_ctchain_sub_t_tag) * (chain)->len)

#define ivm_ctchain_contextStart(chain) \
	((struct ivm_ctchain_sub_t_tag *) \
	 (((ivm_ctchain_t *)(chain)) + 1))

#define ivm_ctchain_contextEnd(chain) \
	(((struct ivm_ctchain_sub_t_tag *) \
	   (((ivm_ctchain_t *)(chain)) + 1))+ ((chain)->len - 1))

#define ivm_ctchain_contextAt(chain, i) \
	((((struct ivm_ctchain_sub_t_tag *) \
		 (((ivm_ctchain_t *) \
			(chain)) + 1)) + (i)))

#define ivm_ctchain_setAt(chain, i, context) \
	(ivm_ctchain_contextAt((chain), (i))->ct = (context))

#define ivm_ctchain_addRef(chain) ((chain) ? ivm_ref_inc(chain), (chain) : (chain))

ivm_ctchain_t *
ivm_ctchain_new(struct ivm_vmstate_t_tag *state, ivm_int_t len);

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

ivm_ctchain_t *
ivm_ctchain_clone(ivm_ctchain_t *chain,
				  struct ivm_vmstate_t_tag *state);

#define ivm_ctchain_getLocal(chain) (ivm_ctchain_contextStart(chain)->ct)
#define ivm_ctchain_getGlobal(chain) (ivm_ctchain_contextEnd(chain)->ct)

ivm_object_t *
ivm_ctchain_search(ivm_ctchain_t *chain,
				   struct ivm_vmstate_t_tag *state,
				   const ivm_string_t *key);

ivm_object_t *
ivm_ctchain_search_cc(ivm_ctchain_t *chain,
					  struct ivm_vmstate_t_tag *state,
					  const ivm_string_t *key,
					  ivm_instr_cache_t *cache);

void
ivm_ctchain_setLocalSlot(ivm_ctchain_t *chain,
						 struct ivm_vmstate_t_tag *state,
						 const ivm_string_t *key,
						 ivm_object_t *val);

void
ivm_ctchain_setLocalSlot_cc(ivm_ctchain_t *chain,
							struct ivm_vmstate_t_tag *state,
							const ivm_string_t *key,
							ivm_object_t *val,
							ivm_instr_cache_t *cache);

ivm_bool_t
ivm_ctchain_setSlotIfExist(ivm_ctchain_t *chain,
						   struct ivm_vmstate_t_tag *state,
						   const ivm_string_t *key,
						   ivm_object_t *val);

ivm_bool_t
ivm_ctchain_setSlotIfExist_cc(ivm_ctchain_t *chain,
							  struct ivm_vmstate_t_tag *state,
							  const ivm_string_t *key,
							  ivm_object_t *val,
							  ivm_instr_cache_t *cache);

void
ivm_ctchain_foreach(ivm_ctchain_t *chain,
					ivm_ctchain_foreach_proc_t proc,
					void *arg);

typedef struct ivm_ctchain_sub_t_tag *ivm_ctchain_iterator_t;

#define IVM_CTCHAIN_ITER_SET(iter, val) ((iter)->ct = val)
#define IVM_CTCHAIN_ITER_GET(iter) ((iter)->ct)
#define IVM_CTCHAIN_EACHPTR(chain, ptr) \
	for ((ptr) = ivm_ctchain_contextStart(chain); \
		 (ptr) <= ivm_ctchain_contextEnd(chain); (ptr)++)

typedef struct {
	ivm_ptpool_t *pools[IVM_CONTEXT_POOL_MAX_CACHE_LEN + 1];
} ivm_context_pool_t;

ivm_context_pool_t *
ivm_context_pool_new(ivm_size_t ecount);

void
ivm_context_pool_free(ivm_context_pool_t *pool);

IVM_COM_END

#endif
