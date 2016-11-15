#include <math.h>

#include "pub/type.h"
#include "pub/const.h"
#include "pub/inlines.h"
#include "pub/vm.h"

#include "std/string.h"
#include "std/enc.h"

#include "vm/num.h"

#include "priv.h"
#include "nnum.h"

IVM_NATIVE_FUNC(_numeric_cons)
{
	ivm_object_t *arg;
	const ivm_string_t *str;
	ivm_bool_t err = IVM_FALSE;
	ivm_double_t num;

	CHECK_ARG_COUNT(1);

	arg = NAT_ARG_AT(1);

	switch (IVM_TYPE_TAG_OF(arg)) {
		case IVM_NUMERIC_T:
			return ivm_object_clone(arg, NAT_STATE());
	
		case IVM_STRING_OBJECT_T:
			str = ivm_string_object_getValue(arg);
			num = ivm_conv_parseDouble(
				ivm_string_trimHead(str),
				ivm_string_length(str),
				IVM_NULL, &err
			);

			RTM_ASSERT(!err, IVM_ERROR_MSG_FAILED_PARSE_NUM(ivm_string_trimHead(str)));

			return ivm_numeric_new(NAT_STATE(), num);

		default: ;
	}

	RTM_FATAL(IVM_ERROR_MSG_WRONG_ARG_C("object of type <numeric> or <string>"));

	return IVM_NONE(NAT_STATE());
}

IVM_NATIVE_FUNC(_numeric_ceil)
{
	CHECK_BASE(IVM_NUMERIC_T);
	return ivm_numeric_new(NAT_STATE(), ceil(ivm_numeric_getValue(NAT_BASE())));
}

IVM_NATIVE_FUNC(_numeric_floor)
{
	CHECK_BASE(IVM_NUMERIC_T);
	return ivm_numeric_new(NAT_STATE(), floor(ivm_numeric_getValue(NAT_BASE())));
}

IVM_NATIVE_FUNC(_numeric_round)
{
	CHECK_BASE(IVM_NUMERIC_T);
	return ivm_numeric_new(NAT_STATE(), round(ivm_numeric_getValue(NAT_BASE())));
}

IVM_NATIVE_FUNC(_numeric_trunc)
{
	ivm_double_t intg;

	CHECK_BASE(IVM_NUMERIC_T);

	modf(ivm_numeric_getValue(NAT_BASE()), &intg);

	return ivm_numeric_new(NAT_STATE(), intg);
}

IVM_NATIVE_FUNC(_numeric_to_s)
{
	ivm_char_t buf[25];
	
	CHECK_BASE(IVM_NUMERIC_T);

	ivm_conv_dtoa(ivm_numeric_getValue(NAT_BASE()), buf);

	return ivm_string_object_new(NAT_STATE(), ivm_vmstate_constantize_r(NAT_STATE(), buf));
}

IVM_NATIVE_FUNC(_numeric_isnan)
{
	CHECK_BASE(IVM_NUMERIC_T);
	return ivm_bool_new(NAT_STATE(), isnan(ivm_numeric_getValue(NAT_BASE())));
}

IVM_NATIVE_FUNC(_numeric_isinf)
{
	CHECK_BASE(IVM_NUMERIC_T);
	return ivm_bool_new(NAT_STATE(), isinf(ivm_numeric_getValue(NAT_BASE())));
}

IVM_INLINE
ivm_bool_t
_isposinf(ivm_number_t val)
{
	return isinf(val) && val > 0;
}

IVM_INLINE
ivm_bool_t
_isneginf(ivm_number_t val)
{
	return isinf(val) && val < 0;
}

IVM_NATIVE_FUNC(_numeric_isposinf)
{
	CHECK_BASE(IVM_NUMERIC_T);
	return ivm_bool_new(NAT_STATE(), _isposinf(ivm_numeric_getValue(NAT_BASE())));
}

IVM_NATIVE_FUNC(_numeric_isneginf)
{
	CHECK_BASE(IVM_NUMERIC_T);
	return ivm_bool_new(NAT_STATE(), _isneginf(ivm_numeric_getValue(NAT_BASE())));
}

IVM_NATIVE_FUNC(_numeric_char)
{
	ivm_char_t c[2];

	CHECK_BASE(IVM_NUMERIC_T);

	c[0] = (ivm_char_t)ivm_numeric_getValue(NAT_BASE());
	c[1] = '\0';

	return ivm_string_object_new_r(NAT_STATE(), c);
}
