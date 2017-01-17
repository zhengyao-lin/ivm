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

/* buffer -> img */
IVM_NATIVE_FUNC(_image_bmp_parse)
{
	ivm_buffer_object_t *buf_obj;
	const ivm_char_t *msg;
	ivm_image_t *img;

	MATCH_ARG("b", &buf_obj);

	img = ivm_image_bmp_parse_c(ivm_buffer_object_getRaw(buf_obj), ivm_buffer_object_getSize(buf_obj), &msg);

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
		ivm_object_setSlot_r(_PROTO, state, "width", IVM_NATIVE_WRAP(state, _image_image_width));
		ivm_object_setSlot_r(_PROTO, state, "height", IVM_NATIVE_WRAP(state, _image_image_height));
	});

	bmp_mod = ivm_object_new(state);
	ivm_object_setSlot_r(mod, state, "bmp", bmp_mod);

	ivm_object_setSlot_r(bmp_mod, state, "parse", IVM_NATIVE_WRAP(state, _image_bmp_parse));

	return mod;
}
