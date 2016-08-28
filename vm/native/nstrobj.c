#include "pub/type.h"
#include "pub/const.h"
#include "pub/inlines.h"
#include "pub/vm.h"

#include "std/string.h"

#include "vm/strobj.h"
#include "vm/num.h"

#include "priv.h"
#include "nstrobj.h"

IVM_NATIVE_FUNC(_string_len)
{
	const ivm_string_t *str;

	CHECK_BASE(IVM_STRING_OBJECT_T);

	str = ivm_string_object_getValue(IVM_AS(NAT_BASE(), ivm_string_object_t));

	return ivm_numeric_new(NAT_STATE(), str ? ivm_string_realLength(str) : 0);
}
