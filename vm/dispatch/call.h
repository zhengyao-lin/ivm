#ifndef _IVM_VM_DISPATCH_CALL_H_
#define _IVM_VM_DISPATCH_CALL_H_

#define _CORO (__ivm_coro__)
#define _RUNTIME (__ivm_coro__->runtime)
#define _CONTEXT (__ivm_context__)
#define _STATE (__ivm_state__)
#define _STRING_POOL (__ivm_pool__)
#define _INSTR (*__ivm_instr__)

#define NEXT_INSTR() (*__ivm_instr__)++; return IVM_ACTION_NONE
#define NEXT_N_INSTR(n) (*__ivm_instr__) += (n); return IVM_ACTION_NONE
#define YIELD() (*__ivm_instr__)++; return IVM_ACTION_YIELD
#define RETURN() (*__ivm_instr__)++; return IVM_ACTION_RETURN
#define INVOKE() return IVM_ACTION_INVOKE

#define _ARG (_INSTR->arg)

#define _STACK (__ivm_stack__)
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
