#ifndef _IVM_PUB_ERR_H_
#define _IVM_PUB_ERR_H_

#include "io.h"
#include "sys.h"

#define IVM_ASSERT(cond, ...) \
	if (!cond) { \
		fprintf(IVM_STDERR, "at %s: line %d: ", __FILE__, __LINE__); \
		fprintf(IVM_STDERR, __VA_ARGS__); \
		fputc('\n', IVM_STDERR); \
		IVM_ABORT(); \
	}

#define IVM_ERROR_MSG_FAILED_ALLOC_NEW(name)		("failed to allocate new room for new " name)
#define IVM_ERROR_MSG_OP_SLOT_OF_UNDEFINED(op)		(op " slot of undefined object")
#define IVM_ERROR_MSG_INSERT_CELL_TO_NON_SUC_CELL	("insert cell into non-successive cells")

#endif
