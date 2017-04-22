#ifndef _IVM_STD_SYS_H_
#define _IVM_STD_SYS_H_

#include <stdlib.h>
#include <locale.h>

#include "pub/type.h"
#include "pub/com.h"

IVM_COM_HEADER

#define IVM_EXIT(c) exit(c)
#define IVM_ABORT() abort()

ivm_char_t *
ivm_sys_getBasePath(const ivm_char_t *file);

#define ivm_sys_setDefaultLocal() setlocale(LC_ALL, "")

#ifdef IVM_OS_WIN32
	#include <direct.h>
	#define ivm_sys_chdir(path) (_chdir(path) == 0)
#else
	#include <unistd.h>
	#define ivm_sys_chdir(path) (chdir(path) == 0)
#endif

IVM_COM_END

#endif
