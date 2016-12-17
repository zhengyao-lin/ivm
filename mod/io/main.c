#include <stdio.h>

#include "pub/com.h"
#include "pub/type.h"
#include "pub/vm.h"
#include "pub/obj.h"
#include "pub/inlines.h"

#include "std/io.h"
#include "std/string.h"
#include "std/time.h"

#include "vm/native/native.h"
#include "vm/native/priv.h"

#if IVM_OS_LINUX || IVM_OS_MAC

	#include "termios.h"
	#include "unistd.h"
	#include "fcntl.h"

	#define IO_USE_TERMIOS 1

#else
	#define IO_USE_TERMIOS 0
#endif

IVM_PRIVATE
ivm_int_t _type_uid;

#define IO_FILE_TYPE_UID (&_type_uid)

#define IO_ERROR_MSG_FAILED_TO_OPEN_FILE(file)							"failed to open file '%s'", (file)
#define IO_ERROR_MSG_UNINIT_FILE_POINTER								"uninitialized file pointer"
#define IO_ERROR_MSG_FAILED_READ_FILE									"failed to read file"
#define IO_ERROR_MSG_TOO_LARGE_LENGTH(len, max)							"too large length(length %ld for the file of length %ld)", (ivm_long_t)(len), (ivm_long_t)(max)
#define IO_ERROR_MSG_TOO_SMALL_BUF(len, req)							"too small buffer(require at least %ld, buffer of size %ld given)", (ivm_long_t)(req), (ivm_long_t)(len)
#define IO_ERROR_MSG_FAILED_SET_POS(pos)								"failed to set read position %ld", (ivm_long_t)(pos)
#define IO_ERROR_MSG_FAILED_WRITE_FILE									"failed to write file"
#define IO_ERROR_MSG_FAILED_REMOVE_FILE(file)							"failed to remove file '%s'", (file)
#define IO_ERROR_MSG_NEG_SIZE											"negative size"

#define CHECK_INIT_FP(fobj) \
	RTM_ASSERT((fobj)->fp, IO_ERROR_MSG_UNINIT_FILE_POINTER)

typedef struct {
	IVM_OBJECT_HEADER
	ivm_file_t *fp;
} ivm_file_object_t;

ivm_object_t *
ivm_file_object_new(ivm_vmstate_t *state,
					ivm_file_t *fp)
{
	ivm_file_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), IVM_TPTYPE(state, IO_FILE_TYPE_UID));

	ret->fp = fp;

	ivm_vmstate_addDesLog(state, IVM_AS_OBJ(ret));

	return IVM_AS_OBJ(ret);
}

void
ivm_file_object_destructor(ivm_object_t *obj,
						   ivm_vmstate_t *state)
{
	ivm_file_free(IVM_AS(obj, ivm_file_object_t)->fp);
	return;
}

void
ivm_file_object_cloner(ivm_object_t *obj,
					   ivm_vmstate_t *state)
{
	IVM_AS(obj, ivm_file_object_t)->fp = IVM_NULL;
	ivm_vmstate_addDesLog(state, obj);
	return;
}

IVM_NATIVE_FUNC(_io_file)
{
	const ivm_string_t *file, *mode = IVM_NULL;
	const ivm_char_t *rfile;
	ivm_file_t *fp;

	MATCH_ARG("s*s", &file, &mode);

	rfile = ivm_string_trimHead(file);

	if (mode) {
		fp = ivm_file_new(rfile, ivm_string_trimHead(mode));
	} else {
		fp = ivm_file_new(rfile, IVM_FMODE_READ_BINARY);
	}

	RTM_ASSERT(fp, IO_ERROR_MSG_FAILED_TO_OPEN_FILE(rfile));

	return ivm_file_object_new(NAT_STATE(), fp);
}

IVM_NATIVE_FUNC(_io_file_close)
{
	ivm_file_object_t *fobj;

	CHECK_BASE_TP(IO_FILE_TYPE_UID);

	fobj = IVM_AS(NAT_BASE(), ivm_file_object_t);
	CHECK_INIT_FP(fobj);

	ivm_file_free(fobj->fp);
	fobj->fp = IVM_NULL;

	return IVM_NONE(NAT_STATE());
}

IVM_NATIVE_FUNC(_io_file_read)
{
	ivm_char_t *cont;
	ivm_file_object_t *fobj;
	ivm_object_t *ret;
	ivm_number_t arg = -1, save_pos = IVM_FALSE;
	ivm_long_t flen, len;

	CHECK_BASE_TP(IO_FILE_TYPE_UID);
	MATCH_ARG("*nn", &arg, &save_pos);

	fobj = IVM_AS(NAT_BASE(), ivm_file_object_t);
	CHECK_INIT_FP(fobj);

	flen = ivm_file_length(fobj->fp);

	if (!HAS_ARG(1)) len = flen - ivm_file_curPos(fobj->fp);
	else {
		RTM_ASSERT(arg >= 0, IO_ERROR_MSG_NEG_SIZE);
		len = arg;
	}

	if (flen == -1) {
		if (!HAS_ARG(1)) {
			cont = ivm_file_readAll_c(fobj->fp, save_pos);
		} else {
			cont = ivm_file_read_n(fobj->fp, len, save_pos);
		}
	} else {
		RTM_ASSERT(len <= flen, IO_ERROR_MSG_TOO_LARGE_LENGTH(len, flen));
		cont = ivm_file_read_n(fobj->fp, len, save_pos);
	}

	RTM_ASSERT(cont, IO_ERROR_MSG_FAILED_READ_FILE);

	ret = ivm_string_object_new_r(NAT_STATE(), cont);
	STD_FREE(cont);

	return ret;
}

IVM_NATIVE_FUNC(_io_file_readBuffer)
{
	ivm_char_t *cont;
	ivm_file_object_t *fobj;
	ivm_number_t arg = -1, save_pos = IVM_FALSE;
	ivm_long_t flen, len;

	CHECK_BASE_TP(IO_FILE_TYPE_UID);
	MATCH_ARG("*nn", &arg, &save_pos);

	fobj = IVM_AS(NAT_BASE(), ivm_file_object_t);
	CHECK_INIT_FP(fobj);

	flen = ivm_file_length(fobj->fp);

	if (!HAS_ARG(1)) len = flen - ivm_file_curPos(fobj->fp);
	else {
		RTM_ASSERT(arg >= 0, IO_ERROR_MSG_NEG_SIZE);
		len = arg;
	}

	if (flen == -1) {
		if (!HAS_ARG(1)) {
			cont = ivm_file_readAll_c(fobj->fp, save_pos);
		} else {
			cont = ivm_file_read_n(fobj->fp, len, save_pos);
		}
	} else {
		RTM_ASSERT(len <= flen, IO_ERROR_MSG_TOO_LARGE_LENGTH(len, flen));
		cont = ivm_file_read_n(fobj->fp, len, save_pos);
	}

	RTM_ASSERT(cont, IO_ERROR_MSG_FAILED_READ_FILE);

	return ivm_buffer_object_new_c(NAT_STATE(), len, (ivm_byte_t *)cont);
}

IVM_NATIVE_FUNC(_io_file_readToBuffer)
{
	ivm_file_object_t *fobj;
	ivm_buffer_object_t *buf_obj;
	ivm_number_t arg = -1, save_pos = IVM_FALSE;
	ivm_size_t bsize;
	ivm_long_t flen, len;

	CHECK_BASE_TP(IO_FILE_TYPE_UID);
	MATCH_ARG("b*nn", &buf_obj, &arg, &save_pos);

	// read length:
	//     1. the length specified
	//     2. file length
	//     3. size the the buffer(is the file is non-static)

	fobj = IVM_AS(NAT_BASE(), ivm_file_object_t);
	CHECK_INIT_FP(fobj);

	flen = ivm_file_length(fobj->fp);
	bsize = ivm_buffer_object_getSize(buf_obj);

	if (!HAS_ARG(2)) {
		if (flen == -1) {
			len = bsize;
		} else {
			len = flen - ivm_file_curPos(fobj->fp);
		}
	} else {
		RTM_ASSERT(arg >= 0, IO_ERROR_MSG_NEG_SIZE);
		len = arg;
	}

	RTM_ASSERT(len <= bsize, IO_ERROR_MSG_TOO_SMALL_BUF(bsize, len));

	if (flen != -1) {
		RTM_ASSERT(len <= flen, IO_ERROR_MSG_TOO_LARGE_LENGTH(len, flen));
	}

	if (ivm_file_read_s(fobj->fp, ivm_buffer_object_getRaw(buf_obj), sizeof(ivm_char_t), len, save_pos)
		!= len) {
		RTM_ASSERT(ivm_file_isEnd(fobj->fp) && !HAS_ARG(2), IO_ERROR_MSG_FAILED_READ_FILE);
	}

	return IVM_NONE(NAT_STATE());
}

IVM_NATIVE_FUNC(_io_file_write)
{
	ivm_file_object_t *fobj;
	const ivm_string_t *str;
	ivm_size_t len;

	CHECK_BASE_TP(IO_FILE_TYPE_UID);
	MATCH_ARG("s", &str);

	fobj = IVM_AS(NAT_BASE(), ivm_file_object_t);
	CHECK_INIT_FP(fobj);

	len = ivm_string_length(str);
	RTM_ASSERT(ivm_file_write(fobj->fp, ivm_string_trimHead(str), sizeof(ivm_char_t), len)
			   == len, IO_ERROR_MSG_FAILED_WRITE_FILE);

	return IVM_NONE(NAT_STATE());
}

IVM_NATIVE_FUNC(_io_file_writeBuffer)
{
	ivm_file_object_t *fobj;
	ivm_buffer_object_t *buf_obj;
	ivm_size_t len;

	CHECK_BASE_TP(IO_FILE_TYPE_UID);
	MATCH_ARG("b", &buf_obj);

	fobj = IVM_AS(NAT_BASE(), ivm_file_object_t);
	CHECK_INIT_FP(fobj);

	len = ivm_buffer_object_getSize(buf_obj);
	RTM_ASSERT(ivm_file_write(fobj->fp, ivm_buffer_object_getRaw(buf_obj), sizeof(ivm_byte_t), len)
			   == len, IO_ERROR_MSG_FAILED_WRITE_FILE);

	return IVM_NONE(NAT_STATE());
}

IVM_NATIVE_FUNC(_io_file_len)
{
	ivm_file_object_t *fobj;

	CHECK_BASE_TP(IO_FILE_TYPE_UID);

	fobj = IVM_AS(NAT_BASE(), ivm_file_object_t);
	CHECK_INIT_FP(fobj);

	return ivm_numeric_new(NAT_STATE(), ivm_file_length(fobj->fp));
}

IVM_NATIVE_FUNC(_io_file_lines)
{
	ivm_char_t *cont;
	ivm_file_object_t *fobj;
	ivm_list_object_t *ret;
	ivm_size_t len;
	const ivm_char_t *i, *end, *head;

	CHECK_BASE_TP(IO_FILE_TYPE_UID);

	fobj = IVM_AS(NAT_BASE(), ivm_file_object_t);
	CHECK_INIT_FP(fobj);

	cont = ivm_file_readAll(fobj->fp);
	RTM_ASSERT(cont, IO_ERROR_MSG_FAILED_READ_FILE);

	ret = IVM_AS(ivm_list_object_new(NAT_STATE(), 0), ivm_list_object_t);
	len = IVM_STRLEN(cont);

	for (head = i = cont, end = i + len;
		 i != end; i++) {
		if (*i == '\n') {
			if (!ivm_list_object_push(
				ret, NAT_STATE(),
				ivm_string_object_new_rl(NAT_STATE(), head, IVM_PTR_DIFF(i, head, ivm_char_t))
			)) goto ERROR;
			head = i + 1;
		}
	}

	if (!ivm_list_object_push(
		ret, NAT_STATE(),
		ivm_string_object_new_rl(NAT_STATE(), head, IVM_PTR_DIFF(i, head, ivm_char_t))
	)) goto ERROR;

	STD_FREE(cont);

goto ERROR_END;
ERROR:
	STD_FREE(cont);
	return IVM_NULL;
ERROR_END:;

	return IVM_AS_OBJ(ret);
}

IVM_NATIVE_FUNC(_io_file_seek)
{
	ivm_file_object_t *fobj;
	ivm_number_t pos = 0;

	CHECK_BASE_TP(IO_FILE_TYPE_UID);
	MATCH_ARG("*n", &pos);

	fobj = IVM_AS(NAT_BASE(), ivm_file_object_t);
	CHECK_INIT_FP(fobj);

	RTM_ASSERT(ivm_file_setPos(fobj->fp, pos), IO_ERROR_MSG_FAILED_SET_POS(pos));

	return IVM_NONE(NAT_STATE());
}

IVM_NATIVE_FUNC(_io_file_cur)
{
	ivm_file_object_t *fobj;

	CHECK_BASE_TP(IO_FILE_TYPE_UID);

	fobj = IVM_AS(NAT_BASE(), ivm_file_object_t);
	CHECK_INIT_FP(fobj);

	return ivm_numeric_new(NAT_STATE(), ivm_file_curPos(fobj->fp));
}

IVM_NATIVE_FUNC(_io_file_flush)
{
	ivm_file_object_t *fobj;

	CHECK_BASE_TP(IO_FILE_TYPE_UID);

	fobj = IVM_AS(NAT_BASE(), ivm_file_object_t);
	CHECK_INIT_FP(fobj);

	ivm_file_flush(fobj->fp);

	return IVM_NONE(NAT_STATE());
}

IVM_NATIVE_FUNC(_io_remove)
{
	const ivm_string_t *str;
	const ivm_char_t *file;

	MATCH_ARG("s", &str);

	file = ivm_string_trimHead(str);

	RTM_ASSERT(ivm_file_remove(file), IO_ERROR_MSG_FAILED_REMOVE_FILE(file));

	return IVM_NONE(NAT_STATE());
}

#if IO_USE_TERMIOS

IVM_NATIVE_FUNC(_io_kbhit)
{
	struct termios orig, cur;
	ivm_int_t ch;
	ivm_int_t oldf;
	ivm_schar_t tmp;
	ivm_number_t echo = IVM_FALSE;

	MATCH_ARG("*n", &echo);

	tcgetattr(STDIN_FILENO, &orig);
	
	// set no buffer
	cur = orig;

	if (echo) {
		cur.c_lflag &= ~ICANON;
		cur.c_lflag |= ECHO;
	} else {
		cur.c_lflag &= ~(ICANON | ECHO);
	}

	tcsetattr(STDIN_FILENO, TCSANOW, &cur);

	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	while ((ch = fgetc(stdin)) == EOF) {
		ivm_time_msleep(10);
	}

	// key pressed
	while ((tmp = fgetc(stdin)) != EOF) {
		ch = (ch << 8) + tmp;
	}
	// key released

	tcsetattr(STDIN_FILENO, TCSANOW, &orig);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	return ivm_numeric_new(NAT_STATE(), ch);
}

#endif

ivm_object_t *
ivm_mod_main(ivm_vmstate_t *state,
			 ivm_coro_t *coro,
			 ivm_context_t *context)
{
	ivm_object_t *mod = ivm_object_new(state);
	ivm_object_t *file_proto;

	ivm_type_t _io_file_type = IVM_TPTYPE_BUILD(
		"io.file", sizeof(ivm_file_object_t),
		IVM_NATIVE_WRAP_C(state, _io_file),
		IO_FILE_TYPE_UID,

		.des = ivm_file_object_destructor,
		.clone = ivm_file_object_cloner,
		.const_bool = IVM_TRUE
	);

	/* io.file */
	IVM_VMSTATE_REGISTER_TPTYPE(state, coro, &_io_file_type, {
		file_proto = ivm_file_object_new(state, IVM_NULL);
		ivm_type_setProto(_TYPE, file_proto);
		ivm_object_setProto(file_proto, state, ivm_vmstate_getTypeProto(state, IVM_OBJECT_T));

		ivm_object_setSlot_r(file_proto, state, "close", IVM_NATIVE_WRAP(state, _io_file_close));
		
		ivm_object_setSlot_r(file_proto, state, "read", IVM_NATIVE_WRAP(state, _io_file_read));
		ivm_object_setSlot_r(file_proto, state, "readBuffer", IVM_NATIVE_WRAP(state, _io_file_readBuffer));
		ivm_object_setSlot_r(file_proto, state, "readToBuffer", IVM_NATIVE_WRAP(state, _io_file_readToBuffer));
		
		ivm_object_setSlot_r(file_proto, state, "write", IVM_NATIVE_WRAP(state, _io_file_write));
		ivm_object_setSlot_r(file_proto, state, "writeBuffer", IVM_NATIVE_WRAP(state, _io_file_writeBuffer));

		ivm_object_setSlot_r(file_proto, state, "len", IVM_NATIVE_WRAP(state, _io_file_len));
		ivm_object_setSlot_r(file_proto, state, "lines", IVM_NATIVE_WRAP(state, _io_file_lines));
		ivm_object_setSlot_r(file_proto, state, "seek", IVM_NATIVE_WRAP(state, _io_file_seek));
		ivm_object_setSlot_r(file_proto, state, "cur", IVM_NATIVE_WRAP(state, _io_file_cur));

		ivm_object_setSlot_r(file_proto, state, "flush", IVM_NATIVE_WRAP(state, _io_file_flush));
	});

	/* io */
	ivm_object_setSlot_r(mod, state, "file", IVM_NATIVE_WRAP_CONS(state, file_proto, _io_file));
	ivm_object_setSlot_r(mod, state, "remove", IVM_NATIVE_WRAP(state, _io_remove));

#if IO_USE_TERMIOS
	ivm_object_setSlot_r(mod, state, "kbhit", IVM_NATIVE_WRAP(state, _io_kbhit));
#endif

	ivm_object_setSlot_r(mod, state, "stdin", ivm_file_object_new(state, ivm_file_new_c(stdin)));
	ivm_object_setSlot_r(mod, state, "stdout", ivm_file_object_new(state, ivm_file_new_c(stdout)));
	ivm_object_setSlot_r(mod, state, "stderr", ivm_file_object_new(state, ivm_file_new_c(stderr)));

	return mod;
}
