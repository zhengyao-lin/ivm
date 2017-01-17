#ifndef _IVM_VM_OPCODE_REQ_H_
#define _IVM_VM_OPCODE_REQ_H_

/* opcode handler requirement */

#include "pub/const.h"
#include "pub/err.h"
#include "pub/vm.h"
#include "pub/obj.h"
#include "pub/inlines.h"

// #include "thread/sched.h"

#include "std/mem.h"

#include "native/native.h"

#include "coro.h"
#include "vmstack.h"
#include "context.h"
#include "call.h"
#include "func.h"
#include "oprt.h"
#include "dbg.h"

#include "opcode.op.h"

IVM_INLINE
void
STACK_FILLIN(ivm_object_t **start,
			 ivm_object_t *obj,
			 ivm_int_t count)
{
	while (count--)
		*start++ = obj;

	return;
}

#if IVM_DISPATCH_METHOD_DIRECT_THREAD
	#include "opcode.dispatch.h"
#endif

#endif
