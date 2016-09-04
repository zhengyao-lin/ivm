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

#define UNIOP_HANDLER(op, op_name, e) \
	{                                                                                          \
		CHECK_STACK(1);                                                                        \
                                                                                               \
		_TMP_OBJ1 = STACK_POP();                                                               \
		_TMP_OBJ2 = ivm_object_getOop(_TMP_OBJ1, IVM_OOP_ID(op));                              \
		if (_TMP_OBJ2) {                                                                       \
			STACK_PUSH(_TMP_OBJ1);                                                             \
			STACK_PUSH(_TMP_OBJ2);                                                             \
			SET_IARG(0);                                                                       \
			GOTO_INSTR(INVOKE_BASE);                                                           \
		} else {                                                                               \
			e;                                                                                 \
	                                                                                           \
			_TMP_UNI_PROC = IVM_OBJECT_GET_UNIOP_PROC(_TMP_OBJ1, op);                          \
	                                                                                           \
			RTM_ASSERT(_TMP_UNI_PROC,                                                          \
					   IVM_ERROR_MSG_NO_UNIOP_FOR(op_name,                                     \
												  IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME)));      \
	                                                                                           \
	        _TMP_OBJ1 = _TMP_UNI_PROC(_STATE, _CORO, _TMP_OBJ1);                               \
	        if (!_TMP_OBJ1) {                                                                  \
	        	EXCEPTION();                                                                   \
			}                                                                                  \
			STACK_PUSH(_TMP_OBJ1);                                                             \
			NEXT_INSTR();                                                                      \
		}                                                                                      \
	}

#define BINOP_HANDLER(op, op_name, e, req, assign) \
	{                                                                                          \
		CHECK_STACK(req);                                                                      \
                                                                                               \
		_TMP_OBJ2 = STACK_POP();                                                               \
		_TMP_OBJ1 = STACK_POP();                                                               \
		_TMP_OBJ3 = (assign);                                                                  \
		_TMP_OBJ4 = ivm_object_getOop(_TMP_OBJ1, IVM_OOP_ID(op));                              \
		if (_TMP_OBJ4) {                                                                       \
			STACK_PUSH(_TMP_OBJ2);                                                             \
			if (_TMP_OBJ3) {                                                                   \
				STACK_PUSH(_TMP_OBJ3);                                                         \
				STACK_PUSH(_TMP_OBJ1);                                                         \
				STACK_PUSH(_TMP_OBJ4);                                                         \
				SET_IARG(2);                                                                   \
			} else {                                                                           \
				STACK_PUSH(_TMP_OBJ1);                                                         \
				STACK_PUSH(_TMP_OBJ4);                                                         \
				SET_IARG(1);                                                                   \
			}                                                                                  \
			GOTO_INSTR(INVOKE_BASE);                                                           \
		} else {                                                                               \
			e;                                                                                 \
			                                                                                   \
			_TMP_BIN_PROC = IVM_OBJECT_GET_BINOP_PROC(_TMP_OBJ1, op, _TMP_OBJ2);               \
	                                                                                           \
			RTM_ASSERT(_TMP_BIN_PROC,                                                          \
					   IVM_ERROR_MSG_NO_BINOP_FOR(IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME),        \
					   							  op_name,                                     \
												  IVM_OBJECT_GET(_TMP_OBJ2, TYPE_NAME)));      \
	                                                                                           \
	        _TMP_OBJ1 = _TMP_BIN_PROC(_STATE, _CORO, _TMP_OBJ1, _TMP_OBJ2, _TMP_OBJ3);         \
	        if (!_TMP_OBJ1) {                                                                  \
	        	EXCEPTION();                                                                   \
			}                                                                                  \
			STACK_PUSH(_TMP_OBJ1);                                                             \
			NEXT_INSTR();                                                                      \
		}                                                                                      \
	}

#define CMP_HANDLER(op, op_name, e) \
	{                                                                                          \
		CHECK_STACK(2);                                                                        \
                                                                                               \
		_TMP_OBJ2 = STACK_POP();                                                               \
		_TMP_OBJ1 = STACK_POP();                                                               \
		_TMP_OBJ4 = ivm_object_getOop(_TMP_OBJ1, IVM_OOP_ID(op));                              \
		if (_TMP_OBJ4) {                                                                       \
			IVM_RUNTIME_SET(_RUNTIME, NO_REG, IVM_TRUE);                                       \
			STACK_PUSH(_TMP_OBJ2);                                                             \
			STACK_PUSH(_TMP_OBJ1);                                                             \
			STACK_PUSH(_TMP_OBJ4);                                                             \
			SET_IARG(1);                                                                       \
			GOTO_INSTR(INVOKE_BASE);                                                           \
		} else {                                                                               \
			IVM_RUNTIME_SET(_RUNTIME, NO_REG, IVM_FALSE);                                      \
			e;                                                                                 \
			_TMP_BIN_PROC = IVM_OBJECT_GET_BINOP_PROC(_TMP_OBJ1, op, _TMP_OBJ2);               \
	                                                                                           \
			RTM_ASSERT(_TMP_BIN_PROC,                                                          \
					   IVM_ERROR_MSG_NO_BINOP_FOR(IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME),        \
					   							  op_name,                                     \
												  IVM_OBJECT_GET(_TMP_OBJ2, TYPE_NAME)));      \
			_TMP_CMP_REG                                                                       \
			= (ivm_ptr_t)_TMP_BIN_PROC(_STATE, _CORO, _TMP_OBJ1, _TMP_OBJ2, IVM_NULL);         \
			if (ivm_vmstate_getException(_STATE)) {                                            \
				EXCEPTION();                                                                   \
			}                                                                                  \
	        STACK_PUSH(ivm_numeric_new(_STATE, _TMP_CMP_REG));                                 \
			NEXT_INSTR();                                                                      \
		}                                                                                      \
	}

#define CMP_HANDLER_R(op, op_name, e) \
	{                                                                                          \
		CHECK_STACK(2);                                                                        \
                                                                                               \
		_TMP_OBJ2 = STACK_POP();                                                               \
		_TMP_OBJ1 = STACK_POP();                                                               \
		_TMP_OBJ4 = ivm_object_getOop(_TMP_OBJ1, IVM_OOP_ID(op));                              \
		if (_TMP_OBJ4) {                                                                       \
			IVM_RUNTIME_SET(_RUNTIME, NO_REG, IVM_TRUE);                                       \
			STACK_PUSH(_TMP_OBJ2);                                                             \
			STACK_PUSH(_TMP_OBJ1);                                                             \
			STACK_PUSH(_TMP_OBJ4);                                                             \
			SET_IARG(1);                                                                       \
			GOTO_INSTR(INVOKE_BASE);                                                           \
		} else {                                                                               \
			IVM_RUNTIME_SET(_RUNTIME, NO_REG, IVM_FALSE);                                      \
			e;                                                                                 \
			_TMP_BIN_PROC = IVM_OBJECT_GET_BINOP_PROC(_TMP_OBJ1, op, _TMP_OBJ2);               \
	                                                                                           \
			RTM_ASSERT(_TMP_BIN_PROC,                                                          \
					   IVM_ERROR_MSG_NO_BINOP_FOR(IVM_OBJECT_GET(_TMP_OBJ1, TYPE_NAME),        \
					   							  op_name,                                     \
												  IVM_OBJECT_GET(_TMP_OBJ2, TYPE_NAME)));      \
	                                                                                           \
	        _TMP_CMP_REG                                                                       \
	        = (ivm_ptr_t)_TMP_BIN_PROC(_STATE, _CORO, _TMP_OBJ1, _TMP_OBJ2, IVM_NULL);         \
	        if (ivm_vmstate_getException(_STATE)) {                                            \
				EXCEPTION();                                                                   \
			}                                                                                  \
			NEXT_INSTR();                                                                      \
		}                                                                                      \
	}

IVM_INLINE
void
STACK_FILLIN(ivm_object_t **start,
			 ivm_object_t *obj,
			 ivm_int_t count)
{
	while (count--)
		*start++ = obj;

	return;
}

#if IVM_DISPATCH_METHOD_DIRECT_THREAD
	#include "dispatch/direct.h"
#endif

#endif
