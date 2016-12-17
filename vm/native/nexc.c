#include "pub/type.h"
#include "pub/const.h"
#include "pub/inlines.h"
#include "pub/vm.h"

#include "std/mem.h"

#include "vm/range.h"
#include "vm/num.h"

#include "priv.h"
#include "nrange.h"

IVM_NATIVE_FUNC(_exception_cons)
{
	const ivm_string_t *msg = IVM_NULL,
					   *file = IVM_NULL;
	ivm_number_t line = 0;

	if (NAT_ARGC() == 1 && CHECK_ARG_1_C(IVM_EXCEPTION_T)) {
		return ivm_object_clone(NAT_ARG_AT(1), NAT_STATE());
	}

	MATCH_ARG("*ssn", &msg, &file, &line);

	return ivm_exception_new(
		NAT_STATE(),
		msg ? ivm_string_trimHead(msg) : IVM_NULL,
		file ? ivm_string_trimHead(file) : IVM_NULL,
		line
	);
}

IVM_NATIVE_FUNC(_exception_to_s)
{
	ivm_exception_t *exc;
	const ivm_string_t *msg = IVM_NULL,
					   *file = IVM_NULL;
	ivm_long_t line = 0;
	ivm_char_t buf[256];

	CHECK_BASE(IVM_EXCEPTION_T);

	exc = IVM_AS(NAT_BASE(), ivm_exception_t);

	msg = ivm_exception_getMsg(exc);
	file = ivm_exception_getFile(exc);
	line = ivm_exception_getLine(exc);

	IVM_SNPRINTF(buf, IVM_ARRLEN(buf), "%s: line %ld: %s",
				 ivm_string_trimHead(file), line, ivm_string_trimHead(msg));

	return ivm_string_object_new_r(NAT_STATE(), buf);
}
