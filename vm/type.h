#ifndef _IVM_VM_TYPE_H_
#define _IVM_VM_TYPE_H_

#include <stdint.h>
#include <stddef.h>
#include <wchar.h>

#include "pub/com.h"

typedef int				ivm_bool_t;

typedef int				ivm_int_t;
typedef unsigned int	ivm_uint_t;
typedef long			ivm_long_t;
typedef unsigned long	ivm_ulong_t;

typedef intptr_t		ivm_ptr_t;
typedef uintptr_t		ivm_uptr_t;
typedef ptrdiff_t		ivm_ptrdiff_t;

typedef int8_t			ivm_sint8_t;
typedef int16_t			ivm_sint16_t;
typedef int32_t			ivm_sint32_t;
typedef int64_t			ivm_sint64_t;

typedef uint8_t			ivm_uint8_t;
typedef uint16_t		ivm_uint16_t;
typedef uint32_t		ivm_uint32_t;
typedef uint64_t		ivm_uint64_t;

typedef float			ivm_single_t;
typedef double			ivm_double_t;

typedef wchar_t			ivm_wchar_t;
typedef char			ivm_char_t;

#define IVM_NULL		(NULL)
#define IVM_FALSE		0
#define IVM_TRUE		!0

typedef ivm_uint64_t	ivm_size_t;
typedef ivm_uint8_t		ivm_byte_t;

typedef ivm_int_t		ivm_type_tag_t;

typedef ivm_double_t	ivm_number_t;

enum {
	IVM_UNDEFINED_T = 0,
	IVM_NULL_T,
	IVM_OBJECT_T,
	IVM_NUMERIC_T,
	IVM_STRING_OBJECT_T,
	IVM_FUNCTION_OBJECT_T,
	IVM_TYPE_COUNT
};

typedef ivm_size_t		ivm_pc_t;
typedef union {
	ivm_long_t iarg;
	ivm_double_t farg;
} ivm_opcode_arg_t;

#define ivm_opcode_arg_toInt(arg) (arg.iarg)
#define ivm_opcode_arg_toFloat(arg) (arg.farg)

#define ivm_opcode_arg_fromInt(i) ((ivm_opcode_arg_t) { .iarg = (i) })
#define ivm_opcode_arg_fromFloat(f) ((ivm_opcode_arg_t) { .farg = (f) })

typedef ivm_uint32_t	ivm_argc_t;

typedef ivm_ptr_t		ivm_mark_t;

#define IVM_MARK_INIT 0

#define IVM_GET(obj, type, member) (IVM_NULL, type##_GET_##member(obj))
#define IVM_SET(obj, type, member, val) (type##_SET_##member((obj), (val)))

#endif
