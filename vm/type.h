#ifndef _IVM_VM_TYPE_H_
#define _IVM_VM_TYPE_H_

#include <stdint.h>
#include <wchar.h>

typedef int				ivm_bool_t;

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

typedef ivm_uint64_t	ivm_size_t;
typedef ivm_uint8_t		ivm_byte_t;

typedef ivm_sint32_t	ivm_type_t;

typedef ivm_double_t	ivm_numeric_t;

#define IVM_UNDEFINED	0
#define IVM_NULL_T		1
#define IVM_OBJECT_T	2
#define IVM_NUMERIC_T	3

#endif
