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

void
ivm_image_free(ivm_image_t *img);

ivm_image_t *
ivm_image_bmp_parse(ivm_file_t *fp,
					const ivm_char_t **err);

#endif
