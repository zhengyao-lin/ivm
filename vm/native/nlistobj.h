#ifndef _IVM_VM_NATIVE_NLISTOBJ_H_
#define _IVM_VM_NATIVE_NLISTOBJ_H_

#include "pub/com.h"
#include "pub/type.h"
#include "pub/vm.h"

IVM_COM_HEADER

IVM_NATIVE_FUNC(_list_cons);
IVM_NATIVE_FUNC(_list_size);
IVM_NATIVE_FUNC(_list_push);
IVM_NATIVE_FUNC(_list_pop);
IVM_NATIVE_FUNC(_list_top);
IVM_NATIVE_FUNC(_list_slice);
IVM_NATIVE_FUNC(_list_iter);

IVM_NATIVE_FUNC(_list_iter_cons);
IVM_NATIVE_FUNC(_list_iter_next);

IVM_COM_END

#endif
