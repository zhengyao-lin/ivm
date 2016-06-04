#ifndef _IVM_VM_INLINE_OBJ_H_
#define _IVM_VM_INLINE_OBJ_H_

#include "pub/com.h"
#include "pub/mem.h"
#include "../vm.h"
#include "../runtime.h"

IVM_COM_HEADER

IVM_INLINE
void
ivm_runtime_free(ivm_runtime_t *runtime,
				 ivm_vmstate_t *state)
{
	if (runtime) {
		ivm_ctchain_free(runtime->context, state);
		MEM_FREE(runtime);
	}
	
	return;
}


IVM_COM_END

#endif
