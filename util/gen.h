#ifndef _IVM_UTIL_GEN_H_
#define _IVM_UTIL_GEN_H_

#include "pub/com.h"
#include "pub/type.h"
#include "pub/vm.h"

#include "std/list.h"
#include "std/pool.h"

IVM_COM_HEADER

typedef struct {
	ivm_size_t line;
	ivm_size_t pos;
} ivm_gen_pos_t;

typedef struct {
	ivm_gen_pos_t pos;

	ivm_char_t type;

	const ivm_char_t *val;
	ivm_size_t len;
} ivm_gen_opcode_arg_t;

#define ivm_gen_opcode_arg_build(l, p, ...) \
	((ivm_gen_opcode_arg_t) { (ivm_gen_pos_t) { (l), (p) }, __VA_ARGS__ })

typedef struct {
	ivm_gen_pos_t pos;

	const ivm_char_t *label;
	ivm_size_t llen;

	const ivm_char_t *opcode;
	ivm_size_t olen;

	ivm_gen_opcode_arg_t arg;
} ivm_gen_instr_t;

#define ivm_gen_instr_build(l, p, ...) \
	((ivm_gen_instr_t) { (ivm_gen_pos_t) { (l), (p) }, __VA_ARGS__ })

typedef ivm_list_t ivm_gen_instr_list_t;
typedef IVM_LIST_ITER_TYPE(ivm_gen_instr_t) ivm_gen_instr_list_iterator_t;

#define ivm_gen_instr_list_new() (ivm_list_new(sizeof(ivm_gen_instr_t)))
#define ivm_gen_instr_list_free ivm_list_free
#define ivm_gen_instr_list_add(list, instr) (ivm_list_push((list), (instr)))

#define IVM_GEN_INSTR_LIST_ITER_SET(iter, val) IVM_LIST_ITER_SET((iter), (val), ivm_gen_instr_t)
#define IVM_GEN_INSTR_LIST_ITER_GET(iter) IVM_LIST_ITER_GET((iter), ivm_gen_instr_t)
#define IVM_GEN_INSTR_LIST_ITER_GET_PTR(iter) IVM_LIST_ITER_GET_PTR((iter), ivm_gen_instr_t)
#define IVM_GEN_INSTR_LIST_EACHPTR(list, iter) IVM_LIST_EACHPTR((list), iter, ivm_gen_instr_t)

typedef struct {
	ivm_gen_pos_t pos;

	const ivm_char_t *label;
	ivm_size_t len;

	ivm_gen_instr_list_t *instrs;
} ivm_gen_block_t;

#define ivm_gen_block_build(l, p, ...) \
	((ivm_gen_block_t) { (ivm_gen_pos_t) { (l), (p) }, __VA_ARGS__ })

typedef ivm_list_t ivm_gen_block_list_t;
typedef IVM_LIST_ITER_TYPE(ivm_gen_block_t) ivm_gen_block_list_iterator_t;

#define ivm_gen_block_list_new() (ivm_list_new(sizeof(ivm_gen_block_t)))
void
ivm_gen_block_list_free(ivm_gen_block_list_t *list);
#define ivm_gen_block_list_push(list, block) (ivm_list_push((list), (block)))

#define IVM_GEN_BLOCK_LIST_ITER_SET(iter, val) IVM_LIST_ITER_SET((iter), (val), ivm_gen_block_t)
#define IVM_GEN_BLOCK_LIST_ITER_GET(iter) IVM_LIST_ITER_GET((iter), ivm_gen_block_t)
#define IVM_GEN_BLOCK_LIST_ITER_GET_PTR(iter) IVM_LIST_ITER_GET_PTR((iter), ivm_gen_block_t)
#define IVM_GEN_BLOCK_LIST_EACHPTR(list, iter) IVM_LIST_EACHPTR((list), iter, ivm_gen_block_t)

typedef struct {
	ivm_gen_pos_t pos;
	ivm_size_t ref_addr;
} ivm_gen_label_ref_t;

typedef ivm_list_t ivm_gen_label_ref_list_t;
typedef IVM_LIST_ITER_TYPE(ivm_gen_label_ref_t) ivm_gen_label_ref_list_iterator_t;

#define ivm_gen_label_ref_list_new() (ivm_list_new(sizeof(ivm_gen_label_ref_t)))
#define ivm_gen_label_ref_list_free ivm_list_free
#define ivm_gen_label_ref_list_add(list, ref) (ivm_list_push((list), (ref)))

#define IVM_GEN_LABEL_REF_LIST_ITER_SET(iter, val) IVM_LIST_ITER_SET((iter), (val), ivm_gen_label_ref_t)
#define IVM_GEN_LABEL_REF_LIST_ITER_GET(iter) IVM_LIST_ITER_GET((iter), ivm_gen_label_ref_t)
#define IVM_GEN_LABEL_REF_LIST_ITER_GET_PTR(iter) IVM_LIST_ITER_GET_PTR((iter), ivm_gen_label_ref_t)
#define IVM_GEN_LABEL_REF_LIST_EACHPTR(list, iter) IVM_LIST_EACHPTR((list), iter, ivm_gen_label_ref_t)

typedef struct {
	ivm_gen_pos_t def_pos;
	ivm_bool_t has_def;

	const ivm_char_t *name;
	ivm_size_t len;
	
	ivm_size_t addr;

	ivm_gen_label_ref_list_t *refs;
} ivm_gen_label_t;

ivm_gen_label_t *
ivm_gen_label_new(const ivm_char_t *name,
				  ivm_size_t len,
				  ivm_bool_t is_def,
				  ivm_gen_pos_t pos,
				  ivm_size_t addr);

void
ivm_gen_label_free(ivm_gen_label_t *label);

typedef ivm_ptlist_t ivm_gen_label_list_t;
typedef IVM_PTLIST_ITER_TYPE(ivm_gen_label_t *) ivm_gen_label_list_iterator_t;

#define ivm_gen_label_list_new() (ivm_ptlist_new())
#define ivm_gen_label_list_free ivm_ptlist_free
#define ivm_gen_label_list_add(list, label) (ivm_ptlist_push((list), (label)))
#define ivm_gen_label_list_empty ivm_ptlist_empty

#define IVM_GEN_LABEL_LIST_ITER_SET(iter, val) (IVM_PTLIST_ITER_SET((iter), (val)))
#define IVM_GEN_LABEL_LIST_ITER_GET(iter) ((ivm_gen_label_t *)IVM_PTLIST_ITER_GET(iter))
#define IVM_GEN_LABEL_LIST_EACHPTR(list, iter) IVM_PTLIST_EACHPTR((list), iter, ivm_gen_label_t *)

typedef struct {
	ivm_string_pool_t *str_pool;
	ivm_gen_block_list_t *block_list;

	/* caches */
	ivm_gen_label_list_t *jmp_table;
} ivm_gen_env_t;

ivm_gen_env_t *
ivm_gen_env_new(ivm_gen_block_list_t *block_list);

void
ivm_gen_env_free(ivm_gen_env_t *env);

ivm_vmstate_t *
ivm_gen_env_generateVM(ivm_gen_env_t *env);

ivm_exec_unit_t *
ivm_gen_env_generateExecUnit(ivm_gen_env_t *env);

IVM_COM_END

#endif
