#define IVM_DEBUG 1

#include "pub/err.h"

#include "io.h"
#include "bmp.h"
#include "mem.h"

ivm_image_t *
ivm_image_new(ivm_size_t width,
			  ivm_size_t height,
			  ivm_pixel_t *dat)
{
	ivm_image_t *ret = STD_ALLOC(sizeof(*ret));

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("image"));

	ret->width = width;
	ret->height = height;
	ret->dat = dat;

	return ret;
}

void
ivm_image_free(ivm_image_t *img)
{
	if (img) {
		STD_FREE(img->dat);
		STD_FREE(img);
	}

	return;
}

struct _bmp_info_t {
	ivm_uint32_t size;

	ivm_uint32_t width;
	ivm_uint32_t height;

	ivm_uint16_t plane;

	ivm_uint16_t bpp; // bits per pixel(1, 4, 8, 16, 24, 32)

	ivm_uint32_t compr; // compression
	ivm_uint32_t compr_size; // compressed size of the image data

	ivm_uint32_t resx; // horizontal pixels per meter(resolution)
	ivm_uint32_t resy; // vertical pixels per meter(resolution)

	ivm_uint32_t clr_used; // color used(in the palette)
	ivm_uint32_t clr_imp; // important color count
} IVM_NOALIGN;

struct _bmp_header_t {
	ivm_uint16_t type;
	ivm_uint32_t size;
	ivm_uint32_t reserv; // == 0
	ivm_uint32_t ofs; // header offset

	struct _bmp_info_t info;
} IVM_NOALIGN;

ivm_image_t *
ivm_image_bmp_parse(ivm_file_t *fp,
					const ivm_char_t **err)
{
	const ivm_char_t *tmp_err = IVM_NULL;
	struct _bmp_header_t header;
	ivm_size_t i, dsize;
	ivm_byte_t *dat = IVM_NULL;
	ivm_uchar_t tmp;
	ivm_pixel_t *pixels, *cur;
	ivm_image_t *ret = IVM_NULL;

	// IVM_TRACE("header size: %ld\n", sizeof(header));

	if (!ivm_file_read(fp, &header, sizeof(header), 1)) {
		tmp_err = "bad header";
		goto ERROR;
	}

	if (header.type != 0x4d42) {
		tmp_err = "not a bmp file";
		goto ERROR;
	}

	if (header.ofs != sizeof(header)) {
		tmp_err = "do not support palette";
		goto ERROR;
	}

	if (!header.info.compr_size) {
		dsize = header.info.compr_size = header.size = sizeof(header);
	}

	/*
	IVM_TRACE("header offset: %d\n", header.ofs);
	IVM_TRACE("file size: %d\n", header.size);
	IVM_TRACE("data size: %d\n", dsize);

	IVM_TRACE("width: %d\n", header.info.width);
	IVM_TRACE("height: %d\n", header.info.height);
	IVM_TRACE("bits per pixel: %d\n", header.info.bpp);

	IVM_TRACE("compression: %d\n", header.info.compr);
	IVM_TRACE("image size: %d\n", header.info.compr_size);

	IVM_TRACE("resolution x: %d\n", header.info.resx);
	IVM_TRACE("resolution y: %d\n", header.info.resy);
	*/

	dat = STD_ALLOC(dsize);

	if (ivm_file_read(fp, dat, sizeof(*dat), dsize) != dsize) {
		tmp_err = "unexpected file ending";
		goto ERROR;
	}

	switch (header.info.bpp) {
		case 1:
			pixels = STD_ALLOC(sizeof(*pixels) * dsize * 8);

			for (i = 0, cur = pixels;
				 i < dsize; i++, cur++) {

#define BIT_AT(n) (cur[(n) - 1] = !!(tmp & (1 << (8 - (n)))))

				tmp = dat[i]; // unsigned

				BIT_AT(1);
				BIT_AT(2);
				BIT_AT(3);
				BIT_AT(4);
				BIT_AT(5);
				BIT_AT(6);
				BIT_AT(7);
				BIT_AT(8);

#undef BIT_AT

			}

			ret = ivm_image_new(header.info.width, header.info.height, pixels);

			STD_FREE(dat);

			break;

		case 24:
			pixels = STD_ALLOC(sizeof(*pixels) * (dsize / 3));

			for (i = 0, cur = pixels;
				 i < dsize; i += 3, cur++) {
				*cur = (dat[i] << 16) + (dat[i + 1] << 8) + dat[i + 2];
			}

			ret = ivm_image_new(header.info.width, header.info.height, pixels);
			STD_FREE(dat);

			break;

		case 32:
			ret = ivm_image_new(header.info.width, header.info.height, (ivm_pixel_t *)dat);
			break;

		default:
			tmp_err = "unsupported bit count per pixel";
			goto ERROR;
	}

goto ERROR_END;
ERROR:;
	
	if (err) {
		*err = tmp_err;
	}

	STD_FREE(dat);

	return IVM_NULL;

ERROR_END:
	
	return ret;
}
