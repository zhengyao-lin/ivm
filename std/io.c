#include <stdio.h>

#include "pub/mem.h"
#include "pub/type.h"
#include "pub/err.h"

#include "io.h"

ivm_file_t *
ivm_file_new(const ivm_char_t *path,
			 const ivm_char_t *mode)
{
	ivm_file_t *ret;
	ivm_file_raw_t fp = IVM_FOPEN(path, mode);
	if (!fp) return IVM_NULL;

	ret = MEM_ALLOC(sizeof(*ret), ivm_file_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("file"));

	ret->fp = fp;

	return ret;
}

void
ivm_file_free(ivm_file_t *file)
{
	if (file) {
		IVM_FCLOSE(file->fp);
		MEM_FREE(file);
	}

	return;
}

#define FGOTO(fp, pos) (IVM_FSEEK((fp), IVM_FSEEK_##pos, 0))

IVM_PRIVATE
ivm_size_t
_ivm_file_length(ivm_file_t *file)
{
	ivm_file_raw_t fp = file->fp;
	ivm_size_t cur = IVM_FTELL(fp);
	ivm_size_t ret;

	FGOTO(fp, TAIL);
	ret = IVM_FTELL(fp);
	IVM_FSEEK(fp, IVM_FSEEK_HEAD, cur);

	return ret;
}

ivm_char_t *
ivm_file_readAll(ivm_file_t *file)
{
	ivm_file_raw_t fp = file->fp;
	ivm_size_t cur = IVM_FTELL(fp);
	ivm_size_t len = _ivm_file_length(file);
	ivm_char_t *ret = MEM_ALLOC(sizeof(*ret) * (len + 1),
								ivm_char_t *);

	FGOTO(fp, HEAD);
	if (len != IVM_FREAD(ret, sizeof(*ret), len, fp)) {
		/* unexpected read len */
		MEM_FREE(ret);
		return IVM_NULL;
	}
	IVM_FSEEK(fp, IVM_FSEEK_HEAD, cur);
	ret[len] = '\0';

	return ret;
}
