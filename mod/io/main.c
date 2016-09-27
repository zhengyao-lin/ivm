#include <stdio.h>

#include "pub/type.h"
#include "pub/vm.h"
#include "pub/obj.h"
#include "pub/inlines.h"

#include "std/io.h"
#include "std/string.h"

#include "vm/native/native.h"
#include "vm/native/priv.h"

#define IO_FILE_TYPE_NAME "io.file"
#define IO_FILE_TYPE_MAGIC ((ivm_ptr_t)ivm_file_object_new)

#define IO_ERROR_MSG_FAILED_TO_OPEN_FILE(file)							"failed to open file '%s'", (file)
#define IO_ERROR_MSG_UNINIT_FILE_POINTER								"uninitialized file pointer"
#define IO_ERROR_MSG_FAILED_READ_FILE									"failed to read file"
#define IO_ERROR_MSG_TOO_LARGE_LENGTH(len, max)							"too large length(length %ld for the file of length %ld)", (ivm_long_t)(len), (ivm_long_t)(max)
#define IO_ERROR_MSG_FAILED_SET_POS(pos)								"failed to set read position %ld", (ivm_long_t)(pos)

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

	ivm_object_init(IVM_AS_OBJ(ret), IVM_TPTYPE(state, IO_FILE_TYPE_NAME));

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

IVM_NATIVE_FUNC(_io_open)
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

	CHECK_BASE_TP(IO_FILE_TYPE_MAGIC);

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
	ivm_number_t arg = -1, save_pos = 1;
	ivm_long_t flen, len;

	CHECK_BASE_TP(IO_FILE_TYPE_MAGIC);
	MATCH_ARG("*nn", &arg, &save_pos);

	fobj = IVM_AS(NAT_BASE(), ivm_file_object_t);
	CHECK_INIT_FP(fobj);

	flen = ivm_file_length(fobj->fp);
	len = ivm_list_realIndex(flen, arg) + 1;

	// IVM_TRACE("read: %ld in %ld\n", len, flen);

	RTM_ASSERT(len <= flen, IO_ERROR_MSG_TOO_LARGE_LENGTH(len, flen));

	cont = ivm_file_read_n(fobj->fp, len, save_pos);

	RTM_ASSERT(cont, IO_ERROR_MSG_FAILED_READ_FILE);

	ret = ivm_string_object_new_r(NAT_STATE(), cont);
	STD_FREE(cont);

	return ret;
}

IVM_NATIVE_FUNC(_io_file_len)
{
	ivm_file_object_t *fobj;

	CHECK_BASE_TP(IO_FILE_TYPE_MAGIC);

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

	CHECK_BASE_TP(IO_FILE_TYPE_MAGIC);

	fobj = IVM_AS(NAT_BASE(), ivm_file_object_t);
	CHECK_INIT_FP(fobj);

	cont = ivm_file_readAll(fobj->fp);
	RTM_ASSERT(cont, IO_ERROR_MSG_FAILED_READ_FILE);

	ret = IVM_AS(ivm_list_object_new(NAT_STATE(), 0), ivm_list_object_t);
	len = IVM_STRLEN(cont);

	for (head = i = cont, end = i + len;
		 i != end; i++) {
		if (*i == '\n') {
			ivm_list_object_push(
				ret, NAT_STATE(),
				ivm_string_object_new_rl(NAT_STATE(), head, IVM_PTR_DIFF(i, head, ivm_char_t))
			);
			head = i + 1;
		}
	}

	ivm_list_object_push(
		ret, NAT_STATE(),
		ivm_string_object_new_rl(NAT_STATE(), head, IVM_PTR_DIFF(i, head, ivm_char_t))
	);

	STD_FREE(cont);

	return IVM_AS_OBJ(ret);
}

IVM_NATIVE_FUNC(_io_file_seek)
{
	ivm_file_object_t *fobj;
	ivm_number_t pos = 0;

	CHECK_BASE_TP(IO_FILE_TYPE_MAGIC);
	MATCH_ARG("*n", &pos);

	fobj = IVM_AS(NAT_BASE(), ivm_file_object_t);
	CHECK_INIT_FP(fobj);

	RTM_ASSERT(ivm_file_setPos(fobj->fp, pos), IO_ERROR_MSG_FAILED_SET_POS(pos));

	return IVM_NONE(NAT_STATE());
}

IVM_NATIVE_FUNC(_io_file_cur)
{
	ivm_file_object_t *fobj;

	CHECK_BASE_TP(IO_FILE_TYPE_MAGIC);

	fobj = IVM_AS(NAT_BASE(), ivm_file_object_t);
	CHECK_INIT_FP(fobj);

	return ivm_numeric_new(NAT_STATE(), ivm_file_curPos(fobj->fp));
}

IVM_PRIVATE
ivm_type_t
_io_file_type = IVM_TPTYPE_BUILD(
	file, sizeof(ivm_file_object_t),
	IO_FILE_TYPE_MAGIC,

	.des = ivm_file_object_destructor,
	.const_bool = IVM_TRUE
);

ivm_object_t *
ivm_mod_main(ivm_vmstate_t *state,
			 ivm_coro_t *coro,
			 ivm_context_t *context)
{
	ivm_object_t *mod = ivm_object_new(state);
	ivm_object_t *file_proto;

	/* io.file */
	IVM_VMSTATE_REGISTER_TPTYPE(state, coro, IO_FILE_TYPE_NAME, &_io_file_type, {
		file_proto = ivm_file_object_new(state, IVM_NULL);
		ivm_type_setProto(_TYPE, file_proto);
		ivm_object_setProto(file_proto, state, ivm_vmstate_getTypeProto(state, IVM_OBJECT_T));

		ivm_object_setSlot_r(file_proto, state, "close", IVM_NATIVE_WRAP(state, _io_file_close));
		ivm_object_setSlot_r(file_proto, state, "read", IVM_NATIVE_WRAP(state, _io_file_read));
		ivm_object_setSlot_r(file_proto, state, "len", IVM_NATIVE_WRAP(state, _io_file_len));
		ivm_object_setSlot_r(file_proto, state, "lines", IVM_NATIVE_WRAP(state, _io_file_lines));
		ivm_object_setSlot_r(file_proto, state, "seek", IVM_NATIVE_WRAP(state, _io_file_seek));
		ivm_object_setSlot_r(file_proto, state, "cur", IVM_NATIVE_WRAP(state, _io_file_cur));
	});

	/* io */
	ivm_object_setSlot_r(mod, state, "open", IVM_NATIVE_WRAP(state, _io_open));
	ivm_object_setSlot_r(mod, state, "file", file_proto);

	return mod;
}