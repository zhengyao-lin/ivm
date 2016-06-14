#ifndef _IVM_VM_OPCODE_REQ_H_
#define _IVM_VM_OPCODE_REQ_H_

/* opcode handler requirement */

#include "pub/const.h"
#include "pub/err.h"
#include "pub/vm.h"

#include "inline/obj.h"
#include "inline/call.h"
#include "inline/runtime.h"
#include "inline/context.h"
#include "inline/vm.h"
#include "inline/func.h"
#include "obj.h"
#include "strobj.h"
#include "num.h"
#include "coro.h"
#include "vmstack.h"
#include "context.h"
#include "call.h"
#include "func.h"
#include "oprt.h"
#include "dbg.h"

#define DEFAULT_UNIOP_HANDLER(op, op_name) \
	{                                                                                          \
		ivm_object_t *op1;                                                                     \
		ivm_uniop_proc_t proc;                                                                 \
                                                                                               \
		CHECK_STACK(1);                                                                        \
                                                                                               \
		op1 = STACK_POP();                                                                     \
		proc = IVM_OBJECT_GET_UNIOP_PROC(op, op1);                                             \
                                                                                               \
		IVM_ASSERT(proc,                                                                       \
				   IVM_ERROR_MSG_NO_UNIOP_FOR(op_name,                                         \
											  IVM_OBJECT_GET(op1, TYPE_NAME)));                \
                                                                                               \
		STACK_PUSH(proc(_STATE, op1));                                                         \
		NEXT_INSTR();                                                                          \
	}

#define DEFAULT_BINOP_HANDLER(op, op_name) \
	{                                                                                          \
		ivm_object_t *op1, *op2;                                                               \
		ivm_binop_proc_t proc;                                                                 \
                                                                                               \
		CHECK_STACK(2);                                                                        \
                                                                                               \
		op2 = STACK_POP();                                                                     \
		op1 = STACK_POP();                                                                     \
		proc = IVM_OBJECT_GET_BINOP_PROC(op1, op, op2);                                        \
                                                                                               \
		IVM_ASSERT(proc,                                                                       \
				   IVM_ERROR_MSG_NO_BINOP_FOR(IVM_OBJECT_GET(op1, TYPE_NAME),                  \
				   							  op_name,                                         \
											  IVM_OBJECT_GET(op2, TYPE_NAME)));                \
                                                                                               \
		STACK_PUSH(proc(_STATE, op1, op2));                                                    \
		NEXT_INSTR();                                                                          \
	}

#define CMP_BINOP_HANDLER(todo) \
	{                                                                                          \
		ivm_object_t *op1, *op2;                                                               \
		ivm_ptr_t _RETVAL;                                                                     \
		ivm_binop_proc_t proc;                                                                 \
                                                                                               \
		CHECK_STACK(2);                                                                        \
                                                                                               \
		op2 = STACK_POP();                                                                     \
		op1 = STACK_POP();                                                                     \
		proc = IVM_OBJECT_GET_BINOP_PROC(op1, CMP, op2);                                       \
                                                                                               \
		IVM_ASSERT(proc,                                                                       \
				   IVM_ERROR_MSG_NO_BINOP_FOR(IVM_OBJECT_GET(op1, TYPE_NAME),                  \
				   							  "<=>",                                           \
											  IVM_OBJECT_GET(op2, TYPE_NAME)));                \
                                                                                               \
		_RETVAL = (ivm_ptr_t)proc(_STATE, op1, op2);                                           \
		todo;                                                                                  \
		NEXT_INSTR();                                                                          \
	}

#endif
