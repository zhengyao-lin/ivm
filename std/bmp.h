#ifndef _IVM_STD_BMP_H_
#define _IVM_STD_BMP_H_

#include "pub/type.h"
#include "pub/const.h"
#include "pub/com.h"

typedef ivm_uint32_t ivm_pixel_t;

#define IVM_PIXEL_ILLEGAL -1

typedef struct {
	ivm_size_t width;
	ivm_size_t height;

	ivm_pixel_t *dat;
} ivm_image_t;

ivm_image_t *
ivm_image_new(ivm_size_t width,
			  ivm_size_t height,
			  ivm_pixel_t *dat);

void
ivm_image_free(ivm_image_t *img);

ivm_image_t *
ivm_image_clone(ivm_image_t *img);

/**
 * picture:
 *   0 1 2 3 4 -- x
 * 0 a b c d e
 * 1 f g h i j
 * 2 k l m n o
 * 3 p q r s t
 * 4 u v w x y
 * |
 * y
 *
 * in memory: abcdefghijklmnopqrstuvwxy
 */
ivm_pixel_t /* return IVM_PIXEL_ILLEGAL if out of boundary */
ivm_image_getPixel(ivm_image_t *img,
				   ivm_size_t x,
				   ivm_size_t y);

ivm_bool_t /* if out of boundary */
ivm_image_setPixel(ivm_image_t *img,
				   ivm_size_t x,
				   ivm_size_t y,
				   ivm_pixel_t pix);

#define ivm_image_width(img) ((img)->width)
#define ivm_image_height(img) ((img)->height)
#define ivm_image_pixels(img) ((img)->dat)

ivm_bool_t
ivm_image_bmp_format(ivm_image_t *image,
					 ivm_stream_t *output);

ivm_image_t *
ivm_image_bmp_parse(ivm_stream_t *dat,
					const ivm_char_t **err);

#endif
