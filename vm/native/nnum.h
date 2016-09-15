#ifndef _IVM_VM_NATIVE_NNUM_H_
#define _IVM_VM_NATIVE_NNUM_H_

#include "pub/com.h"
#include "pub/type.h"
#include "pub/vm.h"

IVM_COM_HEADER

IVM_NATIVE_FUNC(_numeric_ceil);
IVM_NATIVE_FUNC(_numeric_floor);
IVM_NATIVE_FUNC(_numeric_round);

IVM_NATIVE_FUNC(_numeric_isnan);
IVM_NATIVE_FUNC(_numeric_isinf);
IVM_NATIVE_FUNC(_numeric_isposinf);
IVM_NATIVE_FUNC(_numeric_isneginf);

IVM_NATIVE_FUNC(_numeric_char);

IVM_COM_END

#endif
