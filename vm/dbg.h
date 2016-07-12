#ifndef _IVM_VM_DBG_H_
#define _IVM_VM_DBG_H_

#include <stdio.h>

#include "pub/com.h"
#include "pub/const.h"
#include "pub/type.h"
#include "pub/vm.h"

#include "std/heap.h"

#include "exec.h"
#include "instr.h"
#include "coro.h"

IVM_COM_HEADER

#define IVM_DBG_TAB IVM_TAB

typedef struct {
	ivm_coro_action_t action;

	ivm_object_t *retval;

#if IVM_STACK_CACHE_N_TOS == 1
	ivm_object_t *stc0;
#elif IVM_STACK_CACHE_N_TOS == 2
	ivm_object_t *stc0, *stc1;
#endif

	ivm_int_t cst;

	ivm_instr_t *ip;
	ivm_size_t bp, sp;

	ivm_int_t cmp_reg;

	ivm_vmstate_t *state;
	ivm_coro_t *coro;
	ivm_vmstack_t *stack;
} ivm_dbg_runtime_t;

/* output bytecodes in a readable form */
void
ivm_dbg_printExec(ivm_exec_t *exec, const char *prefix, FILE *fp);

void
ivm_dbg_printExecUnit(ivm_exec_unit_t *unit, FILE *fp);

/* print current heap state */
void
ivm_dbg_heapState(ivm_vmstate_t *state, FILE *fp);

void
ivm_dbg_stackState(ivm_coro_t *coro, FILE *fp);

void
ivm_dbg_printRuntime(ivm_dbg_runtime_t runtime);

IVM_COM_END

#endif
