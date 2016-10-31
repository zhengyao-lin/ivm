#ifndef _IVM_STD_BMP_H_
#define _IVM_STD_BMP_H_

#include "pub/type.h"
#include "pub/const.h"
#include "pub/com.h"

typedef ivm_uint32_t ivm_pixel_t;

typedef struct {
	ivm_size_t width;
	ivm_size_t height;

	ivm_pixel_t *dat;
} ivm_image_t;

ivm_image_t *
ivm_image_new(ivm_size_t width,
			  ivm_size_t height,
			  ivm_pixel_t *dat);

ivm_image_t *
ivm_image_clone(ivm_image_t *img);

#define ivm_image_width(img) ((img)->width)
#define ivm_image_height(img) ((img)->height)
#define ivm_image_pixels(img) ((img)->dat)

void
ivm_image_free(ivm_image_t *img);

ivm_bool_t
ivm_image_bmp_format(ivm_image_t *image,
					 ivm_file_t *output);

ivm_image_t *
ivm_image_bmp_parse_c(ivm_byte_t *dat,
					  ivm_size_t size,
					  const ivm_char_t **err);

ivm_image_t *
ivm_image_bmp_parse(ivm_file_t *fp,
					const ivm_char_t **err);

#endif
