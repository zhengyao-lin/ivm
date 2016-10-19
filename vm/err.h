#ifndef _IVM_VM_ERR_H_
#define _IVM_VM_ERR_H_

#include "pub/com.h"
#include "pub/type.h"

#include "std/io.h"
#include "std/sys.h"

IVM_COM_HEADER

#define IVM_ERROR(...) \
	fprintf(IVM_STDERR, "at %s: line %d: ", __FILE__, __LINE__); \
	fprintf(IVM_STDERR, __VA_ARGS__); \
	fputc('\n', IVM_STDERR);

#define IVM_MEMCHECK(ptr) \
	if (!ptr) { \
		fprintf(IVM_STDERR, "at %s: line %d: memory error\n", __FILE__, __LINE__); \
		IVM_ABORT(); \
	}

#ifdef IVM_DEBUG

	#define IVM_ASSERT(cond, ...) \
		if (IVM_UNLIKELY(!(cond))) { \
			IVM_ERROR(__VA_ARGS__); \
			IVM_ABORT(); \
		}

	#define IVM_ASSERT_S(cond) \
		if (IVM_UNLIKELY(!(cond))) { \
			fprintf(IVM_STDERR, "at %s: line %d: assertion failed: %s\n", \
					__FILE__, __LINE__, #cond); \
			IVM_ABORT(); \
		}

	#define IVM_FATAL(...) \
		IVM_ERROR(__VA_ARGS__); \
		IVM_ABORT();

#else

	#define IVM_ASSERT(cond, ...) (cond)
	#define IVM_ASSERT_S(cond) (cond)
	#define IVM_FATAL(...)

#endif

#define IVM_ERROR_MSG_UNKNOWN_ERROR								("unknown error")
#define IVM_ERROR_MSG_FAILED_ALLOC_NEW(name)					("failed to allocate new memory for new " name)
#define IVM_ERROR_MSG_FAILED_ALLOC								("failed to allocate new memory")
#define IVM_ERROR_MSG_OP_NONE(op)								("cannot " op " none object")
#define IVM_ERROR_MSG_OP_SLOT_OF_NONE(name)						"cannot get/set/del slot '%s' of none object", (name)
#define IVM_ERROR_MSG_INSERT_CELL_TO_NON_SUC_CELL				("insert cell into non-successive cells")
#define IVM_ERROR_MSG_WRONG_FREE_HEAP							("free object with wrong heap")
#define IVM_ERROR_MSG_NO_SPARE_MEM								("no spare memory left in heap")
#define IVM_ERROR_MSG_CANNOT_FIND_OBJECT_IN_HEAP				("cannot find object in heap")
#define IVM_ERROR_MSG_BAD_OPCODE_TABLE							("bad opcode table")
#define IVM_ERROR_MSG_BAD_OPCODE								("bad opcode")
#define IVM_ERROR_MSG_RESET_CORO_ROOT							("cannot reset root of sleeping coroutine")
#define IVM_ERROR_MSG_INSUFFICIENT_STACK(req, left)				"insufficient stack(require at least %d element(s), %ld left in stack)", (ivm_int_t)(req), (left)
#define IVM_ERROR_MSG_BYTE_NOT_EQUAL_TO_CHAR					("size of byte is not equal to char")
#define IVM_ERROR_MSG_ILLEGAL_ALLOC_SIZE(size)					"illegal alloc size %ld", (size)
#define IVM_ERROR_MSG_NOT_TYPE(t1, t2)							"expecting type <" t1 "> instead of <%s>", (t2)
#define IVM_ERROR_MSG_UNABLE_TO_INVOKE(t)						"unable to invoke object of type <%s>", (t)
#define IVM_ERROR_MSG_NULL_PTR(name)							("null pointer given to " name)
#define IVM_ERROR_MSG_TOO_SMALL_VALUE_FOR(name, val)			"'%ld' is too small for " name, (val)
#define IVM_ERROR_MSG_NO_UNIOP_FOR(op, t)						"the unary operation %s of <%s> is not defined", (op), (t)
#define IVM_ERROR_MSG_NO_BINOP_FOR(t1, op, t2)					"the binary operation of <%s> %s <%s> is not defined", (t1), (op), (t2)
#define IVM_ERROR_MSG_UNEXPECTED_ARG_TYPE(t)					"unexpected param type %c", (t)
#define IVM_ERROR_MSG_PUSH_DEAD_CORO_STACK						"cannot push object to dead coro"
#define IVM_ERROR_MSG_SERIALIZE_CACHED_EXEC						"cannot serialize cached executable"
#define IVM_ERROR_MSG_UNEXPECTED_INSTR_ARG_CACHE				"unexpected instruction string operand cache(not appear in the string pool)"
#define IVM_ERROR_MSG_FILE_FORMAT_ERR(file, format)				"wrong file format of file %s, expecting %s file", (file), (format)
#define IVM_ERROR_MSG_ILLEGAL_STRING_LEN(len, max)				"illegal string len %ld(expect less than %ld)", (ivm_ulong_t)(len), (ivm_ulong_t)(max)
#define IVM_ERROR_MSG_DECACHE_FUNC_ID_WITHOUT_STATE				("decache function id without vm state")
#define IVM_ERROR_MSG_CORO_NATIVE_ROOT							("root function of coroutine cannot be native")
#define IVM_ERROR_MSG_OPT_NO_GEN_FOR_JMPTO						("no code generated for target address")
#define IVM_ERROR_MSG_NO_ALIVE_CORO_TO_SCHEDULE					("one-round scheduler require at least one alive coroutine")
#define IVM_ERROR_MSG_CORO_EXCEPTION(coro, file, line, msg)		"coro %p killed: exception: %s: line %ld: %s", (void *)(coro), (file), (line), (msg)
#define IVM_ERROR_MSG_CIRCULAR_PROTO_REF						("circular prototype reference detected")
#define IVM_ERROR_MSG_ILLEGAL_GID_TYPE(type)					"illegal group id with type <%s>", (type)
#define IVM_ERROR_MSG_GROUP_ID_OVERFLOW							("group id overflow")
#define IVM_ERROR_MSG_WRONG_NATIVE_ARG_RULE(c)					"unrecognized argument rule %c", (c)
#define IVM_ERROR_MSG_REPEAT_OPTIONAL_MARK						("repeated optional mark in argument rule")
#define IVM_ERROR_MSG_ILLEGAL_STEP								("illegal step")
#define IVM_ERROR_MSG_STRING_IDX_EXCEED(i, size)				"string index exceeded(index %ld is too large for the string of size %ld)", (i), (size)
#define IVM_ERROR_MSG_LINK_OFFSET_MISMATCH(ofs, exp)			"mismatched link offset(the actual offset is %ld while expecting %ld)", (ivm_ulong_t)(ofs), (ivm_ulong_t)(exp)
#define IVM_ERROR_MSG_CONTEXT_NO_PREV_NODE						("current context node has no previous node")
#define IVM_ERROR_MSG_ASSIGN_TO_STRING_INDEX					("cannot assign to string index")
#define IVM_ERROR_MSG_FAILED_LOAD_INIT_FUNC						("failed to load init function")
#define IVM_ERROR_MSG_FAILED_OPEN_FILE							("failed to open the file")
#define IVM_ERROR_MSG_FAILED_PARSE_CACHE						("failed to parse the cache file")
#define IVM_ERROR_MSG_CACHE_NO_ROOT								("root function not specified in the cache file")
#define IVM_ERROR_MSG_FAILED_TO_PARSE_SOURCE					("failed to parse source file")
#define IVM_ERROR_MSG_YIELD_ATOM_CORO							("yield in a atomic coroutine(has native call(s) in the stack)")
#define IVM_ERROR_MSG_CORO_GROUP_NOT_EXIST(cgid)				"coro group with id %d does not exist", (cgid)
#define IVM_ERROR_MSG_CORO_GROUP_SUSPENDED(cgid)				"coro group with id %d has suspended and cannot be yielded to", (cgid)
#define IVM_ERROR_MSG_MOD_NOT_FOUND(mod)						"module '%s' not found in any specified module search path", (mod)
#define IVM_ERROR_MSG_MOD_LOAD_ERROR(mod, path, msg)			"error(s) happended loading module '%s'(found at %s): %s", (mod), (path), (msg)
#define IVM_ERROR_MSG_UNPACK_NON_LIST(type)						"cannot unpack the object of type <%s>", (type)
#define IVM_ERROR_MSG_ITER_END									("iteration has done")
#define IVM_ERROR_MSG_NON_ITERABLE								("non-iterable object used for iteration")
#define IVM_ERROR_MSG_MERGE_EMPTY_EXEC_UNIT						("merging empty exec unit")
#define IVM_ERROR_MSG_UNEXPECT_IS_OPERAND						("unexpected type of the second operand of 'is' function(expecting none or type)")
#define IVM_ERROR_MSG_UNKNOWN_TP_TYPE(name)						"unregistered third-party type '%s'", (name)
#define IVM_ERROR_MSG_REDEF_TP_TYPE(name)						"third-party type '%s' is redefined", (name)
#define IVM_ERROR_MSG_UNABLE_TO_CONVERT_STR(type)				"unable to convert object of type <%s> to string", (type)
#define IVM_ERROR_MSG_MAX_PATH_LEN_REACHED						"max path length reached"
#define IVM_ERROR_MSG_FAILED_GET_ABS_PATH(path)					"failed to obtain the absolute path for '%s'", (path)
#define IVM_ERROR_MSG_FAILED_ALLOC_BUFFER(size)					"failed to allocate buffer with size %ld", (size)
#define IVM_ERROR_MSG_ILLEGAL_BUFFER_SIZE(size)					"illegal buffer size %ld", (size)
#define IVM_ERROR_MSG_NO_ENOUGH_BLOCK							"no enough block"
#define IVM_ERROR_MSG_MEM_ERROR									"memory error"
#define IVM_ERROR_MSG_WRONG_ARG_C(expect)						"wrong argument(expecting %s)", (expect)
#define IVM_ERROR_MSG_FAILED_PARSE_NUM(str)						"failed to parse number '%s'", (str)
#define IVM_ERROR_MSG_EMPTY_BUFFER								"creating empty buffer"

IVM_COM_END

#endif
