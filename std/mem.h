#ifndef _IVM_STD_STD_H_
#define _IVM_STD_STD_H_

#include <stdlib.h>
#include <string.h>

#include "pub/com.h"
#include "pub/type.h"

IVM_COM_HEADER

extern ivm_size_t _std_allocated;

IVM_INLINE
void *
_std_alloc_c(ivm_size_t size)
{
	_std_allocated += size;
	return malloc(size);
}

IVM_INLINE
void *
_std_alloc_init_c(ivm_size_t size)
{
	_std_allocated += size;
	return calloc(1, size);
}

#define STD_ALLOC(size, type)			((type)_std_alloc_c(size))
#define STD_ALLOC_INIT(size, type)		((type)_std_alloc_init_c(size))
#define STD_REALLOC(origin, size, type)	((type)realloc((origin), (size)))
#define STD_FREE(p)						free(p)

#define STD_INIT(p, size)				(memset((p), 0x0, (size)))
#define STD_MEMCPY(dest, src, size)		memcpy((dest), (src), (size))
#define STD_MEMCMP(a, b, size)			memcmp((a), (b), (size))

IVM_COM_END

#endif
