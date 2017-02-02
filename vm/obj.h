#ifndef _IVM_VM_OBJ_H_
#define _IVM_VM_OBJ_H_

#include "pub/com.h"
#include "pub/const.h"
#include "pub/type.h"

#include "std/mem.h"
#include "std/list.h"
#include "std/string.h"

#include "instr.h"
#include "slot.h"
#include "oprt.h"
#include "type.h"

IVM_COM_HEADER

struct ivm_object_t_tag;
struct ivm_vmstate_t_tag;
struct ivm_function_t_tag;
struct ivm_coro_t_tag;

typedef struct ivm_object_t_tag {
	IVM_OBJECT_HEADER
} ivm_object_t;

#define IVM_OBJECT_GET_TYPE_TAG(obj) ((obj)->type->tag)
#define IVM_OBJECT_GET_TYPE_NAME(obj) ((obj)->type->name)
#define IVM_OBJECT_GET_TYPE_SIZE(obj) ((obj)->type->size)
#define IVM_OBJECT_GET_TYPE_DES(obj) ((obj)->type->des)
#define IVM_OBJECT_GET_TYPE_TRAV(obj) ((obj)->type->trav)
#define IVM_OBJECT_GET_TYPE_CONST_BOOL(obj) ((obj)->type->const_bool)
#define IVM_OBJECT_GET_TYPE_TO_BOOL(obj) ((obj)->type->to_bool)
#define IVM_OBJECT_GET_SLOTS(obj) ((obj)->slots)

IVM_INLINE
ivm_object_t *
IVM_OBJECT_GET_COPY(ivm_object_t *obj)
{
	if (IVM_IS64) {
		return (ivm_object_t *)(((ivm_uptr_t)obj->mark.copy << _IVM_MARK_HEADER_BITS) >> _IVM_MARK_HEADER_BITS);
	}

	return obj->mark.copy;
}

#define IVM_OBJECT_GET_WB(obj) ((obj)->mark.sub.wb)
#define IVM_OBJECT_GET_GEN(obj) ((obj)->mark.sub.gen)
#define IVM_OBJECT_GET_INC_GEN(obj) (++(obj)->mark.sub.gen)
#define IVM_OBJECT_GET_TRAV_PROTECT(obj) ((obj)->mark.sub.travp)

#define IVM_OBJECT_SET_SLOTS(obj, val) ((obj)->slots = (val))
// #define IVM_OBJECT_SET_COPY(obj, val) ((obj)->mark.copy = (val))

IVM_INLINE
void
IVM_OBJECT_SET_COPY(ivm_object_t *obj,
					ivm_object_t *copy)
{
	if (IVM_IS64) {
		obj->mark.copy = (ivm_object_t *)
						 ((((ivm_uptr_t)obj->mark.copy
						 	>> (sizeof(ivm_ptr_t) * 8 - _IVM_MARK_HEADER_BITS))
						 	<< (sizeof(ivm_ptr_t) * 8 - _IVM_MARK_HEADER_BITS))
						 	| (ivm_uptr_t)copy);
	} else {
		obj->mark.copy = copy;
	}

	return;
}

// #undef _IVM_MARK_HEADER_BITS

#define IVM_OBJECT_SET_WB(obj, val) ((obj)->mark.sub.wb = (val))
#define IVM_OBJECT_SET_GEN(obj, val) ((obj)->mark.sub.gen = (val))
#define IVM_OBJECT_SET_TRAV_PROTECT(obj, val) ((obj)->mark.sub.travp)

#define IVM_OBJECT_GET(obj, member) IVM_GET((obj), IVM_OBJECT, member)
#define IVM_OBJECT_SET(obj, member, val) IVM_SET((obj), IVM_OBJECT, member, (val))

#define IVM_OBJECT_GET_BINOP(obj, op) (ivm_type_getBinopTable((obj)->type, op))
#define IVM_OBJECT_GET_BINOP_R(obj, i) (ivm_type_getBinopTable_r((obj)->type, (i)))
#define IVM_OBJECT_GET_UNIOP(obj) (ivm_type_getUniopTable((obj)->type))

#define IVM_TYPE_OF(obj) ((obj)->type)
#define IVM_TYPE_TAG_OF IVM_OBJECT_GET_TYPE_TAG
#define IVM_IS_TYPE(obj, type) (IVM_TYPE_OF(obj) == (type))

/* call the operation proc when obj [op] obj(of type) e.g. obj + num */
#define IVM_OBJECT_GET_BINOP_PROC(op1, op, op2) \
	(ivm_binop_table_get(IVM_OBJECT_GET_BINOP((op1), op), \
						 IVM_OBJECT_GET((op2), TYPE_TAG)))

#define IVM_OBJECT_GET_BINOP_PROC_R(op1, i, op2) \
	(ivm_binop_table_get(IVM_OBJECT_GET_BINOP_R((op1), (i)), \
						 IVM_OBJECT_GET((op2), TYPE_TAG)))

// raw and use type
#define IVM_OBJECT_GET_BINOP_PROC_RT(op1, i, t2) \
	(ivm_binop_table_get(IVM_OBJECT_GET_BINOP_R((op1), (i)), (t2)))

#define IVM_OBJECT_GET_UNIOP_PROC(op1, op) \
	(ivm_uniop_table_get(IVM_OBJECT_GET_UNIOP(op1), IVM_UNIOP_ID(op)))

#define IVM_OBJECT_GET_UNIOP_PROC_R(op1, op) \
	(ivm_uniop_table_get(IVM_OBJECT_GET_UNIOP(op1), (op)))

#define ivm_object_markOop(obj) ((obj)->mark.sub.oop = IVM_TRUE)
#define ivm_object_hasOop(obj) ((obj)->mark.sub.oop)

IVM_INLINE
ivm_bool_t
ivm_object_isBTProto(ivm_object_t *obj)
{
	return ivm_type_getProto(obj->type) == obj;
}

IVM_INLINE
void
ivm_object_destruct(ivm_object_t *obj,
					struct ivm_vmstate_t_tag *state)
{
	ivm_destructor_t des;

	if (obj && obj->type && (des = IVM_OBJECT_GET(obj, TYPE_DES))) {
		des(obj, state);
	}

	return;
}

ivm_bool_t
ivm_object_toBool(ivm_object_t *obj,
				  struct ivm_vmstate_t_tag *state);

void
ivm_object_setSlot(ivm_object_t *obj,
				   struct ivm_vmstate_t_tag *state,
				   const ivm_string_t *key,
				   ivm_object_t *value);

void
ivm_object_setSlot_r(ivm_object_t *obj,
					 struct ivm_vmstate_t_tag *state,
					 const ivm_char_t *rkey,
					 ivm_object_t *value);

void
ivm_object_setSlot_cc(ivm_object_t *obj,
					  struct ivm_vmstate_t_tag *state,
					  const ivm_string_t *key,
					  ivm_object_t *value,
					  ivm_instr_t *instr);

ivm_bool_t
ivm_object_setEmptySlot_r(ivm_object_t *obj,
						  struct ivm_vmstate_t_tag *state,
						  const ivm_char_t *rkey,
						  ivm_object_t *value);

ivm_bool_t
ivm_object_setExistSlot(ivm_object_t *obj,
						struct ivm_vmstate_t_tag *state,
						const ivm_string_t *key,
						ivm_object_t *value);

ivm_bool_t
ivm_object_setExistSlot_cc(ivm_object_t *obj,
						   struct ivm_vmstate_t_tag *state,
						   const ivm_string_t *key,
						   ivm_object_t *value,
						   ivm_instr_t *instr);

void
ivm_object_expandSlotTable(ivm_object_t *obj,
						   struct ivm_vmstate_t_tag *state,
						   ivm_size_t size);

void
ivm_object_setOop(ivm_object_t *obj,
				  struct ivm_vmstate_t_tag *state,
				  ivm_int_t op,
				  ivm_object_t *func);

ivm_object_t *
ivm_object_getSlot(ivm_object_t *obj,
				   struct ivm_vmstate_t_tag *state,
				   const ivm_string_t *key);

ivm_object_t *
ivm_object_getSlot_r(ivm_object_t *obj,
					 struct ivm_vmstate_t_tag *state,
					 const ivm_char_t *rkey);

ivm_object_t *
ivm_object_getSlot_cc(ivm_object_t *obj,
					  struct ivm_vmstate_t_tag *state,
					  const ivm_string_t *key,
					  ivm_instr_t *instr);

ivm_object_t *
ivm_object_getOop(ivm_object_t *obj,
				  struct ivm_vmstate_t_tag *state,
				  ivm_int_t oop);

ivm_object_t *
ivm_object_getDefaultOop(ivm_object_t *obj,
						 struct ivm_vmstate_t_tag *state,
						 ivm_int_t oop);

ivm_object_t *
ivm_object_doBinOpFallBack(ivm_object_t *obj, struct ivm_vmstate_t_tag *state,
						   struct ivm_coro_t_tag *coro, ivm_int_t op, ivm_int_t oop_id,
						   ivm_bool_t is_cmp, ivm_object_t *op2);

ivm_object_t *
ivm_object_doTriOpFallBack(ivm_object_t *obj, struct ivm_vmstate_t_tag *state,
						   struct ivm_coro_t_tag *coro, ivm_int_t op, ivm_int_t oop_id,
						   ivm_object_t *op2, ivm_object_t *op3);

#define IVM_AS(obj, type) ((type *)(obj))
#define IVM_AS_OBJ(obj) ((ivm_object_t *)(obj))

IVM_COM_END

#endif
