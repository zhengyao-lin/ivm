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
		if (ivm_vmstate_checkGC(state)) { \
			SAVE_STACK(); \
			ivm_vmstate_doGC(state); \
		} \
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

#define YIELD() tmp_ip++; SAVE_RUNTIME(_RUNTIME, tmp_ip); goto ACTION_YIELD
#define RETURN() tmp_ip++; SAVE_RUNTIME(_RUNTIME, tmp_ip); goto ACTION_RETURN
#define INVOKE() goto ACTION_INVOKE

#define _ARG (_INSTR->arg)

#define _STACK (tmp_stack)
#define STACK_SIZE() (tmp_sp)
#define STACK_TOP() (ivm_vmstack_at(_STACK, tmp_sp - 1))
#define STACK_POP() (tmp_sp--, ivm_vmstack_at(_STACK, tmp_sp))
#define STACK_PUSH(obj) (ivm_vmstack_pushAt(_STACK, tmp_sp, (obj)), ++tmp_sp)
#define STACK_BEFORE(i) (ivm_vmstack_at(_STACK, tmp_sp - 1 - (i)))
#define STACK_CUT(i) (tmp_sp - (i) >= tmp_bp ? (tmp_sp -= (i), ivm_vmstack_ptrAt(_STACK, tmp_sp)) : IVM_NULL)
#define STACK_INC(i) (tmp_sp += (i))

#define AVAIL_STACK (tmp_sp - tmp_bp)
#define CHECK_STACK(req) \
	IVM_ASSERT(AVAIL_STACK >= (req), \
			   IVM_ERROR_MSG_INSUFFICIENT_STACK((req), AVAIL_STACK))

#define SAVE_RUNTIME(runtime, ip) \
	(IVM_RUNTIME_SET((runtime), IP, (ip)), \
	 IVM_RUNTIME_SET((runtime), BP, tmp_bp), \
	 IVM_RUNTIME_SET((runtime), SP, tmp_sp))

#define SAVE_STACK() \
	(IVM_RUNTIME_SET(_RUNTIME, BP, tmp_bp), \
	 IVM_RUNTIME_SET(_RUNTIME, SP, tmp_sp))

#define UPDATE_STACK() \
	(tmp_bp = IVM_RUNTIME_GET(_RUNTIME, BP), \
	 tmp_sp = IVM_RUNTIME_GET(_RUNTIME, SP))

#endif
