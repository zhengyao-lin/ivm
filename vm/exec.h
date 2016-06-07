#ifndef _IVM_VM_EXEC_H_
#define _IVM_VM_EXEC_H_

#include "pub/com.h"
#include "pub/const.h"
#include "std/list.h"
#include "std/string.h"
#include "type.h"
#include "op.h"
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
ivm_exec_addInstr(ivm_exec_t *exec,
				  ivm_instr_t instr);

#define ivm_exec_registerString(exec, str) (ivm_string_pool_register((exec)->pool, (str)))
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
#define ivm_exec_opAt(exec, i) ((exec)->instrs[i].op)

#define ivm_exec_addOp(exec, ...) \
	(ivm_exec_addInstr((exec), IVM_INSTR_GEN(__VA_ARGS__, (exec))))

typedef ivm_size_t ivm_exec_id_t;
typedef ivm_ptlist_t ivm_exec_list_t;

#define ivm_exec_list_new() (ivm_ptlist_new_c(IVM_DEFAULT_EXEC_LIST_BUFFER_SIZE))
#define ivm_exec_list_free ivm_ptlist_free
#define ivm_exec_list_register ivm_ptlist_push
#define ivm_exec_list_size ivm_ptlist_size
#define ivm_exec_list_at(list, i) ((ivm_exec_t *)ivm_ptlist_at((list), (i)))

#if 0

typedef struct ivm_exec_t_tag {
	ivm_string_pool_t *pool;

	ivm_size_t length;
	ivm_size_t cur;
	ivm_byte_t *code;
} ivm_exec_t;

ivm_exec_t *
ivm_exec_new(ivm_string_pool_t *pool);
void
ivm_exec_free(ivm_exec_t *exec);
void
ivm_exec_compact(ivm_exec_t *exec);

void
ivm_exec_addBuffer(ivm_exec_t *exec);

/**
 * format:
 *    $i8, $i16, $i32, $i64: write integer of size 8-16
 *    $s: write string
 *    $$: write a character '$'
 *    other: write exactly what it is
 */

#define IVM_EXEC_FORMAT_ESC_CHAR '$'

ivm_size_t
ivm_exec_addCode(ivm_exec_t *exec,
				 ivm_opcode_t op,
				 const ivm_char_t *format, ...);
ivm_size_t
ivm_exec_addOp(ivm_exec_t *exec,
			   ivm_opcode_t op, ...);
void
ivm_exec_rewrite(ivm_exec_t *exec,
				 ivm_size_t addr,
				 ivm_opcode_t op,
				 const ivm_char_t *format, ...);
/* notice: pass in the address of opcode, not the start of argument */
void
ivm_exec_rewriteArg(ivm_exec_t *exec,
					ivm_size_t addr,
					const ivm_char_t *format, ...);

void
ivm_exec_toInstrBlock(ivm_exec_t *exec,
					  ivm_exec_t *dest);

#define ivm_exec_opAt(exec, pc) ((ivm_opcode_t)(exec)->code[pc])
#define ivm_exec_offset(exec, pc) (&((exec)->code[pc]))
#define ivm_exec_length(exec) ((exec)->cur)
#define ivm_exec_getString(exec, i) (ivm_string_pool_get((exec)->pool, (i)))

typedef ivm_size_t ivm_exec_id_t;
typedef ivm_ptlist_t ivm_exec_list_t;

#define ivm_exec_list_new() (ivm_ptlist_new_c(IVM_DEFAULT_EXEC_LIST_BUFFER_SIZE))
#define ivm_exec_list_free ivm_ptlist_free
#define ivm_exec_list_register ivm_ptlist_push
#define ivm_exec_list_size ivm_ptlist_size
#define ivm_exec_list_at(list, i) ((ivm_exec_t *)ivm_ptlist_at((list), (i)))
#define ivm_exec_list_foreach(list, each) (ivm_ptlist_foreach((list), (ivm_ptlist_foreach_proc_t)(each)))

#endif

IVM_COM_END

#endif
