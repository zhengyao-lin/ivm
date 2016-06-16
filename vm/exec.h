#ifndef _IVM_VM_EXEC_H_
#define _IVM_VM_EXEC_H_

#include "pub/com.h"
#include "pub/const.h"
#include "pub/type.h"

#include "std/list.h"
#include "std/string.h"

#include "opcode.h"
#include "instr.h"

IVM_COM_HEADER

typedef struct ivm_exec_t_tag {
	ivm_string_pool_t *pool;

	ivm_size_t alloc;
	ivm_size_t next;
	ivm_instr_t *instrs;
} ivm_exec_t;

ivm_exec_t *
ivm_exec_new(ivm_string_pool_t *pool);

void
ivm_exec_free(ivm_exec_t *exec);

ivm_size_t
ivm_exec_addInstr_c(ivm_exec_t *exec,
					ivm_instr_t instr);

#define ivm_exec_addInstr(exec, ...) \
	(ivm_exec_addInstr_c((exec), IVM_INSTR_GEN(__VA_ARGS__, (exec))))

#define ivm_exec_registerString(exec, str) (ivm_string_pool_registerRaw((exec)->pool, (str)))
#define ivm_exec_getString(exec, i) (ivm_string_pool_get((exec)->pool, (i)))

#define ivm_exec_length(exec) ((exec)->next)
#define ivm_exec_cur ivm_exec_length
#define ivm_exec_procAt(exec, i) ((exec)->instrs[i].proc)
#define ivm_exec_instrAt(exec, i) ((exec)->instrs[i])
#define ivm_exec_instrPtrAt(exec, i) ((exec)->instrs + (i))
#define ivm_exec_instrPtrStart(exec) ((exec)->instrs)
#define ivm_exec_instrPtrEnd(exec) ((exec)->instrs + (exec)->next)

#define ivm_exec_setArgAt(exec, i, val) ((exec)->instrs[i].arg = (val))

#define ivm_exec_argAt(exec, i) ((exec)->instrs[i].arg)
#define ivm_exec_opcAt(exec, i) ((exec)->instrs[i].opc)

typedef ivm_size_t ivm_exec_id_t;
typedef ivm_ptlist_t ivm_exec_list_t;

#define ivm_exec_list_new() (ivm_ptlist_new_c(IVM_DEFAULT_EXEC_LIST_BUFFER_SIZE))
#define ivm_exec_list_free ivm_ptlist_free
#define ivm_exec_list_register ivm_ptlist_push
#define ivm_exec_list_size ivm_ptlist_size
#define ivm_exec_list_at(list, i) ((ivm_exec_t *)ivm_ptlist_at((list), (i)))

IVM_COM_END

#endif
