#include "pub/type.h"
#include "pub/const.h"
#include "pub/inlines.h"
#include "pub/vm.h"

#include "std/string.h"
#include "std/enc.h"

#include "vm/func.h"
#include "vm/num.h"

#include "priv.h"
#include "nfunc.h"

#if 0

IVM_NATIVE_FUNC(_op_wrapper_call)
{
	ivm_op_wrapper_t *wrapper;
	ivm_binop_proc_t bin_proc;
	ivm_uniop_proc_t uni_proc;
	ivm_object_t *op2, *base, *ret;
	ivm_int_t op;

	CHECK_BASE(IVM_OP_WRAPPER_T);

	wrapper = IVM_AS(NAT_BASE(), ivm_op_wrapper_t);
	base = ivm_op_wrapper_getBase(wrapper);
	op = ivm_op_wrapper_getOp(wrapper);

	if (!base) return IVM_NONE(NAT_STATE());

	if (ivm_op_wrapper_isBinop(wrapper)) {
		MATCH_ARG(".", &op2);

		bin_proc = IVM_OBJECT_GET_BINOP_PROC_R(base, op, op2);

		RTM_ASSERT(bin_proc, IVM_ERROR_MSG_NO_BINOP_FOR(
			IVM_OBJECT_GET(base, TYPE_NAME),
			"<wrapped op>",
			IVM_OBJECT_GET(op2, TYPE_NAME)
		));

		ret = bin_proc(NAT_STATE(), NAT_CORO(), base, op2, IVM_NULL);

		if (ivm_op_wrapper_isCmp(wrapper)) {
			if (ivm_vmstate_getException(NAT_STATE())) { // exception
				return IVM_NULL;
			}

			return ivm_numeric_new(NAT_STATE(), (ivm_ptr_t)ret);
		}

		return ret; // exception passed through
	} else {
		uni_proc = IVM_OBJECT_GET_UNIOP_PROC_R(base, op);

		RTM_ASSERT(uni_proc, IVM_ERROR_MSG_NO_UNIOP_FOR("<wrapped op>", IVM_OBJECT_GET(base, TYPE_NAME)));

		// exception passed through
		return uni_proc(NAT_STATE(), NAT_CORO(), base);
	}

	return IVM_NULL;
}

#endif
