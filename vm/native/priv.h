#ifndef _IVM_VM_NATIVE_PRIV_H_
#define _IVM_VM_NATIVE_PRIV_H_

#include "pub/err.h"

#define CHECK_BASE(expect) \
	IVM_ASSERT(IVM_IS_TYPE(arg.base, (expect)), IVM_NATIVE_ERROR_MSG_WRONG_BASE)

#define IVM_NATIVE_ERROR_MSG_WRONG_BASE				"wrong base type"

#endif
