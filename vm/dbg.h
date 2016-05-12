#ifndef _IVM_VM_DBG_H_
#define _IVM_VM_DBG_H_

#include <stdio.h>
#include "type.h"
#include "exec.h"
#include "vm.h"
#include "gc/heap.h"

#define IVM_DBG_TAB "   "

/* output bytecodes in a readable form */
void
ivm_dbg_disAsmExec(ivm_exec_t *exec, const char *prefix, FILE *fp);

/* print current heap state */
void
ivm_dbg_heapState(ivm_vmstate_t *state, FILE *fp);

#endif
