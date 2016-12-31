#include "pub/err.h"
#include "pub/vm.h"
#include "pub/inlines.h"

#include "std/mem.h"

#include "exc.h"
#include "obj.h"

ivm_object_t *
ivm_exception_new(ivm_vmstate_t *state,
				  const ivm_char_t *msg,
				  const ivm_char_t *file,
				  ivm_long_t line)
{
	ivm_exception_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), IVM_BTTYPE(state, IVM_EXCEPTION_T));

	ret->msg = msg ? IVM_CSTR(state, msg) : IVM_VMSTATE_CONST(state, C_NOMSG);
	ret->file = file ? IVM_CSTR(state, file) : IVM_VMSTATE_CONST(state, C_UNTRACE);
	ret->line = line;

	return IVM_AS_OBJ(ret);
}

ivm_object_t *
ivm_exception_new_c(ivm_vmstate_t *state,
					const ivm_string_t *msg,
					const ivm_string_t *file,
					ivm_long_t line)
{
	ivm_exception_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), IVM_BTTYPE(state, IVM_EXCEPTION_T));

	ret->msg = msg ? ivm_vmstate_constantize(state, msg) : IVM_VMSTATE_CONST(state, C_NOMSG);
	ret->file = file ? ivm_vmstate_constantize(state, file) : IVM_VMSTATE_CONST(state, C_UNTRACE);
	ret->line = line;

	return IVM_AS_OBJ(ret);
}

void
ivm_exception_setMsg(ivm_exception_t *exc,
					 ivm_vmstate_t *state,
					 const ivm_char_t *msg)
{
	exc->msg = msg ? IVM_CSTR(state, msg) : IVM_VMSTATE_CONST(state, C_NOMSG);
	return;
}

void
ivm_exception_setPos(ivm_exception_t *exc,
					 ivm_vmstate_t *state,
					 const ivm_char_t *file,
					 ivm_long_t line)
{
	exc->file = file ? IVM_CSTR(state, file) : IVM_VMSTATE_CONST(state, C_UNTRACE);
	exc->line = line;

	return;
}
