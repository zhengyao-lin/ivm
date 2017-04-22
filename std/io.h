#ifndef _IVM_STD_IO_H_
#define _IVM_STD_IO_H_

#include <stdio.h>

#include "pub/com.h"
#include "pub/type.h"

IVM_COM_HEADER

#define IVM_STDIN	(stdin)
#define IVM_STDOUT	(stdout)
#define IVM_STDERR	(stderr)

#define IVM_OUT(...) \
	/* (fprintf(IVM_STDOUT, __VA_ARGS__)) */

#define IVM_TRACE(...) \
	(fprintf(IVM_STDERR, __VA_ARGS__))

#if defined(IVM_OS_WIN32)
	#define IVM_SNPRINTF(str, len, ...) (_snprintf((str), (len) - 1, __VA_ARGS__))
#else
	#define IVM_SNPRINTF(str, len, ...) (snprintf((str), (len), __VA_ARGS__))
#endif

#define ivm_fputs(fp, str) fputs((str), (fp))
#define ivm_fgets(fp, buf, size) fgets((buf), (size), (fp))

IVM_INLINE
void
ivm_fputs_n(FILE *fp, const ivm_char_t *str, ivm_size_t len)
{
	const ivm_char_t *end = str + len;
	while (str != end) fputc(*str++, fp);
	return;
}

typedef FILE *ivm_file_raw_t;

#define IVM_FOPEN fopen
#define IVM_FCLOSE fclose
#define IVM_FSEEK(fp, pos, ofs) (fseek((fp), (ofs), (pos)))
#define IVM_REWIND rewind
#define IVM_FSEEK_HEAD SEEK_SET
#define IVM_FSEEK_CUR SEEK_CUR
#define IVM_FSEEK_TAIL SEEK_END
#define IVM_FTELL ftell

#define IVM_FREAD fread
#define IVM_FWRITE fwrite
#define IVM_FFLUSH fflush

#define IVM_FMODE_WRITE_BINARY "wb"
#define IVM_FMODE_READ_BINARY "rb"

ivm_bool_t
ivm_file_access(const ivm_char_t *path,
				const ivm_char_t *mode);

#define ivm_file_remove(file) (remove(file) == 0)

typedef struct {
	ivm_file_raw_t fp;
} ivm_file_t;

ivm_file_t *
ivm_file_new(const ivm_char_t *path,
			 const ivm_char_t *mode);

ivm_file_t *
ivm_file_new_c(ivm_file_raw_t raw);

void
ivm_file_free(ivm_file_t *file);

void
ivm_file_free_n(ivm_file_t *file);

ivm_char_t *
ivm_file_readAll(ivm_file_t *file,
				 ivm_size_t *size);

#define ivm_file_flush(file) IVM_FFLUSH((file)->fp)

ivm_char_t *
ivm_file_read_n(ivm_file_t *file,
				ivm_size_t len);

IVM_INLINE
ivm_size_t
ivm_file_write(ivm_file_t *file,
			   const void *buf,
			   ivm_size_t size,
			   ivm_size_t count)
{
	ivm_file_raw_t fp = file->fp;
	ivm_size_t ret = 0;

	if (fp) {
		ret = IVM_FWRITE(buf, size, count, fp);
	}

	return ret;
}

IVM_INLINE
ivm_size_t
ivm_file_writeAt(ivm_file_t *file,
				 ivm_size_t pos,
				 const void *buf,
				 ivm_size_t size,
				 ivm_size_t count)
{
	ivm_file_raw_t fp = file->fp;
	ivm_size_t ret = 0;
	ivm_size_t cur;

	if (fp) {
		cur = IVM_FTELL(fp);
		IVM_FSEEK(fp, IVM_FSEEK_HEAD, pos);
		ret = IVM_FWRITE(buf, size, count, fp);
		IVM_FSEEK(fp, IVM_FSEEK_HEAD, cur);
	}

	return ret;
}

ivm_size_t
ivm_file_length(ivm_file_t *file);

#define ivm_file_isStatic(file) (ivm_file_length(file) != -1)
#define ivm_file_isEnd(file) (feof((file)->fp))

IVM_INLINE
ivm_size_t
ivm_file_read(ivm_file_t *file,
			  void *buf,
			  ivm_size_t size,
			  ivm_size_t count)
{
	ivm_file_raw_t fp = file->fp;
	ivm_size_t ret = 0;

	if (fp) {
		ret = IVM_FREAD(buf, size, count, fp);
	}

	return ret;
}

/* save pos */
IVM_INLINE
ivm_size_t
ivm_file_read_s(ivm_file_t *file,
				void *buf,
				ivm_size_t size,
				ivm_size_t count,
				ivm_bool_t save_pos)
{
	ivm_file_raw_t fp = file->fp;
	ivm_size_t orig = IVM_FTELL(fp);
	ivm_size_t ret = 0;

	if (fp) {
		ret = IVM_FREAD(buf, size, count, fp);
	}

	if (orig != -1 && save_pos) {
		IVM_FSEEK(fp, IVM_FSEEK_HEAD, orig);
	}

	return ret;
}

IVM_INLINE
ivm_long_t
ivm_file_curPos(ivm_file_t *file)
{
	ivm_file_raw_t fp = file->fp;

	if (fp) {
		return IVM_FTELL(fp);
	}

	return 0;
}

IVM_INLINE
ivm_bool_t
ivm_file_setPos(ivm_file_t *file,
				ivm_size_t pos)
{
	return !IVM_FSEEK(file->fp, IVM_FSEEK_HEAD, pos);
}

struct ivm_stream_t_tag;
typedef ivm_size_t (*ivm_stream_writer_t)(struct ivm_stream_t_tag *self, const void *data, ivm_size_t esize, ivm_size_t count);
typedef ivm_size_t (*ivm_stream_reader_t)(struct ivm_stream_t_tag *self, void *buf, ivm_size_t esize, ivm_size_t count);
typedef void (*ivm_stream_destructor_t)(struct ivm_stream_t_tag *self);

#define IVM_STREAM_HEADER \
	ivm_stream_writer_t write; \
	ivm_stream_reader_t read; \
	ivm_stream_destructor_t des;

typedef struct ivm_stream_t_tag {
	IVM_STREAM_HEADER
} ivm_stream_t;

void
ivm_stream_init(ivm_stream_t *stream,
				ivm_stream_writer_t writer,
				ivm_stream_reader_t reader,
				ivm_stream_destructor_t des);

ivm_stream_t *
ivm_stream_new(ivm_stream_writer_t writer,
			   ivm_stream_reader_t reader,
			   ivm_stream_destructor_t des);

void
ivm_stream_free(ivm_stream_t *stream);

void
ivm_stream_dump(ivm_stream_t *stream);

IVM_INLINE
ivm_size_t
ivm_stream_write(ivm_stream_t *stream, const void *data, ivm_size_t esize, ivm_size_t count)
{
	return stream->write(stream, data, esize, count);
}

IVM_INLINE
ivm_size_t
ivm_stream_read(ivm_stream_t *stream, void *buf, ivm_size_t esize, ivm_size_t count)
{
	return stream->read(stream, buf, esize, count);
}

typedef struct {
	IVM_STREAM_HEADER
	ivm_file_t *fp;
} ivm_file_stream_t;

ivm_stream_t *
ivm_file_stream_new(ivm_file_t *fp);

typedef struct {
	IVM_STREAM_HEADER
	ivm_byte_t *buf;
	
	ivm_size_t alloc;

	ivm_size_t wcur;
	ivm_size_t rcur;
} ivm_buffer_stream_t;

ivm_stream_t *
ivm_buffer_stream_new(ivm_byte_t *buf, ivm_size_t size);

#define ivm_buffer_stream_getBuffer(bstream) ((bstream)->buf)
#define ivm_buffer_stream_getSize(bstream) ((bstream)->wcur)

IVM_COM_END

#endif
