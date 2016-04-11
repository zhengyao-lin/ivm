#ifndef _IVM_MEM_MEM_H_
#define _IVM_MEM_MEM_H_

#include <stdlib.h>
#include "pub/type.h"

#define MEM_ALLOC(size)					(malloc(size))
#define MEM_ALLOC_INIT(size)			(calloc(1, (size)))
#define MEM_REALLOC(origin, size)		(realloc((origin), (size)))
#define MEM_INIT(p)						(memset((p), 0x0, sizeof(p)))
#define MEM_FREE(p)						(free(p))

#endif
