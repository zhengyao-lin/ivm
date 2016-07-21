#ifndef _IVM_VM_OPCODE_REQ_H_
#define _IVM_VM_OPCODE_REQ_H_

/* opcode handler requirement */

#include "pub/const.h"
#include "pub/err.h"
#include "pub/vm.h"
#include "pub/inlines.h"

#include "obj.h"
#include "num.h"
#include "strobj.h"
#include "listobj.h"
#include "coro.h"
#include "vmstack.h"
#include "context.h"
#include "call.h"
#include "func.h"
#include "oprt.h"
#include "dbg.h"

#define DEFAULT_UNIOP_HANDLER(op, op_name) \
	{                                                                                          \
		CHECK_STACK(1);                                                                        \
                                                                                               \
		_TMP_OBJ1 = STACK_POP();                                                               \
		_TMP_UNI_PROC = IVM_OBJECT_GET_UNIOP_PROC(_TMP_OBJ1, op);                              \
                                                                                               \
		IVM_ASSERT(_TMP_UNI_PROC,                                                              \
				   IVM_ERROR_MSG_NO_UNIOP_FOR(op_name,                                         \
											  IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME)));          \
                                                                                               \
		STACK_PUSH(_TMP_UNI_PROC(_STATE, _TMP_OBJ1));                                          \
		NEXT_INSTR();                                                                          \
	}

#define DEFAULT_BINOP_HANDLER(op, op_name, assign) \
	{                                                                                          \
		CHECK_STACK(2);                                                                        \
                                                                                               \
		_TMP_OBJ2 = STACK_POP();                                                               \
		_TMP_OBJ1 = STACK_POP();                                                               \
		_TMP_BIN_PROC = IVM_OBJECT_GET_BINOP_PROC(_TMP_OBJ1, op, _TMP_OBJ2);                   \
                                                                                               \
		IVM_ASSERT(_TMP_BIN_PROC,                                                              \
				   IVM_ERROR_MSG_NO_BINOP_FOR(IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME),            \
				   							  op_name,                                         \
											  IVM_OBJECT_GET(_TMP_OBJ2, TYPE_NAME)));          \
                                                                                               \
        _TMP_OBJ1 = _TMP_BIN_PROC(_STATE, _TMP_OBJ1, _TMP_OBJ2, (assign));                     \
		STACK_PUSH(_TMP_OBJ1);                                                                 \
		NEXT_INSTR();                                                                          \
	}

#define CMP_BINOP_HANDLER(todo) \
	{                                                                                          \
		CHECK_STACK(2);                                                                        \
                                                                                               \
		_TMP_OBJ2 = STACK_POP();                                                               \
		_TMP_OBJ1 = STACK_POP();                                                               \
		_TMP_BIN_PROC = IVM_OBJECT_GET_BINOP_PROC(_TMP_OBJ1, CMP, _TMP_OBJ2);                  \
                                                                                               \
		IVM_ASSERT(_TMP_BIN_PROC,                                                              \
				   IVM_ERROR_MSG_NO_BINOP_FOR(IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME),            \
				   							  "<=>",                                           \
											  IVM_OBJECT_GET(_TMP_OBJ2, TYPE_NAME)));          \
                                                                                               \
		_TMP_OBJ1 = _TMP_BIN_PROC(_STATE, _TMP_OBJ1, _TMP_OBJ2, IVM_NULL);                     \
		todo;                                                                                  \
		NEXT_INSTR();                                                                          \
	}

#if IVM_DISPATCH_METHOD_DIRECT_THREAD
	#include "dispatch/direct.h"
#endif

#endif
