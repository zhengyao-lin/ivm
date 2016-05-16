#ifndef _IVM_MEM_MEM_H_
#define _IVM_MEM_MEM_H_

#include <stdlib.h>
#include <string.h>
#include "pub/type.h"

#define MEM_ALLOC(size, type)			((type)malloc(size))
#define MEM_ALLOC_INIT(size, type)		((type)calloc(1, (size)))
#define MEM_REALLOC(origin, size, type)	((type)realloc((origin), (size)))
#define MEM_INIT(p)						(memset((p), 0x0, sizeof(p)))
#define MEM_FREE(p)						(free(p))
#define MEM_COPY(dest, src, size)		(memcpy((dest), (src), (size)))

#endif
