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

void
ivm_file_free_n(ivm_file_t *file)
{
	if (file) {
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

void
ivm_stream_init(ivm_stream_t *stream,
				ivm_stream_writer_t writer,
				ivm_stream_reader_t reader,
				ivm_stream_destructor_t des)
{
	stream->write = writer;
	stream->read = reader;
	stream->des = des;
	return;
}

ivm_stream_t *
ivm_stream_new(ivm_stream_writer_t writer,
			   ivm_stream_reader_t reader,
			   ivm_stream_destructor_t des)
{
	ivm_stream_t *ret = STD_ALLOC(sizeof(*ret));
	IVM_MEMCHECK(ret);

	ivm_stream_init(ret, writer, reader, des);

	return ret;
}

void
ivm_stream_free(ivm_stream_t *stream)
{
	if (stream) {
		if (stream->des) {
			stream->des(stream);
		}

		STD_FREE(stream);
	}

	return;
}

void
ivm_stream_dump(ivm_stream_t *stream)
{
	return;
}

IVM_PRIVATE
ivm_size_t
_ivm_file_stream_write(ivm_stream_t *stream,
					   const void *data, ivm_size_t esize, ivm_size_t count)
{
	return ivm_file_write(((ivm_file_stream_t *)stream)->fp, data, esize, count);
}

IVM_PRIVATE
ivm_size_t
_ivm_file_stream_read(ivm_stream_t *stream,
					  void *buf, ivm_size_t esize, ivm_size_t count)
{
	return ivm_file_read(((ivm_file_stream_t *)stream)->fp, buf, esize, count);
}

ivm_stream_t *
ivm_file_stream_new(ivm_file_t *fp)
{
	ivm_file_stream_t *ret = STD_ALLOC(sizeof(*ret));
	IVM_MEMCHECK(ret);

	ivm_stream_init((ivm_stream_t *)ret,
					_ivm_file_stream_write,
					_ivm_file_stream_read, IVM_NULL);

	ret->fp = fp;

	return (ivm_stream_t *)ret;
}
