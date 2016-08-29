#ifndef _IVM_MEM_MEM_H_
#define _IVM_MEM_MEM_H_

#include <stdlib.h>
#include <string.h>

#include "pub/com.h"
#include "pub/type.h"

IVM_COM_HEADER

extern ivm_size_t _mem_allocated;

IVM_INLINE
void *
mem_alloc_c(ivm_size_t size)
{
	_mem_allocated += size;
	return malloc(size);
}

IVM_INLINE
void *
mem_alloc_init_c(ivm_size_t size)
{
	_mem_allocated += size;
	return calloc(1, size);
}

#define MEM_ALLOC(size, type)			((type)mem_alloc_c(size))
#define MEM_ALLOC_INIT(size, type)		((type)mem_alloc_init_c(size))
#define MEM_REALLOC(origin, size, type)	((type)realloc((origin), (size)))
#define MEM_FREE(p)						free(p)

#define MEM_INIT(p, size)				(memset((p), 0x0, (size)))
#define MEM_COPY(dest, src, size)		memcpy((dest), (src), (size))
#define MEM_COMPARE(a, b, size)			memcmp((a), (b), (size))

#define mem_getAllocated()				_mem_allocated
#define mem_clearAllocated()			(_mem_allocated = 0)

IVM_COM_END

#endif
