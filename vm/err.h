#ifndef _IVM_VM_ERR_H_
#define _IVM_VM_ERR_H_

#include "pub/com.h"
#include "pub/type.h"

#include "std/io.h"
#include "std/sys.h"

IVM_COM_HEADER

#define IVM_ASSERT(cond, ...) \
	if (!(cond)) { \
		fprintf(IVM_STDERR, "at %s: line %d: ", __FILE__, __LINE__); \
		fprintf(IVM_STDERR, __VA_ARGS__); \
		fputc('\n', IVM_STDERR); \
		IVM_ABORT(); \
	}

#define IVM_ASSERT_S(cond) \
	if (!(cond)) { \
		fprintf(IVM_STDERR, "at %s: line %d: assertion failed: %s\n", \
				__FILE__, __LINE__, #cond); \
		IVM_ABORT(); \
	}

#define IVM_FATAL(...) \
	fprintf(IVM_STDERR, "at %s: line %d: ", __FILE__, __LINE__); \
	fprintf(IVM_STDERR, __VA_ARGS__); \
	fputc('\n', IVM_STDERR); \
	IVM_ABORT();

#define IVM_ERROR_MSG_FAILED_ALLOC_NEW(name)			("failed to allocate new room for new " name)
#define IVM_ERROR_MSG_OP_SLOT_OF_UNDEFINED(op)			(op " slot of undefined object")
#define IVM_ERROR_MSG_INSERT_CELL_TO_NON_SUC_CELL		("insert cell into non-successive cells")
#define IVM_ERROR_MSG_WRONG_FREE_HEAP					("free object with wrong heap")
#define IVM_ERROR_MSG_NO_SPARE_MEM						("no spare memory left in heap")
#define IVM_ERROR_MSG_CANNOT_FIND_OBJECT_IN_HEAP		("cannot find object in heap")
#define IVM_ERROR_MSG_BAD_OPCODE_TABLE					("bad opcode table")
#define IVM_ERROR_MSG_BAD_OPCODE						("bad opcode")
#define IVM_ERROR_MSG_RESET_CORO_ROOT					("cannot reset root of sleeping coroutine")
#define IVM_ERROR_MSG_INSUFFICIENT_STACK(req, left)		"insufficient stack(require at least %d element(s), %zd left in stack)", (ivm_int_t)(req), (left)
#define IVM_ERROR_MSG_BYTE_NOT_EQUAL_TO_CHAR			("size of byte is not equal to char")
#define IVM_ERROR_MSG_ILLEGAL_ALLOC_SIZE(size)			"illegal alloc size %ld", (size)
#define IVM_ERROR_MSG_NOT_TYPE(t1, t2)					"expecting type <" t1 "> instead of <%s>", (t2)
#define IVM_ERROR_MSG_NULL_PTR(name)					("null pointer given to " name)
#define IVM_ERROR_MSG_TOO_SMALL_VALUE_FOR(name, val)	"'%ld' is too small for " name, (val)
#define IVM_ERROR_MSG_NO_UNIOP_FOR(op, t)				"the unary operation %s of <%s> is not defined", (op), (t)
#define IVM_ERROR_MSG_NO_BINOP_FOR(t1, op, t2)			"the binary operation of <%s> %s <%s> is not defined", (t1), (op), (t2)
#define IVM_ERROR_MSG_UNEXPECTED_ARG_TYPE(t)			"unexpected param type %c", (t)
#define IVM_ERROR_MSG_PUSH_DEAD_CORO_STACK				"cannot push object to dead coro"
#define IVM_ERROR_MSG_SERIALIZE_CACHED_EXEC				"cannot serialize cached executable"
#define IVM_ERROR_MSG_UNEXPECTED_INSTR_ARG_CACHE		"unexpected instruction string operand cache(not appear in the string pool)"
#define IVM_ERROR_MSG_FILE_FORMAT_ERR(file, format)		"wrong file format of file %s, expecting %s file", (file), (format)
#define IVM_ERROR_MSG_ILLEGAL_STRING_LEN(len, max)		"illegal string len %lu(expect less than %lu)", (ivm_ulong_t)(len), (ivm_ulong_t)(max)

IVM_COM_END

#endif
