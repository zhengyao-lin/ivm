#ifndef _IVM_PUB_ERR_H_
#define _IVM_PUB_ERR_H_

#include "pub/com.h"
#include "io.h"
#include "sys.h"

IVM_COM_HEADER

#define IVM_ASSERT(cond, ...) \
	if (!(cond)) { \
		fprintf(IVM_STDERR, "at %s: line %d: ", __FILE__, __LINE__); \
		fprintf(IVM_STDERR, __VA_ARGS__); \
		fputc('\n', IVM_STDERR); \
		IVM_ABORT(); \
	}

#define IVM_ERROR_MSG_FAILED_ALLOC_NEW(name)		("failed to allocate new room for new " name)
#define IVM_ERROR_MSG_OP_SLOT_OF_UNDEFINED(op)		(op " slot of undefined object")
#define IVM_ERROR_MSG_INSERT_CELL_TO_NON_SUC_CELL	("insert cell into non-successive cells")
#define IVM_ERROR_MSG_WRONG_FREE_HEAP				("free object with wrong heap")
#define IVM_ERROR_MSG_NO_SPARE_MEM					("no spare memory left in heap")
#define IVM_ERROR_MSG_CANNOT_FIND_OBJECT_IN_HEAP	("cannot find object in heap")
#define IVM_ERROR_MSG_BAD_OP_TABLE					("bad opcode table")
#define IVM_ERROR_MSG_BAD_OP						("bad opcode")
#define IVM_ERROT_MSG_RESET_CORO_ROOT				("cannot reset root of sleeping coroutine")
#define IVM_ERROR_MSG_INSUFFICIENT_STACK			("insufficient stack")
#define IVM_ERROR_MSG_BYTE_NOT_EQUAL_TO_CHAR		("size of byte is not equal to char")
#define IVM_ERROR_MSG_SIZE_EXCEEDS_BLOCK_SIZE		("size exceeds the block size")

IVM_COM_END

#endif
