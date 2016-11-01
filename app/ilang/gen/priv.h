#ifndef _IVM_APP_ILANG_GEN_PRIV_H_
#define _IVM_APP_ILANG_GEN_PRIV_H_

#include <setjmp.h>

#include "pub/type.h"
#include "pub/com.h"
#include "pub/const.h"
#include "pub/vm.h"
#include "pub/err.h"

#include "util/parser.h"

#include "gen.h"

#define FLAG ilang_gen_flag_build
#define CHECK_SE() ((ilang_gen_check_flag_t) { .has_side_effect = IVM_TRUE })
#define RETVAL ilang_gen_value_build
#define NORET() ((ilang_gen_value_t) { 0 })
#define INC_SP() (++env->sp)
#define DEC_SP() (--env->sp)

#define GET_LINE(expr) ((expr)->pos.line)

#define GEN_ERR(p, ...) \
	IVM_TRACE("ilang generator error: at line %ld pos %ld: ", (p).line, (p).pos); \
	IVM_TRACE(__VA_ARGS__); \
	IVM_TRACE("\n"); \
	longjmp(env->err_handle, 1);
	// IVM_EXIT(1);

#define GEN_WARN(p, ...) \
	IVM_TRACE("ilang generator warning: at line %ld pos %ld: ", (p).line, (p).pos); \
	IVM_TRACE(__VA_ARGS__); \
	IVM_TRACE("\n");

#define GEN_ERR_MSG_CANNOT_ASSIGN_TO(expr_name)						"cannot assign to, delete or get the reference of %s", (expr_name)
#define GEN_ERR_MSG_NESTED_RET										"nested return expression"
#define GEN_ERR_MSG_FAILED_PARSE_NUM(val, len)						"failed parse num '%.*s'", (int)(len), (val)
#define GEN_ERR_MSG_UNSUPPORTED_UNARY_OP(type)						"unsupported unary operation type %d", (type)
#define GEN_ERR_MSG_UNSUPPORTED_BINARY_OP(type)						"unsupported binary operation type %d", (type)
#define GEN_ERR_MSG_UNSUPPORTED_CMP_TYPE(type)						"unsupported compare type type %d", (type)
#define GEN_ERR_MSG_BREAK_OR_CONT_OUTSIDE_LOOP						"using break/cont outside a loop"
#define GEN_ERR_MSG_BREAK_OR_CONT_IGNORE_ARG						"ignore break/cont argument"
#define GEN_ERR_MSG_MULTIPLE_VARG									"only one variable argument parameter is allowed in a parameter list or list unpacking"
#define GEN_ERR_MSG_FAILED_PARSE_STRING(msg)						"failed to parse string literal: %s", (msg)
#define GEN_ERR_MSG_DUP_PARAM_NAME(name, len)						"duplicated parameter name '%.*s'", (int)(len), (name)
#define GEN_ERR_MSG_TOO_LONG_MOD_NAME(len)							"module name is too long(length of %ld)", (len)
#define GEN_ERR_MSG_NOT_IN_ARG(name)								"%s outside an argument list", (name)
#define GEN_ERR_MSG_NOT_LEFT_VAL(name)								"a %s has to be a left expression", (name)
#define GEN_ERR_MSG_NOT_IN_LIST(name)								"%s outside a list", (name)

#define GEN_ERR_GENERAL(expr, ...) \
	GEN_ERR((expr)->pos, __VA_ARGS__)

#define GEN_WARN_GENERAL(expr, ...) \
	GEN_WARN((expr)->pos, __VA_ARGS__)

#define GEN_ASSERT_NOT_LEFT_VALUE(expr, name, flag) \
	if ((flag).is_left_val) { \
		GEN_ERR((expr)->pos, GEN_ERR_MSG_CANNOT_ASSIGN_TO(name)); \
	}

#define GEN_ASSERT_NO_NESTED_RET(expr, flag) \
	if (!(flag).is_top_level) { \
		GEN_WARN((expr)->pos, GEN_ERR_MSG_NESTED_RET); \
	}

#define GEN_ASSERT_MISSING_IS_ARG(expr, flag) \
	if (!(flag).is_arg) { \
		GEN_ERR((expr)->pos, GEN_ERR_MSG_MISSING_NOT_IN_ARG); \
	}

#define GEN_ASSERT_ONLY_ARG(expr, flag, name) \
	if (!(flag).is_arg) { \
		GEN_ERR((expr)->pos, GEN_ERR_MSG_NOT_IN_ARG(name)); \
	}

#define GEN_ASSERT_ONLY_LEFT_VAL(expr, flag, name) \
	if (!(flag).is_left_val) { \
		GEN_ERR((expr)->pos, GEN_ERR_MSG_NOT_LEFT_VAL(name)); \
	}

#define GEN_ASSERT_ONLY_LIST(expr, flag, name) \
	if (!(flag).varg_offset) { \
		GEN_ERR((expr)->pos, GEN_ERR_MSG_NOT_IN_LIST(name)); \
	}

#define GEN_ERR_MULTIPLE_VARG(expr) \
	GEN_ERR((expr)->pos, GEN_ERR_MSG_MULTIPLE_VARG);

#define GEN_ERR_FAILED_PARSE_STRING(expr, msg) \
	GEN_ERR((expr)->pos, GEN_ERR_MSG_FAILED_PARSE_STRING(msg));

#define GEN_ERR_DUP_PARAM_NAME(expr, name, len) \
	GEN_ERR((expr)->pos, GEN_ERR_MSG_DUP_PARAM_NAME((name), (len)));

#define GEN_ERR_TOO_LONG_MOD_NAME(expr, len) \
	GEN_ERR((expr)->pos, GEN_ERR_MSG_TOO_LONG_MOD_NAME(len));

#endif
