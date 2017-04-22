#ifndef _IVM_APP_IAS_GEN_H_
#define _IVM_APP_IAS_GEN_H_

#include "pub/com.h"
#include "pub/type.h"
#include "pub/vm.h"

#include "std/list.h"
#include "std/pool.h"

IVM_COM_HEADER

typedef struct {
	ivm_size_t line;
	ivm_size_t pos;
} ias_gen_pos_t;

typedef struct {
	ias_gen_pos_t pos;

	ivm_char_t type;

	const ivm_char_t *val;
	ivm_size_t len;
} ias_gen_opcode_arg_t;

#define ias_gen_opcode_arg_build(l, p, ...) \
	((ias_gen_opcode_arg_t) { (ias_gen_pos_t) { (l), (p) }, __VA_ARGS__ })

typedef struct {
	ias_gen_pos_t pos;

	const ivm_char_t *label;
	ivm_size_t llen;

	const ivm_char_t *opcode;
	ivm_size_t olen;

	ias_gen_opcode_arg_t arg;
} ias_gen_instr_t;

#define ias_gen_instr_build(l, p, ...) \
	((ias_gen_instr_t) { (ias_gen_pos_t) { (l), (p) }, __VA_ARGS__ })

typedef ivm_list_t ias_gen_instr_list_t;
typedef IVM_LIST_ITER_TYPE(ias_gen_instr_t) ias_gen_instr_list_iterator_t;

#define ias_gen_instr_list_new() (ivm_list_new(sizeof(ias_gen_instr_t)))
#define ias_gen_instr_list_free ivm_list_free
#define ias_gen_instr_list_add(list, instr) (ivm_list_push((list), (instr)))

#define IAS_GEN_INSTR_LIST_ITER_SET(iter, val) IVM_LIST_ITER_SET((iter), (val), ias_gen_instr_t)
#define IAS_GEN_INSTR_LIST_ITER_GET(iter) IVM_LIST_ITER_GET((iter), ias_gen_instr_t)
#define IAS_GEN_INSTR_LIST_ITER_GET_PTR(iter) IVM_LIST_ITER_GET_PTR((iter), ias_gen_instr_t)
#define IAS_GEN_INSTR_LIST_EACHPTR(list, iter) IVM_LIST_EACHPTR((list), iter, ias_gen_instr_t)

typedef struct {
	ias_gen_pos_t pos;

	const ivm_char_t *label;
	ivm_size_t len;

	ias_gen_instr_list_t *instrs;
} ias_gen_block_t;

#define ias_gen_block_build(l, p, ...) \
	((ias_gen_block_t) { (ias_gen_pos_t) { (l), (p) }, __VA_ARGS__ })

typedef ivm_list_t ias_gen_block_list_t;
typedef IVM_LIST_ITER_TYPE(ias_gen_block_t) ias_gen_block_list_iterator_t;

#define ias_gen_block_list_new() (ivm_list_new(sizeof(ias_gen_block_t)))
void
ias_gen_block_list_free(ias_gen_block_list_t *list);
#define ias_gen_block_list_push(list, block) (ivm_list_push((list), (block)))

#define IAS_GEN_BLOCK_LIST_ITER_SET(iter, val) IVM_LIST_ITER_SET((iter), (val), ias_gen_block_t)
#define IAS_GEN_BLOCK_LIST_ITER_GET(iter) IVM_LIST_ITER_GET((iter), ias_gen_block_t)
#define IAS_GEN_BLOCK_LIST_ITER_GET_PTR(iter) IVM_LIST_ITER_GET_PTR((iter), ias_gen_block_t)
#define IAS_GEN_BLOCK_LIST_EACHPTR(list, iter) IVM_LIST_EACHPTR((list), iter, ias_gen_block_t)

typedef struct {
	ias_gen_pos_t pos;
	ivm_size_t ref_addr;
} ias_gen_label_ref_t;

typedef ivm_list_t ias_gen_label_ref_list_t;
typedef IVM_LIST_ITER_TYPE(ias_gen_label_ref_t) ias_gen_label_ref_list_iterator_t;

#define ias_gen_label_ref_list_new() (ivm_list_new(sizeof(ias_gen_label_ref_t)))
#define ias_gen_label_ref_list_free ivm_list_free
#define ias_gen_label_ref_list_add(list, ref) (ivm_list_push((list), (ref)))

#define IAS_GEN_LABEL_REF_LIST_ITER_SET(iter, val) IVM_LIST_ITER_SET((iter), (val), ias_gen_label_ref_t)
#define IAS_GEN_LABEL_REF_LIST_ITER_GET(iter) IVM_LIST_ITER_GET((iter), ias_gen_label_ref_t)
#define IAS_GEN_LABEL_REF_LIST_ITER_GET_PTR(iter) IVM_LIST_ITER_GET_PTR((iter), ias_gen_label_ref_t)
#define IAS_GEN_LABEL_REF_LIST_EACHPTR(list, iter) IVM_LIST_EACHPTR((list), iter, ias_gen_label_ref_t)

typedef struct {
	ias_gen_pos_t def_pos;
	ivm_bool_t has_def;

	const ivm_char_t *name;
	ivm_size_t len;
	
	ivm_size_t addr;

	ias_gen_label_ref_list_t *refs;
} ias_gen_label_t;

ias_gen_label_t *
ias_gen_label_new(const ivm_char_t *name,
				  ivm_size_t len,
				  ivm_bool_t is_def,
				  ias_gen_pos_t pos,
				  ivm_size_t addr);

void
ias_gen_label_free(ias_gen_label_t *label);

typedef ivm_ptlist_t ias_gen_label_list_t;
typedef IVM_PTLIST_ITER_TYPE(ias_gen_label_t *) ias_gen_label_list_iterator_t;

#define ias_gen_label_list_new() (ivm_ptlist_new())
#define ias_gen_label_list_free ivm_ptlist_free
#define ias_gen_label_list_add(list, label) (ivm_ptlist_push((list), (label)))
#define ias_gen_label_list_empty ivm_ptlist_empty

#define IAS_GEN_LABEL_LIST_ITER_SET(iter, val) (IVM_PTLIST_ITER_SET((iter), (val)))
#define IAS_GEN_LABEL_LIST_ITER_GET(iter) ((ias_gen_label_t *)IVM_PTLIST_ITER_GET(iter))
#define IAS_GEN_LABEL_LIST_EACHPTR(list, iter) IVM_PTLIST_EACHPTR((list), iter, ias_gen_label_t *)

typedef struct {
	ivm_string_pool_t *str_pool;
	ias_gen_block_list_t *block_list;

	/* caches */
	ias_gen_label_list_t *jmp_table;
} ias_gen_env_t;

ias_gen_env_t *
ias_gen_env_new(ias_gen_block_list_t *block_list);

void
ias_gen_env_free(ias_gen_env_t *env);

ivm_vmstate_t *
ias_gen_env_generateVM(ias_gen_env_t *env);

ivm_exec_unit_t *
ias_gen_env_generateExecUnit(ias_gen_env_t *env);

IVM_COM_END

#endif
