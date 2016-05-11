#ifndef _IVM_VM_DBG_H_
#define _IVM_VM_DBG_H_

#include <stdio.h>
#include "type.h"
#include "exec.h"

/* output bytecodes in a readable form */
void
ivm_dbg_disAsmExec(ivm_exec_t *exec, const char *prefix, FILE *fp);

#endif
