#ifndef _IVM_STD_SYS_H_
#define _IVM_STD_SYS_H_

#include <stdlib.h>

#include "pub/type.h"
#include "pub/com.h"

IVM_COM_HEADER

#define IVM_EXIT(c) (exit(c))
#define IVM_ABORT() (abort())

ivm_char_t *
ivm_sys_getBasePath(const ivm_char_t *file);

IVM_COM_END

#endif
