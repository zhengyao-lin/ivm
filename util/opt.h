#ifndef _IVM_UTIL_OPT_H_
#define _IVM_UTIL_OPT_H_

#include "pub/com.h"
#include "pub/const.h"
#include "pub/type.h"
#include "pub/vm.h"

#include "std/list.h"
#include "std/string.h"

IVM_COM_HEADER

typedef ivm_list_t ivm_opt_instr_list_t;

typedef struct ivm_opt_instr_t_tag {
	ivm_byte_t opc;
	ivm_opcode_arg_t arg;
	ivm_size_t addr;
	struct ivm_opt_instr_t_tag *jmpto;
	ivm_ptlist_t *refs;
} ivm_opt_instr_t;

#define ivm_opt_instr_build(...) ((ivm_opt_instr_t) { __VA_ARGS__ })

typedef IVM_LIST_ITER_TYPE(ivm_opt_instr_t) ivm_opt_instr_list_iterator_t;

#define ivm_opt_instr_list_new(size) (ivm_list_new_c(sizeof(ivm_opt_instr_t), (size)))
#define ivm_opt_instr_list_free ivm_list_free
#define ivm_opt_instr_list_push ivm_list_push
#define ivm_opt_instr_list_size ivm_list_size
#define ivm_opt_instr_list_at(list, i) ((ivm_opt_instr_t *)ivm_list_at((list), (i)))

#define IVM_OPT_INSTR_LIST_ITER_GET(iter) (IVM_LIST_ITER_GET_PTR(iter, ivm_opt_instr_t))
#define IVM_OPT_INSTR_LIST_EACHPTR(list, iter) IVM_LIST_EACHPTR((list), iter, ivm_opt_instr_t)

typedef struct {
	ivm_string_pool_t *pool;
	ivm_opt_instr_list_t *instrs;
	ivm_bool_t cached;
} ivm_opt_il_t;

void
ivm_opt_il_free(ivm_opt_il_t *il);

ivm_opt_il_t *
ivm_opt_il_convertFromExec(ivm_exec_t *exec);

void
ivm_opt_il_generateExec(ivm_opt_il_t *il,
						ivm_exec_t *dest);

void
ivm_opt_optExec(ivm_exec_t *exec);

IVM_COM_END

#endif
