#ifndef _IVM_STD_COMPLEX_H_
#define _IVM_STD_COMPLEX_H_

#include <complex.h>

#include "pub/com.h"
#include "pub/type.h"

IVM_COM_HEADER

typedef struct {
	ivm_number_t v[2];
} ivm_complex_t;

#define ivm_complex_build(r, i) \
	((ivm_complex_t) { { (r), (i) } })

#define ivm_complex_real(c) \
	((c).v[0])

#define ivm_complex_imag(c) \
	((c).v[1])

IVM_INLINE
ivm_complex_t
ivm_complex_add(ivm_complex_t c1,
				ivm_complex_t c2)
{
	return ivm_complex_build(c1.v[0] + c2.v[0],
							 c1.v[1] + c2.v[1]);
}

IVM_INLINE
ivm_complex_t
ivm_complex_sub(ivm_complex_t c1,
				ivm_complex_t c2)
{
	return ivm_complex_build(c1.v[0] - c2.v[0],
							 c1.v[1] - c2.v[1]);
}

IVM_INLINE
ivm_complex_t
ivm_complex_mul(ivm_complex_t c1,
				ivm_complex_t c2)
{
	ivm_number_t r1 = c1.v[0],
				 r2 = c2.v[0],
				 i1 = c1.v[1],
				 i2 = c2.v[1];

	return ivm_complex_build(r1 * r2 - i1 * i2,
							 i1 * r2 + r1 * i2);
}

IVM_INLINE
ivm_complex_t
ivm_complex_div(ivm_complex_t c1,
				ivm_complex_t c2)
{
	ivm_number_t r1 = c1.v[0],
				 r2 = c2.v[0],
				 i1 = c1.v[1],
				 i2 = c2.v[1];

	return ivm_complex_build((r1 * r2 + i1 * i2) / (r2 * r2 + i2 * i2),
							 (i1 * r2 - r1 * i2) / (r2 * r2 + i2 * i2));
}

IVM_COM_END

#endif
