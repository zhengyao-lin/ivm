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
	ivm_size_t size;

	IVM_MEMCHECK(ret);

	ret->width = width;
	ret->height = height;

	if (dat) {
		ret->dat = dat;
	} else {
		size = width * height;
		ret->dat = STD_ALLOC_INIT(sizeof(*ret->dat) * size);
		IVM_MEMCHECK(ret->dat);
	}

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

ivm_bool_t
ivm_image_getPixel(ivm_image_t *img,
				   ivm_size_t x,
				   ivm_size_t y,
				   ivm_pixel_t *ret)
{
	ivm_size_t w = img->width, h = img->height;

	if (x >= w || y >= h) {
		return IVM_FALSE;
	}
	
	*ret = img->dat[y * w + x];

	return IVM_TRUE;
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

struct _bmp_info_v4_pad_t {
	ivm_uint32_t rmask;			// mask identifying bits of red component
	ivm_uint32_t gmask;			// mask identifying bits of green component
	ivm_uint32_t bmask;			// mask identifying bits of blue component
	ivm_uint32_t amask;			// mask identifying bits of alpha component
	ivm_uint32_t cs_type;		// color space type
	ivm_uint32_t redx;			// x coordinate of red endpoint
	ivm_uint32_t redy;			// y coordinate of red endpoint
	ivm_uint32_t redz;			// z coordinate of red endpoint
	ivm_uint32_t greenx;		// x coordinate of green endpoint
	ivm_uint32_t greeny;		// y coordinate of green endpoint
	ivm_uint32_t greenz;		// z coordinate of green endpoint
	ivm_uint32_t bluex;			// x coordinate of blue endpoint
	ivm_uint32_t bluey;			// y coordinate of blue endpoint
	ivm_uint32_t bluez;			// z coordinate of blue endpoint
	ivm_uint32_t gamma_red;		// gamma red coordinate scale value
	ivm_uint32_t gamma_green;	// gamma green coordinate scale value
	ivm_uint32_t gamma_blue;	// gamma blue coordinate scale value
};

ivm_bool_t
ivm_image_bmp_format(ivm_image_t *image,
					 ivm_stream_t *output)
{
	struct _bmp_header_t header;
	ivm_size_t width = ivm_image_width(image);
	ivm_size_t height = ivm_image_height(image);
	ivm_size_t pcount = width * height,
			   isize = pcount * 4, i;
	ivm_pixel_t *pixs = ivm_image_pixels(image), tmp;
	ivm_byte_t *dat, *cur;
	ivm_bool_t suc;

	const ivm_size_t hsize = sizeof(header) + 2; // header size

	IVM_PRIVATE const ivm_uint16_t htype = 0x4d42;
	
	header = (struct _bmp_header_t) {
		.size = hsize + isize,
		.reserv = 0,
		.ofs = hsize,
		.info = (struct _bmp_info_t) {
			.size = sizeof(header.info),
			
			.width = width,
			.height = height,

			.plane = 1,

			.bpp = 32,

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
		 i < pcount; i++, cur += 4) {
		// IVM_TRACE("%ld\n", i);
		tmp = pixs[i];
		// *cur = tmp;
		cur[0] = tmp >> 24;
		cur[1] = tmp >> 16;
		cur[2] = tmp >> 8;
		cur[3] = tmp;

		// IVM_TRACE("final: %u\n", pixs[i]);
	}

	suc = ivm_stream_write(output, &htype, sizeof(htype), 1) == 1 &&
		  ivm_stream_write(output, &header, sizeof(header), 1) == 1 &&
		  ivm_stream_write(output, dat, sizeof(*dat), isize) == isize;

	STD_FREE(dat);

	return suc;
}

ivm_byte_t * // decoded data
_bmp_rle8_decode(ivm_byte_t *cdat,
				 ivm_size_t size,
				 ivm_size_t *osize,	// output size
				 const ivm_char_t **err)
{
	ivm_stream_t *bufs = ivm_buffer_stream_new(IVM_NULL, 0);
	ivm_buffer_stream_t *tmp_bufs;
	ivm_byte_t *cur, *end, *ret, tmp;
	ivm_int_t i;

	const ivm_char_t *tmp_err = IVM_NULL;

	if (size % 2) {
		tmp_err = "RLE8: wrong alignment";
		goto RERROR;
	}

	for (cur = cdat, end = cur + size;
		 cur != end; cur += 2) {
		if (*cur) {
			tmp = cur[1];
			ivm_byte_t buf[*cur]; // <= 256

			// IVM_TRACE("comp: %d x %d\n", *cur, tmp);

			for (i = 0; i < *cur; i++) {
				buf[i] = tmp;
			}

			ivm_stream_write(bufs, buf, sizeof(*buf), *cur);
		} else switch (*(cur + 1)) {
			case 1:
				goto RERROR_END;
			case 0:
				// IVM_TRACE("%d\n", IVM_PTR_DIFF(cur, cdat, ivm_byte_t));
				break;
			default:
				// IVM_TRACE("esc: %d\n", *(cur + 2));
				tmp_err = "RLE8: unsupported escape instruction";
				goto RERROR;
		}
	}

goto RERROR_END;
RERROR:;

	if (tmp_err) {
		*err = tmp_err;
	}

	ivm_stream_free(bufs);

	return IVM_NULL;

RERROR_END:;

	tmp_bufs = (ivm_buffer_stream_t *)bufs;

	*osize = ivm_buffer_stream_getSize(tmp_bufs);
	
	ret = STD_ALLOC(sizeof(*ret) * *osize);
	IVM_MEMCHECK(ret);

	STD_MEMCPY(ret, ivm_buffer_stream_getBuffer(tmp_bufs), sizeof(*ret) * *osize);

	ivm_stream_free(bufs);

	return ret;
}

#define _BMP_COMP_RLE8 1
#define _BMP_COMP_RLE4 2

IVM_INLINE
ivm_uint32_t
_get_ofs(ivm_uint32_t mask)
{
	ivm_int_t i = 0;
	if (!mask) return 0;
	while (!(mask & 1)) mask >>= 1, i++;
	return i;
}

IVM_INLINE
ivm_pixel_t
_mix_rgba(ivm_uint32_t r,
		  ivm_uint32_t g,
		  ivm_uint32_t b,
		  ivm_uint32_t a)
{
	return
		((r & 0xff) << 24) |
		((g & 0xff) << 16) |
		((b & 0xff) << 8) |
		(a & 0xff);
}

ivm_image_t *
ivm_image_bmp_parse(ivm_stream_t *dat,
					const ivm_char_t **err)
{
	const ivm_char_t *tmp_err = IVM_NULL;
	struct _bmp_header_t header;
	ivm_size_t i, dsize = -1, psize;
	struct _bmp_info_v4_pad_t v4info;
	ivm_bool_t usev4 = IVM_FALSE; // use v4 version of bmp format
	
	ivm_pixel_t *pixels = IVM_NULL,
				*clr_table = IVM_NULL, *cur;

	ivm_image_t *ret = IVM_NULL;
	ivm_uint16_t htype;

	ivm_uint32_t pr, pg, pb, pa, // tmp value for rgba
				 rofs, gofs, bofs, aofs; // offset of rgba(in their masks)

	ivm_byte_t *pdat = IVM_NULL, tmp, *tmpbuf;

	if (ivm_stream_read(dat, &htype, sizeof(htype), 1) != 1 ||
		htype != 0x4d42) {
		tmp_err = "not a bmp file";
		goto RERROR;
	}

	if (ivm_stream_read(dat, &header, sizeof(header), 1) != 1) {
		tmp_err = "wrong header";
		goto RERROR;
	}

	if (header.info.size > sizeof(header.info)) {
		// ignore bmp v4 header
		usev4 = IVM_TRUE;
		if (ivm_stream_read(dat, &v4info, sizeof(v4info), 1) != 1) {
			tmp_err = "wrong header";
			goto RERROR;
		}
	}

	// IVM_TRACE("%d\n", header.info.clr_used);

	if (header.ofs != sizeof(htype) + sizeof(header)) {
		// read color table

		clr_table = STD_ALLOC(sizeof(*clr_table) * header.info.clr_used);
		IVM_MEMCHECK(clr_table);

		if (ivm_stream_read(dat, clr_table, sizeof(*clr_table), header.info.clr_used)
			!= header.info.clr_used) {
			tmp_err = "unexpected ending";
			goto RERROR;
		}

		if (usev4) {
			// re-mix rgba
			// IVM_TRACE("rmask: %d\n", v4info.rmask);

			rofs = _get_ofs(v4info.rmask);
			gofs = _get_ofs(v4info.gmask);
			bofs = _get_ofs(v4info.bmask);
			aofs = _get_ofs(v4info.amask);

			for (i = 0; i < header.info.clr_used; i++) {
				pr = (clr_table[i] & v4info.rmask) >> rofs;
				pg = (clr_table[i] & v4info.gmask) >> gofs;
				pb = (clr_table[i] & v4info.bmask) >> bofs;
				pa = (clr_table[i] & v4info.amask) >> aofs;
				clr_table[i] = _mix_rgba(pr, pg, pb, pa);
				// IVM_TRACE("color table: %d: %d, %d, %d, %d\n", clr_table[i], pr, pg, pb, pa);
			}
		}

		// IVM_TRACE("clr: %d\n", sizeof(htype) + sizeof(header));

		// tmp_err = "unsupported color table";
		// goto RERROR;
	}

	dsize = header.size - header.ofs; // data size
	psize = header.info.width * header.info.height; // picture size

	// read image data
	pdat = STD_ALLOC(sizeof(*pdat) * dsize);
	IVM_MEMCHECK(pdat);
	if (ivm_stream_read(dat, pdat, sizeof(*pdat), dsize) != dsize) {
		tmp_err = "unexpected ending";
		goto RERROR;
	}

	/*
	IVM_TRACE("bitmap size: %d\n", header.info.compr_size);
	IVM_TRACE("hsize: %d\n", header.info.size);
	IVM_TRACE("ofs: %d\n", header.ofs);
	IVM_TRACE("clr: %d\n", header.info.clr_used);
	IVM_TRACE("clr_imp: %d\n", header.info.clr_imp);
	*/

	// check compression
	switch (header.info.compr) {
		case _BMP_COMP_RLE8:
			tmpbuf = _bmp_rle8_decode(pdat, dsize, &dsize, &tmp_err);
			if (!tmpbuf) {
				goto RERROR;
			}

			STD_FREE(pdat);
			pdat = tmpbuf;

			// IVM_TRACE("after compression: %d\n", dsize);

			break;

		case 0: break; // no compression
		default:
			tmp_err = "unsupported compression";
			goto RERROR;
	}

	// check enough image data
	if (!header.info.compr &&
		dsize * 8 / header.info.bpp < psize) {
		tmp_err = "no enough data";
		goto RERROR;
	}

	switch (header.info.bpp) {
		case 1:
			if (!clr_table || header.info.clr_used < 2) {
				tmp_err = "lack color table";
				goto RERROR;
			}

			pixels = STD_ALLOC(sizeof(*pixels) * psize);
			IVM_MEMCHECK(pixels);

			for (i = 0, cur = pixels;
				 i < dsize; i++, cur += 8) {

#define BIT_AT(n) (cur[(n) - 1] = clr_table[!!(tmp & (1 << (8 - (n))))])

				tmp = pdat[i]; // unsigned

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

		case 8:
			if (!clr_table || header.info.clr_used < 256) {
				tmp_err = "lack color table";
				goto RERROR;
			}

			pixels = STD_ALLOC(sizeof(*pixels) * psize);
			IVM_MEMCHECK(pixels);

			for (i = 0, cur = pixels;
				 i < psize; i++, cur++) {
				tmp = pdat[i];
				*cur = clr_table[tmp];
				// IVM_TRACE("pix: %d\n", tmp);
			}

			ret = ivm_image_new(header.info.width, header.info.height, pixels);

			break;

		case 24:
			if (dsize % 3) {
				tmp_err = "wrong alignment";
				goto RERROR;
			}

			pixels = STD_ALLOC(sizeof(*pixels) * psize);
			IVM_MEMCHECK(pixels);

			// low  ->  high
			// -------------
			// | R | G | B |
			// -------------
			for (i = 0, cur = pixels;
				 i < psize * 3; i += 3, cur++) {
				tmpbuf = pdat + i;
				*cur = (tmpbuf[0] << 24) | (tmpbuf[1] << 16) | (tmpbuf[2] << 8);
				// IVM_TRACE("pix: %u\n", *cur);
			}

			ret = ivm_image_new(header.info.width, header.info.height, pixels);

			break;

		case 32:
			if (dsize % 4) {
				tmp_err = "wrong alignment";
				goto RERROR;
			}

			pixels = STD_ALLOC(sizeof(*pixels) * psize);
			IVM_MEMCHECK(pixels);

			// low    ->    high
			// -----------------
			// | R | G | B | A |
			// -----------------
			for (i = 0, cur = pixels;
				 i < psize * 4; i += 4, cur++) {
				tmpbuf = pdat + i;
				*cur = (tmpbuf[0] << 24) | (tmpbuf[1] << 16) | (tmpbuf[2] << 8) | tmpbuf[3];
			}

			ret = ivm_image_new(header.info.width, header.info.height, pixels);
			
			break;

		default:
			tmp_err = "unsupported bit count per pixel";
			goto RERROR;
	}

goto RERROR_END;
RERROR:;
	
	if (err) {
		*err = tmp_err;
	}

	if (pixels) {
		STD_FREE(pixels);
	}

	if (clr_table) {
		STD_FREE(clr_table);
	}

	if (pdat) {
		STD_FREE(pdat);
	}

	return IVM_NULL;

RERROR_END:

	if (clr_table) {
		STD_FREE(clr_table);
	}

	if (pdat) {
		STD_FREE(pdat);
	}
	
	return ret;
}
