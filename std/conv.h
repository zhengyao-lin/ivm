#ifndef _IVM_STD_CONV_H_
#define _IVM_STD_CONV_H_

#include <stdlib.h>

#include "pub/type.h"
#include "pub/com.h"

#include "io.h"
#include "sys.h"

IVM_COM_HEADER

/*
	convert double to string
	buffer no less than 25
 */
ivm_int_t
ivm_conv_dtoa(ivm_double_t d, ivm_char_t dest[25]);

ivm_bool_t
ivm_conv_isAllAscii(const ivm_char_t *str);

IVM_INLINE
ivm_size_t
ivm_conv_mbstowcs_len(const ivm_char_t *mbs)
{
	return mbstowcs(IVM_NULL, mbs, 0);
}

IVM_INLINE
ivm_size_t
ivm_conv_wcstombs_len(const ivm_wchar_t *wcs)
{
	return wcstombs(IVM_NULL, wcs, 0);
}

IVM_INLINE
ivm_size_t
ivm_conv_mbstowcs(const ivm_char_t *mbs, ivm_wchar_t *buf, ivm_size_t buf_size)
{
	return mbstowcs(buf, mbs, buf_size);
}

IVM_INLINE
ivm_size_t
ivm_conv_wcstombs(const ivm_wchar_t *wcs, ivm_char_t *buf, ivm_size_t buf_size)
{
	return wcstombs(buf, wcs, buf_size);
}

IVM_COM_END

#endif
