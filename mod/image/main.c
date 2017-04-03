#include <stdio.h>

#include "pub/type.h"
#include "pub/vm.h"
#include "pub/obj.h"
#include "pub/err.h"
#include "pub/inlines.h"

#include "std/string.h"
#include "std/mem.h"
#include "std/bmp.h"

#include "vm/native/native.h"
#include "vm/native/priv.h"

IVM_PRIVATE
ivm_int_t _type_uid;

#define IMAGE_TYPE_UID (&_type_uid)

#define IMAGE_ERROR_MSG_UNINIT_IMAGE							"uninitialized image data"
#define IMAGE_ERROR_MSG_CANNOT_OPEN_FILE(file)					"unable to open file \"%s\"", (file)
#define IMAGE_ERROR_MSG_UNKNOWN_FORMAT(format)					"unknown format \"%s\"", (format)
#define IMAGE_ERROR_MSG_FAIL_TO_FORMAT							"failed to format image"
#define IMAGE_ERROR_MSG_INDEX_OUT_OF_BOUNDARY(x, y)				"index (%ld, %ld) is out of boundary", (ivm_long_t)(x), (ivm_long_t)(y)

typedef struct {
	IVM_OBJECT_HEADER
	ivm_image_t *dat;
} ivm_image_object_t;

ivm_object_t *
ivm_image_object_new(ivm_vmstate_t *state, ivm_image_t *img)
{
	ivm_image_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), IVM_TPTYPE(state, IMAGE_TYPE_UID));

	ret->dat = img;

	if (img) {
		ivm_vmstate_addDesLog(state, IVM_AS_OBJ(ret));
	}

	return IVM_AS_OBJ(ret);
}

void
ivm_image_object_destructor(ivm_object_t *obj,
							ivm_vmstate_t *state)
{
	ivm_image_free(IVM_AS(obj, ivm_image_object_t)->dat);
	return;
}

void
ivm_image_object_cloner(ivm_object_t *obj,
						ivm_vmstate_t *state)
{
	ivm_image_object_t *img = IVM_AS(obj, ivm_image_object_t);

	if (img->dat) {
		img->dat = ivm_image_clone(img->dat);
		ivm_vmstate_addDesLog(state, obj);
	}

	return;
}

IVM_NATIVE_FUNC(_image_image)
{
	ivm_image_t *img;
	ivm_number_t width, height;

	if (NAT_ARGC() >= 2) {
		MATCH_ARG("nn", &width, &height);
		CHECK_OVERFLOW(width);
		CHECK_OVERFLOW(height);

		img = ivm_image_new(width, height, IVM_NULL);
		return ivm_image_object_new(NAT_STATE(), img);
	}

	CHECK_ARG_1_TP(IMAGE_TYPE_UID);
	return ivm_object_clone(NAT_ARG_AT(1), NAT_STATE());
}

IVM_NATIVE_FUNC(_image_image_width)
{
	ivm_image_object_t *img;

	CHECK_BASE_TP(IMAGE_TYPE_UID);

	img = IVM_AS(NAT_BASE(), ivm_image_object_t);
	RTM_ASSERT(img->dat, IMAGE_ERROR_MSG_UNINIT_IMAGE);

	return ivm_numeric_new(NAT_STATE(), ivm_image_width(img->dat));
}

IVM_NATIVE_FUNC(_image_image_height)
{
	ivm_image_object_t *img;

	CHECK_BASE_TP(IMAGE_TYPE_UID);

	img = IVM_AS(NAT_BASE(), ivm_image_object_t);
	RTM_ASSERT(img->dat, IMAGE_ERROR_MSG_UNINIT_IMAGE);

	return ivm_numeric_new(NAT_STATE(), ivm_image_height(img->dat));
}

IVM_NATIVE_FUNC(_image_image_encode)
{
	ivm_image_object_t *img;
	const ivm_string_t *enc, *file_s = IVM_NULL;
	const ivm_char_t *file;
	ivm_file_t *fp;
	ivm_stream_t *stream;
	ivm_bool_t suc;
	ivm_byte_t *buf;
	ivm_size_t buf_size;
	ivm_object_t *ret;

	CHECK_BASE_TP(IMAGE_TYPE_UID);
	MATCH_ARG("s*s", &enc, &file_s);

	img = IVM_AS(NAT_BASE(), ivm_image_object_t);

	if (!ivm_string_compareToRaw(enc, "bmp")) {
		if (file_s) {
			file = ivm_string_trimHead(file_s);
			fp = ivm_file_new(file, IVM_FMODE_WRITE_BINARY);
			RTM_ASSERT(fp, IMAGE_ERROR_MSG_CANNOT_OPEN_FILE(file));

			stream = ivm_file_stream_new(fp);
			suc = ivm_image_bmp_format(img->dat, stream);
			ivm_stream_free(stream);

			RTM_ASSERT(suc, IMAGE_ERROR_MSG_FAIL_TO_FORMAT);

			ivm_file_free(fp);

			ret = IVM_NONE(NAT_STATE());
		} else {
			stream = ivm_buffer_stream_new(IVM_NULL, 0);
			suc = ivm_image_bmp_format(img->dat, stream);
			
			if (!suc) {
				ivm_stream_free(stream);
				RTM_FATAL(IMAGE_ERROR_MSG_FAIL_TO_FORMAT);
			}

			buf = ivm_buffer_stream_getBuffer((ivm_buffer_stream_t *)stream);
			buf_size = ivm_buffer_stream_getSize((ivm_buffer_stream_t *)stream);
			
			ret = ivm_buffer_object_newCopy(NAT_STATE(), buf_size, buf);
			ivm_stream_free(stream);
		}
	} else {
		RTM_FATAL(IMAGE_ERROR_MSG_UNKNOWN_FORMAT(ivm_string_trimHead(enc)));
	}

	return ret;
}

IVM_NATIVE_FUNC(_image_image_get)
{
	ivm_image_object_t *img;
	ivm_pixel_t pix;
	ivm_number_t x, y;

	CHECK_BASE_TP(IMAGE_TYPE_UID);
	MATCH_ARG("nn", &x, &y);

	img = IVM_AS(NAT_BASE(), ivm_image_object_t);

	CHECK_OVERFLOW(x);
	CHECK_OVERFLOW(y);

	RTM_ASSERT(ivm_image_getPixel(img->dat, x, y, &pix), IMAGE_ERROR_MSG_INDEX_OUT_OF_BOUNDARY(x, y));

	return ivm_numeric_new(NAT_STATE(), pix);
}

IVM_NATIVE_FUNC(_image_image_set)
{
	ivm_image_object_t *img;
	ivm_number_t x, y;
	ivm_uint32_t pix;

	CHECK_BASE_TP(IMAGE_TYPE_UID);
	MATCH_ARG("nnu", &x, &y, &pix);

	img = IVM_AS(NAT_BASE(), ivm_image_object_t);

	CHECK_OVERFLOW(x);
	CHECK_OVERFLOW(y);

	RTM_ASSERT(ivm_image_setPixel(img->dat, x, y, pix),
			   IMAGE_ERROR_MSG_INDEX_OUT_OF_BOUNDARY(x, y));

	return IVM_NONE(NAT_STATE());
}

/* file -> img */
IVM_NATIVE_FUNC(_image_bmp_parse)
{
	const ivm_string_t *file;
	const ivm_char_t *msg, *path;
	ivm_image_t *img;
	ivm_stream_t *stream;
	ivm_file_t *fp;

	MATCH_ARG("s", &file);

	path = ivm_string_trimHead(file);
	fp = ivm_file_new(path, IVM_FMODE_READ_BINARY);
	RTM_ASSERT(fp, IMAGE_ERROR_MSG_CANNOT_OPEN_FILE(path));

	stream = ivm_file_stream_new(fp);

	img = ivm_image_bmp_parse(stream, &msg);

	ivm_stream_free(stream);
	ivm_file_free(fp);

	RTM_ASSERT(img, "%s", msg);

	return ivm_image_object_new(NAT_STATE(), img);
}

ivm_object_t *
ivm_mod_main(ivm_vmstate_t *state,
			 ivm_coro_t *coro,
			 ivm_context_t *context)
{
	ivm_object_t *mod = ivm_object_new(state);
	ivm_object_t *bmp_mod;
	ivm_object_t *img_proto;

	ivm_type_t _image_type = IVM_TPTYPE_BUILD(
		"image.image", sizeof(ivm_image_object_t),
		IVM_NATIVE_WRAP_C(state, _image_image),
		IMAGE_TYPE_UID,

		.des = ivm_image_object_destructor,
		.clone = ivm_image_object_cloner,
		.const_bool = IVM_TRUE
	);

	/* image.image */
	IVM_VMSTATE_REGISTER_TPTYPE(state, coro, &_image_type, ivm_image_object_new(state, IVM_NULL), {
		img_proto = _PROTO;

		ivm_object_setSlot_r(_PROTO, state, "width", IVM_NATIVE_WRAP(state, _image_image_width));
		ivm_object_setSlot_r(_PROTO, state, "height", IVM_NATIVE_WRAP(state, _image_image_height));
		ivm_object_setSlot_r(_PROTO, state, "encode", IVM_NATIVE_WRAP(state, _image_image_encode));

		ivm_object_setSlot_r(_PROTO, state, "get", IVM_NATIVE_WRAP(state, _image_image_get));
		ivm_object_setSlot_r(_PROTO, state, "set", IVM_NATIVE_WRAP(state, _image_image_set));
	});

	ivm_object_setSlot_r(mod, state, "image", IVM_NATIVE_WRAP_CONS(state, img_proto, _image_image));

	bmp_mod = ivm_object_new(state);
	ivm_object_setSlot_r(mod, state, "bmp", bmp_mod);

	ivm_object_setSlot_r(bmp_mod, state, "parse", IVM_NATIVE_WRAP(state, _image_bmp_parse));

	return mod;
}
