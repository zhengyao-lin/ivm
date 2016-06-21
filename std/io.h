#ifndef _IVM_STD_IO_H_
#define _IVM_STD_IO_H_

#include "pub/com.h"
#include "pub/type.h"

#include <stdio.h>

IVM_COM_HEADER

#define IVM_STDIN	(stdin)
#define IVM_STDOUT	(stdout)
#define IVM_STDERR	(stderr)

#define IVM_OUT(...) \
	/* (fprintf(IVM_STDOUT, __VA_ARGS__)) */

#define IVM_TRACE(...) \
	(fprintf(IVM_STDERR, __VA_ARGS__))

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

typedef struct {
	ivm_file_raw_t fp;
} ivm_file_t;

ivm_file_t *
ivm_file_new(const ivm_char_t *path,
			 const ivm_char_t *mode);

void
ivm_file_free(ivm_file_t *file);

ivm_char_t *
ivm_file_readAll(ivm_file_t *file);

IVM_COM_END

#endif
