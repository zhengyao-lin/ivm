#include "pub/type.h"
#include "pub/const.h"
#include "pub/inlines.h"
#include "pub/vm.h"

#include "std/string.h"
#include "std/enc.h"

#include "vm/strobj.h"
#include "vm/num.h"

#include "priv.h"
#include "nstrobj.h"

#if 0

IVM_INLINE
ivm_size_t
_get_encode(const ivm_char_t *str,
			ivm_size_t len,
			ivm_char_t *buf,
			ivm_size_t size)
{
	const ivm_char_t *end;
	ivm_size_t oc = 0;
	ivm_char_t c;

	if (!size) return 0;

	for (end = str + len;
		 str != end; str++) {
		c = *str;

		if ((c > 'A' && c < 'Z') ||
			(c > 'a' && c < 'z') ||
			(c > '0' && c < '9')) {
			if (c > 'A' && c < 'Z')
				c += 'a' - 'A';
			buf[oc++] = c;
			if (oc >= size) return oc;
		}
	}

	return oc;
}

#endif

IVM_NATIVE_FUNC(_string_len)
{
	const ivm_string_t *str;

	CHECK_BASE(IVM_STRING_OBJECT_T);

	str = ivm_string_object_getValue(IVM_AS(NAT_BASE(), ivm_string_object_t));

	return ivm_numeric_new(NAT_STATE(), ivm_string_utf8Length(str));
}
