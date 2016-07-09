#ifndef _IVM_APP_ILANG_GEN_H_
#define _IVM_APP_ILANG_GEN_H_

#include "pub/com.h"
#include "pub/const.h"
#include "pub/type.h"
#include "pub/vm.h"
#include "pub/err.h"

#include "std/list.h"
#include "std/heap.h"

IVM_COM_HEADER

typedef struct {
	ivm_size_t line;
	ivm_size_t pos;
} ilang_gen_pos_t;

#define ilang_gen_pos_build(line, pos) ((ilang_gen_pos_t) { (line), (pos) })

typedef struct {
	const ivm_char_t *val;
	ivm_size_t len;
} ilang_gen_token_value_t;

#define ilang_gen_token_value_build(val, len) ((ilang_gen_token_value_t) { (val), (len) })

typedef struct {
	ivm_string_pool_t *str_pool;
	ivm_exec_unit_t *unit;
	ivm_exec_t *cur_exec;
	ivm_size_t continue_addr;
	ivm_list_t *break_ref;
} ilang_gen_env_t;

typedef struct {
	ivm_bool_t is_left_val;
	ivm_bool_t is_top_level; // true: don't leave anything on the stack
	ivm_bool_t if_use_cond_reg; // use (virtual)register to in condition expression
	ivm_bool_t is_callee; // return base if possible
} ilang_gen_flag_t;

#define ilang_gen_flag_build(...) ((ilang_gen_flag_t) { __VA_ARGS__ })

typedef struct {
	ivm_bool_t use_cond_reg; // return value to confirm the use of register
	ivm_bool_t has_base; // has base object on the stack under top(second object)
} ilang_gen_value_t;

#define ilang_gen_value_build(...) ((ilang_gen_value_t) { __VA_ARGS__ })

struct ilang_gen_expr_t_tag;

typedef ilang_gen_value_t (*ilang_gen_eval_t)(struct ilang_gen_expr_t_tag *expr,
											  ilang_gen_flag_t flag,
											  ilang_gen_env_t *env);

typedef struct {
	ivm_bool_t has_side_effect;
} ilang_gen_check_flag_t;

typedef ivm_bool_t (*ilang_gen_checker_t)(struct ilang_gen_expr_t_tag *expr, ilang_gen_check_flag_t flag);

#define ILANG_GEN_EXPR_HEADER \
	ilang_gen_pos_t pos; \
	ilang_gen_eval_t eval; \
	ilang_gen_checker_t check;

typedef struct ilang_gen_expr_t_tag {
	ILANG_GEN_EXPR_HEADER
} ilang_gen_expr_t;

IVM_INLINE
void
ilang_gen_expr_init(ilang_gen_expr_t *expr,
					ilang_gen_pos_t pos,
					ilang_gen_eval_t eval,
					ilang_gen_checker_t check)
{
	expr->pos = pos;
	expr->eval = eval;
	expr->check = check;
	return;
}

typedef ivm_ptlist_t ilang_gen_expr_list_t;
typedef IVM_PTLIST_ITER_TYPE(ilang_gen_expr_t *) ilang_gen_expr_list_iterator_t;

typedef struct {
	ivm_heap_t *heap;
	ilang_gen_expr_list_t *expr_log;
	ivm_ptlist_t *ptlist_log;
	ivm_ptlist_t *list_log;
	ilang_gen_expr_t *top_level;
} ilang_gen_trans_unit_t;

IVM_INLINE
ilang_gen_expr_list_t *
ilang_gen_expr_list_new(ilang_gen_trans_unit_t *unit)
{
	ivm_ptlist_t *list = ivm_ptlist_new();

	ivm_ptlist_push(unit->ptlist_log, list);

	return list;
}

#define ilang_gen_expr_list_push ivm_ptlist_push
#define ilang_gen_expr_list_size ivm_ptlist_size

#define ILANG_GEN_EXPR_LIST_ITER_SET(iter, val) (IVM_PTLIST_ITER_SET((iter), (val)))
#define ILANG_GEN_EXPR_LIST_ITER_GET(iter) ((ilang_gen_expr_t *)IVM_PTLIST_ITER_GET(iter))
#define ILANG_GEN_EXPR_LIST_ITER_IS_FIRST IVM_PTLIST_ITER_IS_FIRST
#define ILANG_GEN_EXPR_LIST_EACHPTR(list, iter) IVM_PTLIST_EACHPTR((list), iter, ilang_gen_expr_t *)
#define ILANG_GEN_EXPR_LIST_EACHPTR_R(list, iter) IVM_PTLIST_EACHPTR_R((list), iter, ilang_gen_expr_t *)

ilang_gen_value_t
ilang_gen_expr_block_eval(ilang_gen_expr_t *expr,
						  ilang_gen_flag_t flag,
						  ilang_gen_env_t *env);

ilang_gen_value_t
ilang_gen_int_expr_eval(ilang_gen_expr_t *expr,
						ilang_gen_flag_t flag,
						ilang_gen_env_t *env);

ilang_gen_value_t
ilang_gen_float_expr_eval(ilang_gen_expr_t *expr,
						  ilang_gen_flag_t flag,
						  ilang_gen_env_t *env);

ilang_gen_value_t
ilang_gen_string_expr_eval(ilang_gen_expr_t *expr,
						   ilang_gen_flag_t flag,
						   ilang_gen_env_t *env);

ilang_gen_value_t
ilang_gen_id_expr_eval(ilang_gen_expr_t *expr,
					   ilang_gen_flag_t flag,
					   ilang_gen_env_t *env);

ilang_gen_value_t
ilang_gen_table_expr_eval(ilang_gen_expr_t *expr,
						  ilang_gen_flag_t flag,
						  ilang_gen_env_t *env);

ilang_gen_value_t
ilang_gen_call_expr_eval(ilang_gen_expr_t *expr,
						 ilang_gen_flag_t flag,
						 ilang_gen_env_t *env);

ilang_gen_value_t
ilang_gen_slot_expr_eval(ilang_gen_expr_t *expr,
						 ilang_gen_flag_t flag,
						 ilang_gen_env_t *env);

ilang_gen_value_t
ilang_gen_unary_expr_eval(ilang_gen_expr_t *expr,
						  ilang_gen_flag_t flag,
						  ilang_gen_env_t *env);

ilang_gen_value_t
ilang_gen_binary_expr_eval(ilang_gen_expr_t *expr,
						   ilang_gen_flag_t flag,
						   ilang_gen_env_t *env);

ilang_gen_value_t
ilang_gen_cmp_expr_eval(ilang_gen_expr_t *expr,
						ilang_gen_flag_t flag,
						ilang_gen_env_t *env);

ilang_gen_value_t
ilang_gen_fn_expr_eval(ilang_gen_expr_t *expr,
					   ilang_gen_flag_t flag,
					   ilang_gen_env_t *env);

ilang_gen_value_t
ilang_gen_if_expr_eval(ilang_gen_expr_t *expr,
					   ilang_gen_flag_t flag,
					   ilang_gen_env_t *env);

ilang_gen_value_t
ilang_gen_while_expr_eval(ilang_gen_expr_t *expr,
						  ilang_gen_flag_t flag,
						  ilang_gen_env_t *env);

ilang_gen_value_t
ilang_gen_intr_expr_eval(ilang_gen_expr_t *expr,
						 ilang_gen_flag_t flag,
						 ilang_gen_env_t *env);

ilang_gen_value_t
ilang_gen_assign_expr_eval(ilang_gen_expr_t *expr,
						   ilang_gen_flag_t flag,
						   ilang_gen_env_t *env);

#define COMMON_EXPR(name, fname, init, ...) \
	ivm_bool_t                                                                       \
	ilang_gen_##name##_check(struct ilang_gen_expr_t_tag *expr,                      \
							 ilang_gen_check_flag_t flag);                           \
	IVM_INLINE                                                                       \
	ilang_gen_expr_t *                                                               \
	ilang_gen_##name##_new(ilang_gen_trans_unit_t *trans_unit,                       \
						   ilang_gen_pos_t pos,                                      \
						   __VA_ARGS__)                                              \
	{                                                                                \
		ilang_gen_##name##_t *ret = ivm_heap_alloc(trans_unit->heap, sizeof(*ret));  \
                                                                                     \
		IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW(fname));                      \
		ilang_gen_expr_list_push(trans_unit->expr_log, ret);                         \
                                                                                     \
		ilang_gen_expr_init(IVM_AS(ret, ilang_gen_expr_t),                           \
							pos, ilang_gen_##name##_eval,                            \
							ilang_gen_##name##_check);                               \
		init;                                                                        \
                                                                                     \
		return IVM_AS(ret, ilang_gen_expr_t);                                        \
	} int dummy()

/* block */
typedef struct {
	ILANG_GEN_EXPR_HEADER
	ilang_gen_expr_list_t *list;
} ilang_gen_expr_block_t;

COMMON_EXPR(expr_block, "expression block", {
	ret->list = list;
}, ilang_gen_expr_list_t *list);

/* int expr */
typedef struct {
	ILANG_GEN_EXPR_HEADER
	ilang_gen_token_value_t val;
} ilang_gen_int_expr_t;

COMMON_EXPR(int_expr, "integer expression", {
	ret->val = val;
}, ilang_gen_token_value_t val);

/* float expr */
typedef struct {
	ILANG_GEN_EXPR_HEADER
	ilang_gen_token_value_t val;
} ilang_gen_float_expr_t;

COMMON_EXPR(float_expr, "float expression", {
	ret->val = val;
}, ilang_gen_token_value_t val);

/* string expr */
typedef struct {
	ILANG_GEN_EXPR_HEADER
	ilang_gen_token_value_t val;
} ilang_gen_string_expr_t;

COMMON_EXPR(string_expr, "string expression", {
	ret->val = val;
}, ilang_gen_token_value_t val);

/* id expr */
typedef struct {
	ILANG_GEN_EXPR_HEADER
	ilang_gen_token_value_t val;
} ilang_gen_id_expr_t;

COMMON_EXPR(id_expr, "id expression", {
	ret->val = val;
}, ilang_gen_token_value_t val);

/* table expr */
typedef struct {
	ilang_gen_pos_t pos;
	ilang_gen_token_value_t name;
	ilang_gen_expr_t *expr;
} ilang_gen_table_entry_t;

#define ilang_gen_table_entry_build(pos, name, expr) \
	((ilang_gen_table_entry_t) { (pos), (name), (expr) })

typedef ivm_list_t ilang_gen_table_entry_list_t;
typedef IVM_LIST_ITER_TYPE(ilang_gen_table_entry_t) ilang_gen_table_entry_list_iterator_t;

IVM_INLINE
ilang_gen_table_entry_list_t *
ilang_gen_table_entry_list_new(ilang_gen_trans_unit_t *unit)
{
	ivm_list_t *list = ivm_list_new(sizeof(ilang_gen_table_entry_t));
	
	ivm_ptlist_push(unit->list_log, list);

	return list;
}

#define ilang_gen_table_entry_list_push ivm_list_push
#define ilang_gen_table_entry_list_size ivm_list_size

#define ILANG_GEN_TABLE_ENTRY_LIST_ITER_SET(iter, val) IVM_LIST_ITER_SET((iter), (val), ilang_gen_table_entry_t)
#define ILANG_GEN_TABLE_ENTRY_LIST_ITER_GET(iter) IVM_LIST_ITER_GET((iter), ilang_gen_table_entry_t)
#define ILANG_GEN_TABLE_ENTRY_LIST_ITER_GET_PTR(iter) IVM_LIST_ITER_GET_PTR((iter), ilang_gen_table_entry_t)
#define ILANG_GEN_TABLE_ENTRY_LIST_EACHPTR(list, iter) IVM_LIST_EACHPTR((list), iter, ilang_gen_table_entry_t)
#define ILANG_GEN_TABLE_ENTRY_LIST_EACHPTR_R(list, iter) IVM_LIST_EACHPTR_R((list), iter, ilang_gen_table_entry_t)

typedef struct {
	ILANG_GEN_EXPR_HEADER
	ilang_gen_table_entry_list_t *list;
} ilang_gen_table_expr_t;

COMMON_EXPR(table_expr, "table expression", {
	ret->list = list;
}, ilang_gen_table_entry_list_t *list);

/* call expr */
typedef struct {
	ILANG_GEN_EXPR_HEADER
	ilang_gen_expr_t *callee;
	ilang_gen_expr_list_t *args;
} ilang_gen_call_expr_t;

COMMON_EXPR(call_expr, "call expression", {
	ret->callee = callee;
	ret->args = args;
}, ilang_gen_expr_t *callee, ilang_gen_expr_list_t *args);

/* slot expr */
typedef struct {
	ILANG_GEN_EXPR_HEADER
	ilang_gen_expr_t *obj;
	ilang_gen_token_value_t slot;
} ilang_gen_slot_expr_t;

COMMON_EXPR(slot_expr, "slot expression", {
	ret->obj = obj;
	ret->slot = slot;
}, ilang_gen_expr_t *obj, ilang_gen_token_value_t slot);

/* unary expr */
typedef struct {
	ILANG_GEN_EXPR_HEADER
	ilang_gen_expr_t *opr;
	ivm_int_t type;
} ilang_gen_unary_expr_t;

COMMON_EXPR(unary_expr, "unary expression", {
	ret->opr = opr;
	ret->type = type;
}, ilang_gen_expr_t *opr, ivm_int_t type);

/* binary expr */
typedef struct {
	ILANG_GEN_EXPR_HEADER
	ilang_gen_expr_t *op1;
	ilang_gen_expr_t *op2;
	ivm_int_t type;
} ilang_gen_binary_expr_t;

COMMON_EXPR(binary_expr, "binary expression", {
	ret->op1 = op1;
	ret->op2 = op2;
	ret->type = type;
}, ilang_gen_expr_t *op1, ilang_gen_expr_t *op2, ivm_int_t type);

/* cmp expr */
enum {
	ILANG_GEN_CMP_LT = -2,
	ILANG_GEN_CMP_LE = -1,
	ILANG_GEN_CMP_EQ = 0,
	ILANG_GEN_CMP_GE = 1,
	ILANG_GEN_CMP_GT = 2,
	ILANG_GEN_CMP_NE = 3,
};

typedef struct {
	ILANG_GEN_EXPR_HEADER
	ilang_gen_expr_t *op1;
	ilang_gen_expr_t *op2;
	ivm_int_t cmp_type; /* -2: lt, -1: le, 0: eq, 1: ge, 2: gt, 3: ne */
} ilang_gen_cmp_expr_t;

COMMON_EXPR(cmp_expr, "compare expression", {
	ret->op1 = op1;
	ret->op2 = op2;
	ret->cmp_type = cmp_type;
}, ilang_gen_expr_t *op1, ilang_gen_expr_t *op2, ivm_int_t cmp_type);

typedef ilang_gen_token_value_t ilang_gen_param_t;
typedef ivm_list_t ilang_gen_param_list_t;
typedef IVM_LIST_ITER_TYPE(ilang_gen_param_t) ilang_gen_param_list_iterator_t;

IVM_INLINE
ilang_gen_param_list_t *
ilang_gen_param_list_new(ilang_gen_trans_unit_t *unit)
{
	ivm_list_t *list = ivm_list_new(sizeof(ilang_gen_param_t));
	
	ivm_ptlist_push(unit->list_log, list);

	return list;
}

#define ilang_gen_param_list_push ivm_list_push

#define ILANG_GEN_PARAM_LIST_ITER_SET(iter, val) IVM_LIST_ITER_SET((iter), (val), ilang_gen_param_t)
#define ILANG_GEN_PARAM_LIST_ITER_GET(iter) IVM_LIST_ITER_GET((iter), ilang_gen_param_t)
#define ILANG_GEN_PARAM_LIST_ITER_GET_PTR(iter) IVM_LIST_ITER_GET_PTR((iter), ilang_gen_param_t)
#define ILANG_GEN_PARAM_LIST_EACHPTR_R(list, iter) IVM_LIST_EACHPTR_R((list), iter, ilang_gen_param_t)

typedef struct {
	ILANG_GEN_EXPR_HEADER
	ilang_gen_param_list_t *params;
	ilang_gen_expr_t *body;
} ilang_gen_fn_expr_t;

COMMON_EXPR(fn_expr, "function expression", {
	ret->params = params;
	ret->body = body;
}, ilang_gen_param_list_t *params, ilang_gen_expr_t *body);

typedef struct {
	ilang_gen_expr_t *cond;
	ilang_gen_expr_t *body;
} ilang_gen_branch_t;

#define ilang_gen_branch_build(cond, body) ((ilang_gen_branch_t) { (cond), (body) })

typedef ivm_list_t ilang_gen_branch_list_t;
typedef IVM_LIST_ITER_TYPE(ilang_gen_branch_t) ilang_gen_branch_list_iterator_t;

IVM_INLINE
ilang_gen_branch_list_t *
ilang_gen_branch_list_new(ilang_gen_trans_unit_t *unit)
{
	ivm_list_t *list = ivm_list_new(sizeof(ilang_gen_branch_t));

	ivm_ptlist_push(unit->list_log, list);

	return list;
}

#define ilang_gen_branch_list_push ivm_list_push
#define ilang_gen_branch_list_size ivm_list_size

#define ILANG_GEN_BRANCH_LIST_ITER_SET(iter, val) IVM_LIST_ITER_SET((iter), (val), ilang_gen_branch_t)
#define ILANG_GEN_BRANCH_LIST_ITER_GET(iter) IVM_LIST_ITER_GET((iter), ilang_gen_branch_t)
#define ILANG_GEN_BRANCH_LIST_ITER_GET_PTR(iter) IVM_LIST_ITER_GET_PTR((iter), ilang_gen_branch_t)
#define ILANG_GEN_BRANCH_LIST_EACHPTR(list, iter) IVM_LIST_EACHPTR((list), iter, ilang_gen_branch_t)
#define ILANG_GEN_BRANCH_LIST_EACHPTR_R(list, iter) IVM_LIST_EACHPTR_R((list), iter, ilang_gen_branch_t)

typedef struct {
	ILANG_GEN_EXPR_HEADER
	ilang_gen_branch_t main;
	ilang_gen_branch_list_t *elifs;
	ilang_gen_branch_t last;
} ilang_gen_if_expr_t;

COMMON_EXPR(if_expr, "if expression", {
	ret->main = main;
	ret->elifs = elifs;
	ret->last = last;
}, ilang_gen_branch_t main,
   ilang_gen_branch_list_t *elifs,
   ilang_gen_branch_t last);

typedef struct {
	ILANG_GEN_EXPR_HEADER
	ilang_gen_expr_t *cond;
	ilang_gen_expr_t *body;
} ilang_gen_while_expr_t;

COMMON_EXPR(while_expr, "while expression", {
	ret->cond = cond;
	ret->body = body;
}, ilang_gen_expr_t *cond,
   ilang_gen_expr_t *body);

enum {
	ILANG_GEN_INTR_RET = 1,
	ILANG_GEN_INTR_CONT,
	ILANG_GEN_INTR_BREAK
};

/* intr expr */
typedef struct {
	ILANG_GEN_EXPR_HEADER
	ivm_int_t sig;
	ilang_gen_expr_t *val;
} ilang_gen_intr_expr_t;

COMMON_EXPR(intr_expr, "intr expression", {
	ret->sig = sig;
	ret->val = val;
}, ivm_int_t sig, ilang_gen_expr_t *val);

/* assign expr */
typedef struct {
	ILANG_GEN_EXPR_HEADER
	ilang_gen_expr_t *lhe;
	ilang_gen_expr_t *rhe;
} ilang_gen_assign_expr_t;

COMMON_EXPR(assign_expr, "assign expression", {
	ret->lhe = lhe;
	ret->rhe = rhe;
}, ilang_gen_expr_t *lhe, ilang_gen_expr_t *rhe);

#undef COMMON_EXPR

IVM_INLINE
ilang_gen_trans_unit_t *
ilang_gen_trans_unit_new()
{
	ilang_gen_trans_unit_t *ret = MEM_ALLOC(sizeof(*ret),
											ilang_gen_trans_unit_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("translate unit"));

	ret->heap = ivm_heap_new(IVM_DEFAULT_PARSER_INIT_HEAP_SIZE);
	ret->ptlist_log = ivm_ptlist_new();
	ret->list_log = ivm_ptlist_new();
	ret->expr_log = ilang_gen_expr_list_new(ret);
	ret->top_level = IVM_NULL;

	return ret;
}

IVM_INLINE
void
ilang_gen_trans_unit_setTopLevel(ilang_gen_trans_unit_t *unit,
								 ilang_gen_expr_list_t *expr_list)
{
	unit->top_level = ilang_gen_expr_block_new(
		unit, ilang_gen_pos_build(0, 0), expr_list
	);

	return;
}

IVM_INLINE
void
ilang_gen_trans_unit_free(ilang_gen_trans_unit_t *unit)
{
	IVM_PTLIST_ITER_TYPE(ivm_ptlist_t *) piter;
	IVM_PTLIST_ITER_TYPE(ivm_list_t *) liter;

	if (unit) {
		ivm_heap_free(unit->heap);

		IVM_PTLIST_EACHPTR(unit->ptlist_log, piter, ivm_ptlist_t *) {
			ivm_ptlist_free(IVM_PTLIST_ITER_GET(piter));
		}

		IVM_PTLIST_EACHPTR(unit->list_log, liter, ivm_list_t *) {
			ivm_list_free(IVM_PTLIST_ITER_GET(liter));
		}

		ivm_ptlist_free(unit->ptlist_log);
		ivm_ptlist_free(unit->list_log);

		MEM_FREE(unit);
	}

	return;
}

ivm_exec_unit_t *
ilang_gen_generateExecUnit(ilang_gen_trans_unit_t *unit);

IVM_COM_END

#endif
