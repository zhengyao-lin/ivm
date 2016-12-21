#include "pub/type.h"
#include "pub/const.h"
#include "pub/inlines.h"
#include "pub/vm.h"
#include "pub/obj.h"

#include "std/io.h"

#include "vm/mod/mod.h"

#include "priv.h"
#include "glob.h"

IVM_NATIVE_FUNC(_global_import)
{
	const ivm_string_t *mod;

	MATCH_ARG("s", &mod);

	return ivm_mod_load(mod, NAT_STATE(), NAT_CORO(), NAT_CONTEXT());
}

IVM_NATIVE_FUNC(_global_typename)
{
	ivm_object_t *obj;

	MATCH_ARG(".", &obj);

	return ivm_string_object_new_r(NAT_STATE(), IVM_OBJECT_GET(obj, TYPE_NAME));
}

IVM_NATIVE_FUNC(_global_is)
{
	ivm_object_t *op1, *op2, *proto, *cur;

	MATCH_ARG("..", &op1, &op2);

	// 1. op1 == op2 == none
	// 2. op1.proto == op2.proto

	if (IVM_IS_NONE(NAT_STATE(), op2) && IVM_IS_NONE(NAT_STATE(), op1)) {
		return ivm_numeric_new(NAT_STATE(), IVM_TRUE);
	}

	proto = ivm_object_getProto(op2);
	cur = op1;
	
	while ((cur = ivm_object_getProto(cur))
		   != IVM_NULL) {
		if (cur == proto) return ivm_numeric_new(NAT_STATE(), IVM_TRUE);
	}

	return ivm_numeric_new(NAT_STATE(), IVM_FALSE);
}

IVM_NATIVE_FUNC(_global_input)
{
	const ivm_string_t *msg = IVM_NULL;
	ivm_char_t *buf = IVM_NULL;
	ivm_size_t cur = 0, i;
	ivm_object_t *ret;

	MATCH_ARG("*s", &msg);

	if (msg) {
		ivm_fputs(IVM_STDOUT, ivm_string_trimHead(msg));
	}

#define _BUF_SIZE 1024

	do {
		buf = STD_REALLOC(buf, sizeof(*buf) * (cur + _BUF_SIZE + 1));
		STD_INIT(buf + cur, sizeof(*buf) * (_BUF_SIZE + 1));
		
		if (!ivm_fgets(IVM_STDIN, buf + cur, sizeof(*buf) * (_BUF_SIZE + 1))) {
			STD_FREE(buf);
			IVM_FATAL(IVM_ERROR_MSG_INPUT_READ_EOF);
		}

		cur += _BUF_SIZE;
		// IVM_TRACE("%s: %d %d\n", buf, IVM_STRLEN(buf), cur);
	} while (buf[cur - 1] != '\0' && buf[cur - 1] != '\n');

#undef _BUF_SIZE

	// FIX: possible false-positive invalid read reported by Valgrind due to use of SEE instr compiled in gcc >= 4.6?
	i = IVM_STRLEN(buf);
	if (i) {
		if (i > 1 && buf[i - 2] == '\r') {
			buf[i - 2] = '\0';
		} else {
			buf[i - 1] = '\0';
		}
	}

	ret = ivm_string_object_new_r(NAT_STATE(), buf);
	STD_FREE(buf);

	return  ret;
}

BUILTIN_FUNC(_global_map, {
	I(CHECK_E, 2)
	I(NEW_LIST, 0)

	I(PUSH_BLOCK)
	I(DUP_PREV_BLOCK_N, 2)
	I(GET_SLOT_N, "iter")
	I(INVOKE_BASE, 0)
	
	I(ITER_NEXT, 8)
	I(INVOKE_BASE, 0)
	I(RPROT_CAC)

	I(DUP_PREV_BLOCK_N, 1)
	I(INVOKE, 1)

	I(DUP_PREV_BLOCK_N, 0)
    I(PUSH_LIST)

    I(JUMP, -7)
    I(POP_BLOCK)

    I(RETURN)
})

IVM_NATIVE_FUNC(_global_print)
{
	ivm_object_t *obj, *to_s, *base, *ret;
	ivm_function_object_t *func;

	CHECK_ARG_COUNT(1);

	obj = NAT_ARG_AT(1);
	to_s = ivm_object_getSlot(obj, NAT_STATE(), IVM_VMSTATE_CONST(NAT_STATE(), C_TO_S));
			
	if (to_s && (func = ivm_object_callable(to_s, NAT_STATE(), &base))) {
		ret = ivm_coro_callBase_0(NAT_CORO(), NAT_STATE(), func, base ? base : obj);

		if (!ret) return IVM_NULL;

		if (IVM_IS_BTTYPE(ret, NAT_STATE(), IVM_STRING_OBJECT_T)) {
			IVM_TRACE("%s\n", ivm_string_trimHead(ivm_string_object_getValue(ret)));
			return IVM_NONE(NAT_STATE());
		}
	}

	RTM_FATAL(IVM_ERROR_MSG_ILLEGAL_TO_S);

	return IVM_NONE(NAT_STATE());
}

IVM_NATIVE_FUNC(_global_spawn)
{
	ivm_coro_t *coro;

	MATCH_ARG("c", &coro);

	RTM_ASSERT(coro, IVM_ERROR_MSG_RESUME_EMPTY_CORO);
	RTM_ASSERT(ivm_coro_isAlive(coro),
			   IVM_ERROR_MSG_RESUME_DEAD_CORO(coro));
	RTM_ASSERT(!ivm_coro_isActive(coro),
			   IVM_ERROR_MSG_RESUME_ACTIVE_CORO(coro));

	return ivm_vmstate_spawnThread(NAT_STATE(), coro, IVM_NULL);
}

void
ivm_native_global_bind(ivm_vmstate_t *state,
					   ivm_context_t *ctx)
{
	ivm_type_t *type;
	ivm_object_t *cons;
	ivm_type_tag_t i;

	for (i = 1; i < IVM_TYPE_COUNT; i++) {
		type = IVM_BTTYPE(state, i);

		cons = ivm_function_object_new(state, ctx, ivm_type_getCons(type));
		ivm_object_setProto(cons, state, ivm_type_getProto(type));
		ivm_context_setSlot_r(ctx, state, ivm_type_getName(type), cons);
	}

	ivm_context_setSlot_r(ctx, state, "none", IVM_NONE(state));
	ivm_context_setSlot_r(ctx, state, "true", ivm_bool_new(state, IVM_TRUE));
	ivm_context_setSlot_r(ctx, state, "false", ivm_bool_new(state, IVM_FALSE));
	ivm_context_setSlot_r(ctx, state, "eof", ivm_numeric_new(state, EOF));

	ivm_context_setSlot_r(ctx, state, "typename", IVM_NATIVE_WRAP(state, _global_typename));
	ivm_context_setSlot_r(ctx, state, "input", IVM_NATIVE_WRAP(state, _global_input));
	ivm_context_setSlot_r(ctx, state, "print", IVM_NATIVE_WRAP(state, _global_print));

	ivm_context_setSlot_r(ctx, state, "map", IVM_BUILTIN_WRAP(state, _global_map));
	
	ivm_context_setSlot_r(ctx, state, "$import", IVM_NATIVE_WRAP(state, _global_import));
	// ivm_context_setSlot_r(ctx, state, "$is", IVM_NATIVE_WRAP(state, _global_is));

	ivm_context_setSlot_r(ctx, state, "__spawn", IVM_NATIVE_WRAP(state, _global_spawn));

	return;
}
