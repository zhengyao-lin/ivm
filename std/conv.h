#ifndef _IVM_STD_CONV_H_
#define _IVM_STD_CONV_H_

#include "pub/type.h"
#include "pub/com.h"

IVM_COM_HEADER

/*
	convert double to string
	buffer no less than 25
 */
ivm_int_t
ivm_dtoa(ivm_double_t d, ivm_char_t dest[25]);

IVM_COM_END

#endif
