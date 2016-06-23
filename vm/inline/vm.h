#ifndef _IVM_VM_INLINE_VM_H_
#define _IVM_VM_INLINE_VM_H_

#include "pub/com.h"
#include "pub/mem.h"
#include "pub/vm.h"

IVM_COM_HEADER

/* context pool */
#define ivm_vmstate_allocContext(state, len) \
	(ivm_context_pool_alloc((state)->ct_pool, (len)))
#define ivm_vmstate_dumpContext(state, ct) \
	(ivm_context_pool_dump((state)->ct_pool, (ct)))

IVM_COM_END

#endif
