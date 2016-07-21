#ifndef _IVM_VM_OPRT_REQ_H_
#define _IVM_VM_OPRT_REQ_H_

#include "pub/vm.h"
#include "pub/err.h"
#include "pub/inlines.h"

#include "std/string.h"

#include "obj.h"
#include "num.h"
#include "strobj.h"
#include "listobj.h"

#define GET_STRING_INDEX() \
	{                                                                                         \
		ivm_object_t *tmp_obj;                                                                \
                                                                                              \
		if (_ASSIGN) {                                                                        \
			ivm_object_setSlot(_OP1, _STATE, ivm_string_object_getValue(_OP2), _ASSIGN);      \
			return _ASSIGN;                                                                   \
		}                                                                                     \
                                                                                              \
		tmp_obj = ivm_object_getSlot(_OP1, _STATE, ivm_string_object_getValue(_OP2));         \
                                                                                              \
		return tmp_obj ? tmp_obj : IVM_NULL_OBJ(_STATE);                                      \
	}

#endif
