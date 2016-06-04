#ifndef _IVM_VM_DISPATCH_DIRECT_H_
#define _IVM_VM_DISPATCH_DIRECT_H_

#define _CORO (coro)
#define _RUNTIME (tmp_runtime)
#define _CONTEXT (tmp_context)
#define _STATE (state)
#define _STRING_POOL (tmp_exec->pool)
#define _INSTR (tmp_ip)

#define NEXT_INSTR() \
	if (++tmp_ip != tmp_ip_end) { \
		ivm_vmstate_checkGC(state); \
		goto *(tmp_ip->entry); \
	} else { \
		goto END_EXEC; \
	}

#define NEXT_N_INSTR(n) \
	if ((tmp_ip += (n)) != tmp_ip_end) { \
		goto *(tmp_ip->entry); \
	} else { \
		goto END_EXEC; \
	} \

#define YIELD() IVM_RUNTIME_SET(_RUNTIME, IP, ++tmp_ip); goto ACTION_YIELD
#define INVOKE() goto ACTION_INVOKE

#define _ARG (_INSTR->arg)

#define _STACK (tmp_stack)
#define STACK_SIZE() (ivm_vmstack_size(_STACK))
#define STACK_TOP() (ivm_vmstack_top(_STACK))
#define STACK_POP() (ivm_vmstack_pop(_STACK))
#define STACK_PUSH(obj) (ivm_vmstack_push(_STACK, (obj)))
#define STACK_BEFORE(i) (ivm_vmstack_before(_STACK, (i)))
#define STACK_CUT(i) (ivm_vmstack_cut(_STACK, (i)))
#define STACK_INC(i) \
	(ivm_vmstack_incTop(_STACK, (i)))

#define FRAME_STACK_TOP() (ivm_frame_stack_top(_CORO->frame_st))
#define AVAIL_STACK (STACK_SIZE() - IVM_FRAME_GET(FRAME_STACK_TOP(), STACK_TOP))
#define CHECK_STACK(req) IVM_ASSERT(AVAIL_STACK >= (req), \
									IVM_ERROR_MSG_INSUFFICIENT_STACK)

#endif
