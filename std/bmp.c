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

	IVM_MEMCHECK(ret);

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

ivm_image_t *
ivm_image_clone(ivm_image_t *img)
{
	ivm_size_t size;
	ivm_image_t *ret = STD_ALLOC(sizeof(*ret));

	IVM_MEMCHECK(ret);

	ret->width = img->width;
	ret->height = img->height;
	size = sizeof(*ret->dat) * ret->width * ret->height;
	ret->dat = STD_ALLOC(size);

	IVM_MEMCHECK(ret->dat);

	STD_MEMCPY(ret->dat, img->dat, size);

	return ret;
}

ivm_pixel_t
ivm_image_getPixel(ivm_image_t *img,
				   ivm_size_t x,
				   ivm_size_t y)
{
	ivm_size_t w = img->width, h = img->height;

	if (x >= w || y >= h) {
		return IVM_PIXEL_ILLEGAL;
	}
	
	return img->dat[y * w + x];
}

ivm_bool_t
ivm_image_setPixel(ivm_image_t *img,
				   ivm_size_t x,
				   ivm_size_t y,
				   ivm_pixel_t pix)
{
	ivm_size_t w = img->width, h = img->height;

	if (x >= w || y >= h) {
		return IVM_FALSE;
	}

	img->dat[y * w + x] = pix;

	return IVM_TRUE;
}

struct _bmp_info_t {
	ivm_uint32_t size;			// size of info struct

	ivm_uint32_t width;			// width(in pixels)
	ivm_uint32_t height;		// height(in pixels)

	ivm_uint16_t plane;			// usually 1

	ivm_uint16_t bpp;			// bits per pixel(1, 4, 8, 16, 24, 32)

	ivm_uint32_t compr;			// compression
	ivm_uint32_t compr_size;	// compressed(optional) size of the image data

	ivm_uint32_t resx;			// horizontal pixels per meter(resolution)
	ivm_uint32_t resy;			// vertical pixels per meter(resolution)

	ivm_uint32_t clr_used;		// color used(in the palette)
	ivm_uint32_t clr_imp;		// important color count
};

struct _bmp_header_t {
	ivm_uint32_t size;			// size of the whole file
	ivm_uint32_t reserv;		// == 0
	ivm_uint32_t ofs;			// header offset

	struct _bmp_info_t info;
};

ivm_bool_t
ivm_image_bmp_format(ivm_image_t *image,
					 ivm_stream_t *output)
{
	struct _bmp_header_t header;
	ivm_size_t width = ivm_image_width(image);
	ivm_size_t height = ivm_image_height(image);
	ivm_size_t pcount = width * height,
			   isize = pcount * 3, i;
	ivm_pixel_t *pixs = ivm_image_pixels(image), tmp;
	ivm_byte_t *dat, *cur;
	ivm_bool_t suc;

	IVM_PRIVATE const ivm_uint16_t htype = 0x4d42;
	
	header = (struct _bmp_header_t) {
		.size = sizeof(header) + isize,
		.reserv = 0,
		.ofs = sizeof(header),
		.info = (struct _bmp_info_t) {
			.size = sizeof(header.info),
			
			.width = width,
			.height = height,

			.plane = 1,

			.bpp = 24,

			.compr = 0,
			.compr_size = isize,

			.resx = 4,
			.resy = 4,

			.clr_used = 0,
			.clr_imp = 0
		}
	};

	dat = STD_ALLOC(isize);

	IVM_MEMCHECK(dat);

	for (i = 0, cur = dat;
		 i < pcount; i++, cur += 3) {
		// IVM_TRACE("%ld\n", i);
		tmp = pixs[i];
		cur[0] = tmp >> 16;
		cur[1] = tmp >> 8;
		cur[2] = tmp;
	}

	suc = ivm_stream_write(output, &htype, sizeof(htype), 1) == 1 &&
		  ivm_stream_write(output, &header, sizeof(header), 1) == 1 &&
		  ivm_stream_write(output, dat, 1, isize) == isize;

	STD_FREE(dat);

	return suc;
}

ivm_image_t *
ivm_image_bmp_parse(ivm_stream_t *dat,
					const ivm_char_t **err)
{
	const ivm_char_t *tmp_err = IVM_NULL;
	struct _bmp_header_t header;
	ivm_size_t i, dsize = -1;
	ivm_uchar_t tmp, tmpbuf[3];
	ivm_pixel_t *pixels = IVM_NULL, *cur;
	ivm_image_t *ret = IVM_NULL;
	ivm_uint16_t htype;

	if (ivm_stream_read(dat, &htype, sizeof(htype), 1) != 1 ||
		htype != 0x4d42) {
		tmp_err = "not a bmp file";
		goto ERROR;
	}

	if (ivm_stream_read(dat, &header, sizeof(header), 1) != 1) {
		tmp_err = "wrong header";
		goto ERROR;
	}

	if (header.ofs != sizeof(htype) + sizeof(header)) {
		tmp_err = "unsupported palette";
		goto ERROR;
	}

	dsize = header.size - sizeof(htype) - sizeof(header);

	switch (header.info.bpp) {
		case 1:
			pixels = STD_ALLOC(sizeof(*pixels) * dsize * 8);

			for (i = 0, cur = pixels;
				 i < dsize; i++, cur++) {

#define BIT_AT(n) (cur[(n) - 1] = !!(tmp & (1 << (8 - (n)))))

				// tmp = dat[i]; // unsigned

				if (ivm_stream_read(dat, &tmp, sizeof(tmp), 1) != 1) {
					tmp_err = "unexpected ending";
					goto ERROR;
				}

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

			break;

		case 24:
			pixels = STD_ALLOC(sizeof(*pixels) * (dsize / 3));

			for (i = 0, cur = pixels;
				 i < dsize; i += 3, cur++) {
				if (ivm_stream_read(dat, tmpbuf, sizeof(*tmpbuf), 3) != 3) {
					tmp_err = "unexpected ending";
					goto ERROR;
				}

				*cur = (tmpbuf[0] << 16) + (tmpbuf[1] << 8) + tmpbuf[2];
			}

			ret = ivm_image_new(header.info.width, header.info.height, pixels);

			break;

		case 32:
			pixels = STD_ALLOC(sizeof(*pixels) * dsize);
			
			if (ivm_stream_read(dat, pixels, sizeof(*pixels), dsize) != dsize) {
				tmp_err = "unexpected ending";
				goto ERROR;
			}

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

	if (pixels) {
		STD_FREE(pixels);
	}

	return IVM_NULL;

ERROR_END:
	
	return ret;
}
