#include "pub/const.h"
#include "pub/com.h"
#include "pub/err.h"
#include "pub/vm.h"
#include "pub/inlines.h"

#include "std/mem.h"
#include "std/string.h"

#include "gc.h"
#include "obj.h"
#include "instr.h"
#include "slot.h"

#include "func.h"
#include "coro.h"

ivm_bool_t
ivm_object_toBool(ivm_object_t *obj,
				  ivm_vmstate_t *state)
{
	ivm_type_t *type = IVM_TYPE_OF(obj);
	ivm_bool_converter_t conv = type->to_bool;

	if (conv)
		return conv(obj, state);

	return type->const_bool;
}

void
ivm_object_setSlot(ivm_object_t *obj,
				   ivm_vmstate_t *state,
				   const ivm_string_t *key,
				   ivm_object_t *value)
{
	if (obj->slots) {
		ivm_object_copyOnWrite(obj, state);
	} else {
		obj->slots = ivm_slot_table_newAt(state, IVM_OBJECT_GET(obj, GEN));
	}

	ivm_slot_table_setSlot(obj->slots, state, key, value);

	return;
}

void
ivm_object_setSlot_r(ivm_object_t *obj,
					 ivm_vmstate_t *state,
					 const ivm_char_t *rkey,
					 ivm_object_t *value)
{
	if (obj->slots) {
		ivm_object_copyOnWrite(obj, state);
	} else {
		obj->slots = ivm_slot_table_newAt(state, IVM_OBJECT_GET(obj, GEN));
	}

	ivm_slot_table_setSlot_r(obj->slots, state, rkey, value);

	return;
}

void
ivm_object_setSlot_cc(ivm_object_t *obj,
					  ivm_vmstate_t *state,
					  const ivm_string_t *key,
					  ivm_object_t *value,
					  ivm_instr_t *instr)
{
	if (obj->slots) {
		ivm_object_copyOnWrite(obj, state);
	} else {
		obj->slots = ivm_slot_table_newAt(state, IVM_OBJECT_GET(obj, GEN));
	}

	ivm_slot_table_setSlot_cc(obj->slots, state, key, value, instr);

	return;
}

ivm_bool_t
ivm_object_setExistSlot(ivm_object_t *obj,
						ivm_vmstate_t *state,
						const ivm_string_t *key,
						ivm_object_t *value)
{
	if (obj->slots) {
		ivm_object_copyOnWrite(obj, state);
	} else {
		obj->slots = ivm_slot_table_newAt(state, IVM_OBJECT_GET(obj, GEN));
	}

	return ivm_slot_table_setExistSlot(obj->slots, state, key, value);
}

ivm_bool_t
ivm_object_setExistSlot_cc(ivm_object_t *obj,
						   ivm_vmstate_t *state,
						   const ivm_string_t *key,
						   ivm_object_t *value,
						   ivm_instr_t *instr)
{
	if (obj->slots) {
		ivm_object_copyOnWrite(obj, state);
	} else {
		obj->slots = ivm_slot_table_newAt(state, IVM_OBJECT_GET(obj, GEN));
	}

	return ivm_slot_table_setExistSlot_cc(obj->slots, state, key, value, instr);
}

ivm_bool_t
ivm_object_setEmptySlot_r(ivm_object_t *obj,
						  ivm_vmstate_t *state,
						  const ivm_char_t *rkey,
						  ivm_object_t *value)
{
	if (obj->slots) {
		ivm_object_copyOnWrite(obj, state);
	} else {
		obj->slots = ivm_slot_table_newAt(state, IVM_OBJECT_GET(obj, GEN));
	}

	return ivm_slot_table_setEmptySlot_r(obj->slots, state, rkey, value);
}

void
ivm_object_setOop(ivm_object_t *obj,
				  ivm_vmstate_t *state,
				  ivm_int_t op,
				  ivm_object_t *func)
{
	if (obj->slots) {
		ivm_object_copyOnWrite(obj, state);
	} else {
		obj->slots = ivm_slot_table_newAt(state, IVM_OBJECT_GET(obj, GEN));
	}

	ivm_object_markOop(obj);
	ivm_slot_table_setOop(obj->slots, state, op, func);

	return;
}

void
ivm_object_expandSlotTable(ivm_object_t *obj,
						   ivm_vmstate_t *state,
						   ivm_size_t size)
{
	if (obj->slots) {
		ivm_slot_table_expandTo(ivm_object_copyOnWrite(obj, state), state, size);
	} else {
		obj->slots = ivm_slot_table_newAt_c(state, size, IVM_OBJECT_GET(obj, GEN));
	}

	return;
}

IVM_INLINE
ivm_object_t *
_ivm_object_searchProtoSlot(ivm_object_t *obj,
							ivm_vmstate_t *state,
							const ivm_string_t *key)
{
	ivm_object_t *i = ivm_object_getProto(obj),
				 *ret = IVM_NULL;

	if (!i) return ret;

	/* no loop is allowed when setting proto */
	while (i) {
		if (i->slots) {
			ret = ivm_slot_getValue(ivm_slot_table_getSlot(i->slots, state, key));
		}

		if (ret) break;

		i = ivm_object_getProto(i);
	}

	return ret;
}

IVM_INLINE
ivm_object_t *
_ivm_object_searchProtoSlot_cc(ivm_object_t *obj,
							   ivm_vmstate_t *state,
							   const ivm_string_t *key,
							   ivm_instr_t *instr)
{
	ivm_object_t *i = ivm_object_getProto(obj),
				 *ret = IVM_NULL;

	if (!i) return ret;

	/* no loop is allowed when setting proto */
	while (i) {
		if (i->slots) {
			ret = ivm_slot_getValue(ivm_slot_table_getSlot_cc(i->slots, state, key, instr));
		}
		if (ret) break;
		i = ivm_object_getProto(i);
	}

	return ret;
}

ivm_object_t *
ivm_object_getSlot(ivm_object_t *obj,
				   ivm_vmstate_t *state,
				   const ivm_string_t *key)
{
	ivm_object_t *ret = IVM_NULL;
	ivm_slot_table_t *slots = obj->slots;

	if (slots) {
		ret = ivm_slot_getValue(ivm_slot_table_getSlot(slots, state, key));
	}

	if (!ret) {
		ret = _ivm_object_searchProtoSlot(obj, state, key);
	}

	return ret;
}

ivm_object_t *
ivm_object_getSlot_r(ivm_object_t *obj,
					 ivm_vmstate_t *state,
					 const ivm_char_t *rkey)
{
	ivm_object_t *ret = IVM_NULL;
	const ivm_string_t *key = ivm_vmstate_constantize_r(state, rkey);
	ivm_slot_table_t *slots = obj->slots;

	if (slots) {
		ret = ivm_slot_getValue(ivm_slot_table_getSlot(slots, state, key));
	}

	if (!ret) {
		ret = _ivm_object_searchProtoSlot(obj, state, key);
	}

	return ret;
}

ivm_object_t *
ivm_object_getSlot_cc(ivm_object_t *obj,
					  ivm_vmstate_t *state,
					  const ivm_string_t *key,
					  ivm_instr_t *instr)
{
	ivm_object_t *ret = IVM_NULL;
	ivm_slot_table_t *slots = obj->slots;

	if (slots) {
		ret = ivm_slot_getValue(ivm_slot_table_getSlot_cc(slots, state, key, instr));
	}

	if (!ret) {
		ret = _ivm_object_searchProtoSlot_cc(obj, state, key, instr);
	}

	return ret;
}

ivm_object_t *
ivm_object_getOop(ivm_object_t *obj,
				  ivm_vmstate_t *state,
				  ivm_int_t oop_id)
{
	register ivm_function_t *tmp_func;

	_IVM_OBJECT_OPSEARCH(obj, {
		return tmp;
	}, {
		tmp_func = ivm_type_getDefaultOop(obj->type, oop_id);
		if (tmp_func) {
			return ivm_function_object_new(state, IVM_NULL, tmp_func);
		}
	}, {
		return IVM_NULL;
	});
}

ivm_object_t *
ivm_object_getDefaultOop(ivm_object_t *obj,
						 ivm_vmstate_t *state,
						 ivm_int_t oop)
{
	register ivm_function_t *tmp_func;

	if (ivm_object_isBTProto(obj) &&
		!ivm_slot_table_hasBlockedOop(obj->slots, oop)) {
		tmp_func = ivm_type_getDefaultOop(obj->type, oop);
		if (tmp_func) {
			return ivm_function_object_new(state, IVM_NULL, tmp_func);
		}
	}

	return IVM_NULL;
}

#define SET_EXC IVM_CORO_NATIVE_FATAL_C

/**
 * NOTE: Op fallback
 *
 * Op fallback will occur when a builtin op finds that the types of operands do not
 * match what it has expected. It will try to find another operator handler in the prototypes
 * and call it instead.
 *
 * e.g.
 *
 * object.proto.proto = { proto: none, [=]: fn k, v: { print("here!") } }
 * 
 * {}[1] = "hi"
 * {}.[=](2, "hi")
 * 
 * These two expressions will have the same output("here!").
 * Because the the original index op of object which has the form of <object> [=] <string>
 * does not match the operands(<object>, <numeric>).
 * So it will fall back to find another op which finally falls the "here" function.
 *
 * NOTE that even though the finally executed function is the "here" function, {}.[=] has the same value as
 * the default index op of object.
 *
 * And also NOTE that op fallback is not a default action taken by the vm but rather by the op handlers themselves.
 */
ivm_object_t *
ivm_object_doBinOpFallBack(ivm_object_t *obj, ivm_vmstate_t *state,
						   ivm_coro_t *coro, ivm_int_t op, ivm_int_t oop_id,
						   ivm_bool_t is_cmp, ivm_object_t *op2)
{
	ivm_object_t *proto = ivm_type_getProto(obj->type);
	ivm_object_t *oop, *base;
	ivm_binop_proc_t proc = ivm_object_getBinOp(proto, state, op, oop_id, op2, &oop);
	ivm_function_object_t *func;

	if (proc) {
		return is_cmp ? ivm_numeric_new(state, (ivm_ptr_t)proc(state, coro, obj, op2))
					  : proc(state, coro, obj, op2);
	} else if (oop) {
		func = ivm_object_callable(oop, state, &base);
		
		if (!func) {
			SET_EXC(coro, state, IVM_ERROR_MSG_UNABLE_TO_INVOKE(IVM_OBJECT_GET(obj, TYPE_NAME)));
			return IVM_NULL;
		}

		base = base ? base : obj;

		return ivm_coro_callBase_1(coro, state, func, base, op2);
	}

	SET_EXC(coro, state,
			IVM_ERROR_MSG_NO_BINOP_FOR(IVM_OBJECT_GET(obj, TYPE_NAME),
									   ivm_oop_rawName(oop_id),
									   IVM_OBJECT_GET(op2, TYPE_NAME)));

	return IVM_NULL;
}

ivm_object_t *
ivm_object_doTriOpFallBack(ivm_object_t *obj, ivm_vmstate_t *state,
						   ivm_coro_t *coro, ivm_int_t op, ivm_int_t oop_id,
						   ivm_object_t *op2, ivm_object_t *op3)
{
	ivm_object_t *proto = ivm_type_getProto(obj->type);
	ivm_object_t *oop, *base;
	ivm_triop_proc_t proc = (ivm_triop_proc_t)ivm_object_getBinOp(proto, state, op, oop_id, op2, &oop);
	ivm_function_object_t *func;

	if (proc) {
		return  proc(state, coro, obj, op2, op3);
	} else if (oop) {
		func = ivm_object_callable(oop, state, &base);
		
		if (!func) {
			SET_EXC(coro, state, IVM_ERROR_MSG_UNABLE_TO_INVOKE(IVM_OBJECT_GET(obj, TYPE_NAME)));
			return IVM_NULL;
		}

		base = base ? base : obj;

		return ivm_coro_callBase_2(coro, state, func, base, op2, op3);
	}

	SET_EXC(coro, state,
			IVM_ERROR_MSG_NO_BINOP_FOR(IVM_OBJECT_GET(obj, TYPE_NAME),
									   ivm_oop_rawName(oop_id),
									   IVM_OBJECT_GET(op2, TYPE_NAME)));

	return IVM_NULL;
}

/* d for dynamic */

#define IS_PROTO(rkey) ( \
	(rkey)[0] == 'p' &&    \
	(rkey)[1] == 'r' &&    \
	(rkey)[2] == 'o' &&    \
	(rkey)[3] == 't' &&    \
	(rkey)[4] == 'o' &&    \
	(rkey)[5] == '\0'      \
)

ivm_bool_t /* true for success, false for circular prototype chain */
ivm_object_setSlot_d(ivm_object_t *obj,
					 ivm_vmstate_t *state,
					 const ivm_string_t *key,
					 ivm_object_t *value)
{
	ivm_int_t oop = ivm_vmstate_isOopSymbol(state, key);
	const ivm_char_t *rkey = ivm_string_trimHead(key);

	if (IS_PROTO(rkey)) {
		if (!ivm_object_hasProto(obj, value)) {
			ivm_object_setProto(obj, state, value);
		} else {
			return IVM_FALSE;
		}
	} else if (oop != -1) {
		ivm_object_setOop(obj, state, oop, value);
	} else {
		ivm_object_setSlot(obj, state, key, value);
	}

	return IVM_TRUE;
}

ivm_object_t *
ivm_object_getSlot_d(ivm_object_t *obj,
					 ivm_vmstate_t *state,
					 const ivm_string_t *key)
{
	ivm_int_t oop = ivm_vmstate_isOopSymbol(state, key);
	const ivm_char_t *rkey = ivm_string_trimHead(key);

	if (IS_PROTO(rkey)) {
		return ivm_object_getProto(obj);
	} else if (oop != -1) {
		return ivm_object_getOop(obj, state, oop);
	}

	return ivm_object_getSlot(obj, state, key);
}
