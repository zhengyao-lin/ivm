#include "pub/type.h"
#include "pub/mem.h"
#include "pub/err.h"

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

			ret = MEM_ALLOC(sizeof(*ret) * (len + 1),
							ivm_char_t *);
			IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("path"));

			MEM_COPY(ret, file, len);
			ret[len] = '\0';
			
			return ret;
		}
	}

	return IVM_STRDUP(".");
}
