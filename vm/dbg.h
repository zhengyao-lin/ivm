#ifndef _IVM_VM_DBG_H_
#define _IVM_VM_DBG_H_

#include <stdio.h>
#include "pub/com.h"
#include "type.h"
#include "exec.h"
#include "vm.h"
#include "coro.h"
#include "gc/heap.h"

IVM_COM_HEADER

#define IVM_DBG_TAB "   "

/* output bytecodes in a readable form */
void
ivm_dbg_disAsmExec(ivm_exec_t *exec, const char *prefix, FILE *fp);

/* print current heap state */
void
ivm_dbg_heapState(ivm_vmstate_t *state, FILE *fp);

void
ivm_dbg_stackState(ivm_coro_t *coro, FILE *fp);

IVM_COM_END

#endif
