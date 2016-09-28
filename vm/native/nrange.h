#ifndef _IVM_VM_NATIVE_NRANGE_H_
#define _IVM_VM_NATIVE_NRANGE_H_

#include "pub/com.h"
#include "pub/type.h"
#include "pub/vm.h"

IVM_COM_HEADER

IVM_NATIVE_FUNC(_range_cons);
IVM_NATIVE_FUNC(_range_iter);

IVM_NATIVE_FUNC(_range_iter_cons);
IVM_NATIVE_FUNC(_range_iter_next);

IVM_COM_END

#endif
