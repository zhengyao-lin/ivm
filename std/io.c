#include <stdio.h>

#include "pub/type.h"
#include "pub/const.h"
#include "pub/err.h"

#include "mem.h"
#include "io.h"

ivm_bool_t
ivm_file_access(const ivm_char_t *path,
				const ivm_char_t *mode)
{
	ivm_file_raw_t fp = IVM_FOPEN(path, mode);

	if (fp) {
		IVM_FCLOSE(fp);
		return IVM_TRUE;
	}

	return IVM_FALSE;
}

ivm_file_t *
ivm_file_new(const ivm_char_t *path,
			 const ivm_char_t *mode)
{
	ivm_file_t *ret;
	ivm_file_raw_t fp = IVM_FOPEN(path, mode);
	if (!fp) return IVM_NULL;

	ret = STD_ALLOC(sizeof(*ret));

	IVM_MEMCHECK(ret);

	ret->fp = fp;

	return ret;
}

ivm_file_t *
ivm_file_new_c(ivm_file_raw_t raw)
{
	ivm_file_t *ret;
	if (!raw) return IVM_NULL;

	ret = STD_ALLOC(sizeof(*ret));

	IVM_MEMCHECK(ret);

	ret->fp = raw;

	return ret;
}

void
ivm_file_free(ivm_file_t *file)
{
	if (file) {
		IVM_FCLOSE(file->fp);
		STD_FREE(file);
	}

	return;
}

#define FGOTO(fp, pos) (IVM_FSEEK((fp), IVM_FSEEK_##pos, 0))

ivm_long_t
ivm_file_length(ivm_file_t *file)
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
ivm_file_readAll_c(ivm_file_t *file,
				   ivm_bool_t save_pos)
{
	ivm_file_raw_t fp = file->fp;
	ivm_size_t orig = IVM_FTELL(fp);
	ivm_size_t len = ivm_file_length(file), tmp_len;
	ivm_char_t *ret, *cur;

	if (len == -1) {
		// read from non-static file
		len = IVM_DEFAULT_FILE_READ_BUFFER_SIZE;
		cur = ret = STD_ALLOC(sizeof(*ret) * (len + 1));

		while ((tmp_len = IVM_FREAD(cur, sizeof(*cur), IVM_DEFAULT_FILE_READ_BUFFER_SIZE, fp))
			   == IVM_DEFAULT_FILE_READ_BUFFER_SIZE) {
			len += IVM_DEFAULT_FILE_READ_BUFFER_SIZE;
			ret = STD_REALLOC(ret, sizeof(*ret) * (len + 1));
			cur = ret + len - IVM_DEFAULT_FILE_READ_BUFFER_SIZE; // next write point
		}

		if (!feof(fp)) {
			STD_FREE(ret);
			return IVM_NULL;
		}

		len -= IVM_DEFAULT_FILE_READ_BUFFER_SIZE - tmp_len;
		ret = STD_REALLOC(ret, sizeof(*ret) * (len + 1));
	} else {
		len -= orig;
		ret = STD_ALLOC(sizeof(*ret) * (len + 1));
		// FGOTO(fp, HEAD);
		if (len != IVM_FREAD(ret, sizeof(*ret), len, fp)) {
			/* unexpected read len */
			STD_FREE(ret);
			return IVM_NULL;
		}
	}

	if (orig != -1 && save_pos) {
		IVM_FSEEK(fp, IVM_FSEEK_HEAD, orig);
	}

	ret[len] = '\0';

	return ret;
}

ivm_char_t *
ivm_file_read_n(ivm_file_t *file,
				ivm_size_t len,
				ivm_bool_t save_pos)
{
	ivm_file_raw_t fp = file->fp;
	ivm_size_t orig = IVM_FTELL(fp);
	ivm_char_t *ret;

	ret = STD_ALLOC(sizeof(*ret) * (len + 1));
	// IVM_TRACE("%ld\n", len);
	
	if (len != IVM_FREAD(ret, sizeof(*ret), len, fp)) {
		/* unexpected read len */
		STD_FREE(ret);
		return IVM_NULL;
	}
	
	if (orig != -1 && save_pos) {
		IVM_FSEEK(fp, IVM_FSEEK_HEAD, orig);
	}
	
	ret[len] = '\0';

	return ret;
}
