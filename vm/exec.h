#ifndef _IVM_VM_EXEC_H_
#define _IVM_VM_EXEC_H_

#include "pub/const.h"
#include "std/list.h"
#include "type.h"
#include "str.h"
#include "op.h"

#if IVM_DEBUG

#define IVM_DEFAULT_EXEC_BUFFER_SIZE 1
#define IVM_DEFAULT_EXEC_LIST_BUFFER_SIZE 1

#else

#define IVM_DEFAULT_EXEC_BUFFER_SIZE 32
#define IVM_DEFAULT_EXEC_LIST_BUFFER_SIZE 32

#endif

typedef struct {
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
 *    %i8, %i16, %i32, %i64: write integer of size 8-16
 *    %s: write string
 *    %%: write a character '%'
 *    other: write exactly what it is
 */
ivm_size_t
ivm_exec_addCode(ivm_exec_t *exec,
				 ivm_opcode_t op,
				 ivm_char_t *format, ...);
void
ivm_exec_rewrite(ivm_exec_t *exec,
				 ivm_size_t addr,
				 ivm_opcode_t op,
				 ivm_char_t *format, ...);
/* notice: pass in the address of opcode, not the start of argument */
void
ivm_exec_rewriteArg(ivm_exec_t *exec,
					ivm_size_t addr,
					ivm_char_t *format, ...);

#define ivm_exec_opAt(exec, pc) ((ivm_opcode_t)exec->code[pc])
#define ivm_exec_length(exec) (exec->cur)

typedef ivm_size_t ivm_exec_id_t;
typedef ivm_ptlist_t ivm_exec_list_t;

#define ivm_exec_list_new() (ivm_ptlist_new_c(IVM_DEFAULT_EXEC_LIST_BUFFER_SIZE))
#define ivm_exec_list_free ivm_ptlist_free
#define ivm_exec_list_register ivm_ptlist_push
#define ivm_exec_list_size ivm_ptlist_size
#define ivm_exec_list_at(list, i) ((ivm_exec_t *)ivm_ptlist_at((list), (i)))
#define ivm_exec_list_foreach(list, each) (ivm_ptlist_foreach((list), (ivm_ptlist_foreach_proc_t)(each)))

#endif
