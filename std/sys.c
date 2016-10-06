#include "pub/type.h"
#include "pub/err.h"
#include "pub/com.h"

#include "mem.h"
#include "string.h"
#include "sys.h"

ivm_char_t *
ivm_sys_getBasePath(const ivm_char_t *file)
{
	ivm_size_t len = IVM_STRLEN(file);
	const ivm_char_t *i, *end;
	ivm_char_t *ret;

	for (i = file + len - 1, end = file - 1;
		 i != end; i--) {
		if (*i == '/' ||
			*i == '\\') {
			len = IVM_PTR_DIFF(i, file, ivm_char_t);

			if (!len) return IVM_STRDUP(IVM_FILE_SEPARATOR_S);

			ret = STD_ALLOC(sizeof(*ret) * (len + 1));
			IVM_MEMCHECK(ret);

			STD_MEMCPY(ret, file, len);
			ret[len] = '\0';
			
			return ret;
		}
	}

	return IVM_STRDUP(".");
}
