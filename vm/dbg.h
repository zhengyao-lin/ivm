#ifndef _IVM_VM_DBG_H_
#define _IVM_VM_DBG_H_

#include <stdio.h>
#include "type.h"
#include "exec.h"

void
ivm_dbg_disAsmExec_c(ivm_exec_t *exec, const char *prefix, FILE *fp);

#endif
