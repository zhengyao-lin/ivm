#ifndef _IVM_PUB_ERR_H_
#define _IVM_PUB_ERR_H_

#include "io.h"
#include "sys.h"

#define IVM_ASSERT(cond, ...) \
	if (!cond) { \
		fprintf(IVM_STDERR, __VA_ARGS__); \
		fputc('\n', IVM_STDERR); \
		IVM_ABORT(); \
	}

#endif
