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
	CHECK_ARG_1(IVM_NUMERIC_T);
	return ivm_object_clone(NAT_ARG_AT(1), NAT_STATE());
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

IVM_NATIVE_FUNC(_numeric_isnan)
{
	CHECK_BASE(IVM_NUMERIC_T);
	return ivm_numeric_new(NAT_STATE(), isnan(ivm_numeric_getValue(NAT_BASE())));
}

IVM_NATIVE_FUNC(_numeric_isinf)
{
	CHECK_BASE(IVM_NUMERIC_T);
	return ivm_numeric_new(NAT_STATE(), isinf(ivm_numeric_getValue(NAT_BASE())));
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
	return ivm_numeric_new(NAT_STATE(), _isposinf(ivm_numeric_getValue(NAT_BASE())));
}

IVM_NATIVE_FUNC(_numeric_isneginf)
{
	CHECK_BASE(IVM_NUMERIC_T);
	return ivm_numeric_new(NAT_STATE(), _isneginf(ivm_numeric_getValue(NAT_BASE())));
}

IVM_NATIVE_FUNC(_numeric_char)
{
	ivm_char_t c[2];

	CHECK_BASE(IVM_NUMERIC_T);

	c[0] = (ivm_char_t)ivm_numeric_getValue(NAT_BASE());
	c[1] = '\0';

	return ivm_string_object_new_r(NAT_STATE(), c);
}
