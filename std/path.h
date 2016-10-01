#ifndef _IVM_STD_PATH_H_
#define _IVM_STD_PATH_H_

#include <stdlib.h>

#include "pub/com.h"
#include "pub/type.h"

IVM_COM_HEADER

#if defined(IVM_OS_LINUX)
	#include <sys/param.h>
	#include <limits.h>

	#ifdef PATH_MAX
		#define IVM_PATH_MAX_LEN PATH_MAX
	#endif
#endif

#ifndef IVM_PATH_MAX_LEN
	#define IVM_PATH_MAX_LEN 1024
#endif

ivm_bool_t
ivm_path_realpath(ivm_char_t buffer[IVM_PATH_MAX_LEN + 1],
				  ivm_char_t *rpath /* relative path */);

IVM_COM_END

#endif
