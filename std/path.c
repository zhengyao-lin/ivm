#include "pub/com.h"
#include "pub/err.h"

#include "string.h"
#include "mem.h"
#include "path.h"

#if defined(IVM_OS_WIN32)

ivm_bool_t
ivm_path_realpath(ivm_char_t buffer[IVM_PATH_MAX_LEN + 1],
				  ivm_char_t *rpath)
{
	ivm_char_t tmp[IVM_PATH_MAX_LEN + 1];

	if (_fullpath(tmp, rpath, IVM_PATH_MAX_LEN)) {
		STD_MEMCPY(buffer, tmp, IVM_STRLEN(tmp) + 1);
		return IVM_TRUE;
	}

	return IVM_FALSE;
}

#else

ivm_bool_t
ivm_path_realpath(ivm_char_t buffer[IVM_PATH_MAX_LEN + 1],
				  ivm_char_t *rpath)
{
	ivm_char_t tmp[IVM_PATH_MAX_LEN + 1];

	if (realpath(rpath, tmp)) {
		STD_MEMCPY(buffer, tmp, IVM_STRLEN(tmp) + 1);
		return IVM_TRUE;
	}

	return IVM_FALSE;
}

#endif
