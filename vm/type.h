#ifndef _IVM_VM_TYPE_H_
#define _IVM_VM_TYPE_H_

#include <stdint.h>
#include <wchar.h>

typedef int				ivm_bool_t;

typedef int				ivm_int_t;
typedef unsigned int	ivm_uint_t;

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

typedef ivm_sint32_t	ivm_type_tag_t;

typedef ivm_double_t	ivm_number_t;

#define IVM_UNDEFINED_T			0
#define IVM_NULL_T				1
#define IVM_OBJECT_T			2
#define IVM_NUMERIC_T			3
#define IVM_FUNCTION_OBJECT_T	4

typedef ivm_size_t		ivm_pc_t;
typedef ivm_sint32_t	ivm_op_arg_t;

typedef intptr_t		ivm_mark_t;

#define IVM_MARK_INIT 0

#define IVM_GET(obj, type, member) (IVM_NULL, type##_GET_##member(obj))
#define IVM_SET(obj, type, member, val) (type##_SET_##member((obj), (val)))

#endif
