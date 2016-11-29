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

void
ivm_native_global_bind(ivm_vmstate_t *state,
					   ivm_context_t *ctx)
{
	ivm_type_t *type;
	ivm_object_t *cons;
	ivm_type_tag_t i;

	for (i = 0; i < IVM_TYPE_COUNT; i++) {
		type = IVM_BTTYPE(state, i);
		cons = ivm_function_object_newNative(state, ivm_type_getCons(type));
		ivm_object_setProto(cons, state, ivm_type_getProto(type));

		ivm_context_setSlot_r(ctx, state, ivm_type_getName(type), cons);
	}

	ivm_context_setSlot_r(ctx, state, "none", IVM_NONE(state));
	ivm_context_setSlot_r(ctx, state, "true", ivm_bool_new(state, IVM_TRUE));
	ivm_context_setSlot_r(ctx, state, "false", ivm_bool_new(state, IVM_FALSE));

	ivm_context_setSlot_r(ctx, state, "typename", IVM_NATIVE_WRAP(state, _global_typename));
	ivm_context_setSlot_r(ctx, state, "input", IVM_NATIVE_WRAP(state, _global_input));
	
	ivm_context_setSlot_r(ctx, state, "$import", IVM_NATIVE_WRAP(state, _global_import));
	// ivm_context_setSlot_r(ctx, state, "$is", IVM_NATIVE_WRAP(state, _global_is));

	return;
}
