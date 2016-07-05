#ifndef _IVM_APP_ILANG_GEN_H_
#define _IVM_APP_ILANG_GEN_H_

#include "pub/com.h"
#include "pub/type.h"
#include "pub/vm.h"
#include "pub/mem.h"
#include "pub/err.h"

#include "std/list.h"
#include "std/pool.h"

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
	ivm_exec_unit_t *unit;
} ilang_gen_env_t;

typedef struct {
	ivm_bool_t is_left_val;
	ivm_bool_t is_top_level; // true: don't leave anything on the stack
} ilang_gen_flag_t;

typedef struct {
	ivm_instr_t assign_instr;
} ilang_gen_value_t;

struct ilang_gen_expr_t_tag;

typedef ilang_gen_value_t (*ilang_gen_eval_t)(struct ilang_gen_expr_t_tag *expr,
											  ilang_gen_flag_t flag,
											  ilang_gen_env_t *env);

typedef void (*ilang_gen_destructor_t)(struct ilang_gen_expr_t_tag *expr);

#define ILANG_GEN_EXPR_HEADER \
	ilang_gen_pos_t pos; \
	ilang_gen_eval_t eval; \
	ilang_gen_destructor_t des;

typedef struct ilang_gen_expr_t_tag {
	ILANG_GEN_EXPR_HEADER
} ilang_gen_expr_t;

IVM_INLINE
void
ilang_gen_expr_init(ilang_gen_expr_t *expr,
					ilang_gen_pos_t pos,
					ilang_gen_eval_t eval,
					ilang_gen_destructor_t des)
{
	expr->pos = pos;
	expr->eval = eval;
	expr->des = des;
	return;
}

IVM_INLINE
void
ilang_gen_expr_free(ilang_gen_expr_t *expr)
{
	if (expr) {
		if (expr->des) {
			expr->des(expr);
		}

		MEM_FREE(expr);
	}

	return;
}

typedef ivm_ptlist_t ilang_gen_expr_list_t;
typedef IVM_PTLIST_ITER_TYPE(ilang_gen_expr_t *) ilang_gen_expr_list_iterator_t;

#define ilang_gen_expr_list_new ivm_ptlist_new
#define ilang_gen_expr_list_push ivm_ptlist_push

#define ILANG_GEN_EXPR_LIST_ITER_SET(iter, val) (IVM_PTLIST_ITER_SET((iter), (val)))
#define ILANG_GEN_EXPR_LIST_ITER_GET(iter) ((ilang_gen_expr_t *)IVM_PTLIST_ITER_GET(iter))
#define ILANG_GEN_EXPR_LIST_EACHPTR(list, iter) IVM_PTLIST_EACHPTR((list), iter, ilang_gen_expr_t *)
#define ILANG_GEN_EXPR_LIST_EACHPTR_R(list, iter) IVM_PTLIST_EACHPTR_R((list), iter, ilang_gen_expr_t *)

IVM_INLINE
void
ilang_gen_expr_list_free(ilang_gen_expr_list_t *list)
{
	ilang_gen_expr_list_iterator_t eiter;

	if (list) {
		ILANG_GEN_EXPR_LIST_EACHPTR_R(list, eiter) {
			ilang_gen_expr_free(
				ILANG_GEN_EXPR_LIST_ITER_GET(eiter)
			);
		}

		ivm_ptlist_free(list);
	}

	return;
}

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
ilang_gen_intr_expr_eval(ilang_gen_expr_t *expr,
						 ilang_gen_flag_t flag,
						 ilang_gen_env_t *env);

ilang_gen_value_t
ilang_gen_assign_expr_eval(ilang_gen_expr_t *expr,
						   ilang_gen_flag_t flag,
						   ilang_gen_env_t *env);

void
ilang_gen_expr_block_destruct(ilang_gen_expr_t *expr);

void
ilang_gen_call_expr_destruct(ilang_gen_expr_t *expr);

void
ilang_gen_slot_expr_destruct(ilang_gen_expr_t *expr);

void
ilang_gen_unary_expr_destruct(ilang_gen_expr_t *expr);

void
ilang_gen_binary_expr_destruct(ilang_gen_expr_t *expr);

void
ilang_gen_cmp_expr_destruct(ilang_gen_expr_t *expr);

void
ilang_gen_fn_expr_destruct(ilang_gen_expr_t *expr);

void
ilang_gen_if_expr_destruct(ilang_gen_expr_t *expr);

void
ilang_gen_intr_expr_destruct(ilang_gen_expr_t *expr);

void
ilang_gen_assign_expr_destruct(ilang_gen_expr_t *expr);

#define COMMON_EXPR(name, fname, des, init, ...) \
	IVM_INLINE                                                                       \
	ilang_gen_expr_t *                                                               \
	ilang_gen_##name##_new(ilang_gen_pos_t pos,                                      \
						   __VA_ARGS__)                                              \
	{                                                                                \
		ilang_gen_##name##_t *ret = MEM_ALLOC(sizeof(*ret),                          \
											  ilang_gen_##name##_t *);               \
                                                                                     \
		IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW(fname));                      \
                                                                                     \
		ilang_gen_expr_init(IVM_AS(ret, ilang_gen_expr_t),                           \
							pos, ilang_gen_##name##_eval, (des));                    \
		init;                                                                        \
                                                                                     \
		return IVM_AS(ret, ilang_gen_expr_t);                                        \
	} int dummy()


/* block */
typedef struct {
	ILANG_GEN_EXPR_HEADER
	ilang_gen_expr_list_t *list;
} ilang_gen_expr_block_t;

COMMON_EXPR(expr_block, "expression block", ilang_gen_expr_block_destruct, {
	ret->list = list;
}, ilang_gen_expr_list_t *list);

/* int expr */
typedef struct {
	ILANG_GEN_EXPR_HEADER
	ilang_gen_token_value_t val;
} ilang_gen_int_expr_t;

COMMON_EXPR(int_expr, "integer expression", IVM_NULL, {
	ret->val = val;
}, ilang_gen_token_value_t val);

/* float expr */
typedef struct {
	ILANG_GEN_EXPR_HEADER
	ilang_gen_token_value_t val;
} ilang_gen_float_expr_t;

COMMON_EXPR(float_expr, "float expression", IVM_NULL, {
	ret->val = val;
}, ilang_gen_token_value_t val);

/* string expr */
typedef struct {
	ILANG_GEN_EXPR_HEADER
	ilang_gen_token_value_t val;
} ilang_gen_string_expr_t;

COMMON_EXPR(string_expr, "string expression", IVM_NULL, {
	ret->val = val;
}, ilang_gen_token_value_t val);

/* id expr */
typedef struct {
	ILANG_GEN_EXPR_HEADER
	ilang_gen_token_value_t val;
} ilang_gen_id_expr_t;

COMMON_EXPR(id_expr, "id expression", IVM_NULL, {
	ret->val = val;
}, ilang_gen_token_value_t val);

/* call expr */
typedef struct {
	ILANG_GEN_EXPR_HEADER
	ilang_gen_expr_t *callee;
	ilang_gen_expr_list_t *args;
} ilang_gen_call_expr_t;

COMMON_EXPR(call_expr, "call expression", ilang_gen_call_expr_destruct, {
	ret->callee = callee;
	ret->args = args;
}, ilang_gen_expr_t *callee, ilang_gen_expr_list_t *args);

/* slot expr */
typedef struct {
	ILANG_GEN_EXPR_HEADER
	ilang_gen_expr_t *obj;
	ilang_gen_token_value_t slot;
} ilang_gen_slot_expr_t;

COMMON_EXPR(slot_expr, "slot expression", ilang_gen_slot_expr_destruct, {
	ret->obj = obj;
	ret->slot = slot;
}, ilang_gen_expr_t *obj, ilang_gen_token_value_t slot);

/* unary expr */
typedef struct {
	ILANG_GEN_EXPR_HEADER
	ilang_gen_expr_t *opr;
	ivm_int_t type;
} ilang_gen_unary_expr_t;

COMMON_EXPR(unary_expr, "unary expression", ilang_gen_unary_expr_destruct, {
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

COMMON_EXPR(binary_expr, "binary expression", ilang_gen_binary_expr_destruct, {
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

COMMON_EXPR(cmp_expr, "compare expression", ilang_gen_cmp_expr_destruct, {
	ret->op1 = op1;
	ret->op2 = op2;
	ret->cmp_type = cmp_type;
}, ilang_gen_expr_t *op1, ilang_gen_expr_t *op2, ivm_int_t cmp_type);

typedef ilang_gen_token_value_t ilang_gen_param_t;
typedef ivm_list_t ilang_gen_param_list_t;
typedef IVM_LIST_ITER_TYPE(ilang_gen_param_t) ilang_gen_param_list_iterator_t;

#define ilang_gen_param_list_new() (ivm_list_new(sizeof(ilang_gen_param_t)))
#define ilang_gen_param_list_free ivm_list_free
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

COMMON_EXPR(fn_expr, "function expression", ilang_gen_fn_expr_destruct, {
	ret->params = params;
	ret->body = body;
}, ilang_gen_param_list_t *params, ilang_gen_expr_t *body);

typedef struct {
	ilang_gen_expr_t *cond;
	ilang_gen_expr_t *body;
} ilang_gen_branch_t;

#define ilang_gen_branch_build(cond, body) ((ilang_gen_branch_t) { (cond), (body) })

IVM_INLINE
void
ilang_gen_branch_dump(ilang_gen_branch_t *branch)
{
	if (branch) {
		ilang_gen_expr_free(branch->cond);
		ilang_gen_expr_free(branch->body);
	}

	return;
}

typedef ivm_list_t ilang_gen_branch_list_t;
typedef IVM_LIST_ITER_TYPE(ilang_gen_branch_t) ilang_gen_branch_list_iterator_t;

#define ilang_gen_branch_list_new() (ivm_list_new(sizeof(ilang_gen_branch_t)))
#define ilang_gen_branch_list_push ivm_list_push

#define ILANG_GEN_BRANCH_LIST_ITER_SET(iter, val) IVM_LIST_ITER_SET((iter), (val), ilang_gen_branch_t)
#define ILANG_GEN_BRANCH_LIST_ITER_GET(iter) IVM_LIST_ITER_GET((iter), ilang_gen_branch_t)
#define ILANG_GEN_BRANCH_LIST_ITER_GET_PTR(iter) IVM_LIST_ITER_GET_PTR((iter), ilang_gen_branch_t)
#define ILANG_GEN_BRANCH_LIST_EACHPTR(list, iter) IVM_LIST_EACHPTR((list), iter, ilang_gen_branch_t)
#define ILANG_GEN_BRANCH_LIST_EACHPTR_R(list, iter) IVM_LIST_EACHPTR_R((list), iter, ilang_gen_branch_t)

IVM_INLINE
void
ilang_gen_branch_list_free(ilang_gen_branch_list_t *list)
{
	ilang_gen_branch_list_iterator_t biter;

	if (list) {
		ILANG_GEN_BRANCH_LIST_EACHPTR(list, biter) {
			ilang_gen_branch_dump(
				ILANG_GEN_BRANCH_LIST_ITER_GET_PTR(biter)
			);
		}

		ivm_list_free(list);
	}

	return;
}

typedef struct {
	ILANG_GEN_EXPR_HEADER
	ilang_gen_branch_t main;
	ilang_gen_branch_list_t *elifs;
	ilang_gen_branch_t last;
} ilang_gen_if_expr_t;

COMMON_EXPR(if_expr, "if expression", ilang_gen_if_expr_destruct, {
	ret->main = main;
	ret->elifs = elifs;
	ret->last = last;
}, ilang_gen_branch_t main,
   ilang_gen_branch_list_t *elifs,
   ilang_gen_branch_t last);

enum {
	ILANG_GEN_INTR_RET = 1
};

/* intr expr */
typedef struct {
	ILANG_GEN_EXPR_HEADER
	ivm_int_t sig;
	ilang_gen_expr_t *val;
} ilang_gen_intr_expr_t;

COMMON_EXPR(intr_expr, "intr expression", ilang_gen_intr_expr_destruct, {
	ret->sig = sig;
	ret->val = val;
}, ivm_int_t sig, ilang_gen_expr_t *val);

/* assign expr */
typedef struct {
	ILANG_GEN_EXPR_HEADER
	ilang_gen_expr_t *lhe;
	ilang_gen_expr_t *rhe;
} ilang_gen_assign_expr_t;

COMMON_EXPR(assign_expr, "assign expression", ilang_gen_assign_expr_destruct, {
	ret->lhe = lhe;
	ret->rhe = rhe;
}, ilang_gen_expr_t *lhe, ilang_gen_expr_t *rhe);

#undef COMMON_EXPR

typedef struct {
	ilang_gen_expr_list_t *expr_list;
} ilang_gen_trans_unit_t;

IVM_INLINE
ilang_gen_trans_unit_t *
ilang_gen_trans_unit_new(ilang_gen_expr_list_t *expr_list)
{
	ilang_gen_trans_unit_t *ret = MEM_ALLOC(sizeof(*ret),
											ilang_gen_trans_unit_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("translate unit"));

	ret->expr_list = expr_list;

	return ret;
}

IVM_INLINE
void
ilang_gen_trans_unit_free(ilang_gen_trans_unit_t *unit)
{
	if (unit) {
		ilang_gen_expr_list_free(unit->expr_list);
		MEM_FREE(unit);
	}

	return;
}

IVM_COM_END

#endif
