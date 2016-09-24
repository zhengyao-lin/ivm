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

IVM_COM_HEADER

#define _IVM_MARK_HEADER_BITS 4 // (2 + IVM_OOP_COUNT)

#define IVM_OBJECT_HEADER \
	ivm_type_t *type;                                          \
	struct ivm_object_t_tag *proto;                            \
	ivm_slot_table_t *slots;                                   \
	union {                                                    \
		struct {                                               \
			ivm_int_t dummy1: sizeof(ivm_sint64_t) / 2 * 8;    \
			ivm_int_t dummy2:                                  \
				sizeof(ivm_sint64_t) / 2 * 8 -                 \
				_IVM_MARK_HEADER_BITS;                         \
			/* ivm_uint_t oop: IVM_OOP_COUNT; */               \
			/* need or not to check oop */                     \
			ivm_uint_t oop: 1;                                 \
			ivm_uint_t locked: 1;                              \
			ivm_uint_t wb: 1;                                  \
			ivm_uint_t gen: 1;                                 \
		} sub;                                                 \
		struct ivm_object_t_tag *copy;                         \
	} mark;

struct ivm_object_t_tag;
struct ivm_vmstate_t_tag;
struct ivm_function_t_tag;
struct ivm_collector_t_tag;
struct ivm_traverser_arg_t_tag;

typedef void (*ivm_destructor_t)(struct ivm_object_t_tag *, struct ivm_vmstate_t_tag *);
typedef void (*ivm_traverser_t)(struct ivm_object_t_tag *, struct ivm_traverser_arg_t_tag *);
typedef ivm_bool_t (*ivm_bool_converter_t)(struct ivm_object_t_tag *, struct ivm_vmstate_t_tag *);
typedef void (*ivm_cloner_t)(struct ivm_object_t_tag *, struct ivm_vmstate_t_tag *);

typedef struct ivm_type_t_tag {
	ivm_binop_table_t binops[IVM_BINOP_COUNT];
	ivm_uniop_table_t uniops;

	ivm_destructor_t des;
	ivm_traverser_t trav;

	ivm_bool_converter_t to_bool;
	ivm_cloner_t clone;

	const ivm_char_t *name;
	ivm_size_t size;
	struct ivm_object_t_tag *proto; /* default prototype */

	ivm_type_tag_t tag;

	ivm_bool_t const_bool; /* if to_bool is null, this is the value returned */
} ivm_type_t;

ivm_type_t *
ivm_type_new(ivm_type_t type);

void
ivm_type_free(ivm_type_t *type);

void
ivm_type_init(ivm_type_t *type, ivm_type_t *src);

void
ivm_type_dump(ivm_type_t *type);

#define ivm_type_setTag(type, t) ((type)->tag = (t))
#define ivm_type_setProto(type, p) ((type)->proto = (p))
#define ivm_type_getProto(type) ((type)->proto)

// #define ivm_type_setBinopTable(type, op, table) ((type)->binops[IVM_BINOP_ID(op)] = (table))
#define ivm_type_getBinopTable(type, op) ((type)->binops + IVM_BINOP_ID(op))
#define ivm_type_getBinopTable_r(type, i) ((type)->binops + (i))

#define ivm_type_getUniopTable(type) ((type)->uniops)

#define ivm_type_setHeader(type, p) ((type)->header = (p))
#define ivm_type_getHeader(type) ((type)->header)

typedef void (*ivm_type_init_proc_t)(ivm_type_t *, struct ivm_vmstate_t_tag *);

typedef ivm_ptlist_t ivm_type_list_t;
typedef IVM_PTLIST_ITER_TYPE(ivm_type_t *) ivm_type_list_iterator_t;

#define ivm_type_list_new() (ivm_ptlist_new_c(IVM_DEFAULT_TYPE_LIST_BUFFER_SIZE))
#define ivm_type_list_free ivm_ptlist_free
#define ivm_type_list_init(list) (ivm_ptlist_init_c((list), IVM_DEFAULT_TYPE_LIST_BUFFER_SIZE))
#define ivm_type_list_dump ivm_ptlist_dump
#define ivm_type_list_register ivm_ptlist_push
#define ivm_type_list_size ivm_ptlist_size
#define ivm_type_list_at(list, i) ((ivm_type_t *)ivm_ptlist_at((list), (i)))

#define IVM_TYPE_LIST_ITER_SET(iter, val) (IVM_PTLIST_ITER_SET((iter), (val)))
#define IVM_TYPE_LIST_ITER_GET(iter) ((ivm_type_t *)IVM_PTLIST_ITER_GET(iter))
#define IVM_TYPE_LIST_EACHPTR(list, iter) IVM_PTLIST_EACHPTR((list), iter, ivm_type_t *)

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
#define IVM_OBJECT_GET_COPY(obj) \
	((ivm_object_t *)(((ivm_uptr_t)(obj)->mark.copy << _IVM_MARK_HEADER_BITS) >> _IVM_MARK_HEADER_BITS))
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
	obj->mark.copy = (ivm_object_t *)
					 ((((ivm_uptr_t)obj->mark.copy
					 	>> (sizeof(ivm_ptr_t) * 8 - _IVM_MARK_HEADER_BITS))
					 	<< (sizeof(ivm_ptr_t) * 8 - _IVM_MARK_HEADER_BITS))
					 	| (ivm_uptr_t)copy);

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

/* call the operation proc when obj [op] obj(of type) e.g. obj + num */
#define IVM_OBJECT_GET_BINOP_PROC(op1, op, op2) \
	(ivm_binop_table_get(IVM_OBJECT_GET_BINOP((op1), op), \
						 IVM_OBJECT_GET((op2), TYPE_TAG)))

#define IVM_OBJECT_DO_BINOP_PROC(op1, op, op2) \
	(ivm_binop_table_get(IVM_OBJECT_GET_BINOP((op1), op), \
						 IVM_OBJECT_GET((op2), TYPE_TAG)))

#define IVM_OBJECT_GET_BINOP_PROC_R(op1, i, op2) \
	(ivm_binop_table_get(IVM_OBJECT_GET_BINOP_R((op1), (i)), \
						 IVM_OBJECT_GET((op2), TYPE_TAG)))

#define IVM_OBJECT_GET_UNIOP_PROC(op1, op) \
	(ivm_uniop_table_get(IVM_OBJECT_GET_UNIOP(op1), IVM_UNIOP_ID(op)))

#define IVM_OBJECT_GET_UNIOP_PROC_R(op1, op) \
	(ivm_uniop_table_get(IVM_OBJECT_GET_UNIOP(op1), (op)))

IVM_INLINE
ivm_object_t *
ivm_object_doBinOp_c(struct ivm_vmstate_t_tag *state,
					 struct ivm_coro_t_tag *coro,
					 ivm_object_t *op1,
					 ivm_int_t op,
					 ivm_object_t *op2)
{
	ivm_binop_proc_t proc = IVM_OBJECT_GET_BINOP_PROC_R(op1, op, op2);
	if (proc) {
		return proc(state, coro, op1, op2, IVM_NULL);
	}
	return IVM_NULL;
}

#define ivm_object_doBinOp(state, coro, op1, op, op2) \
	(ivm_object_doBinOp_c((state), (coro), (op1), IVM_BINOP_ID(op), (op2)))

#define ivm_object_markOop(obj) ((obj)->mark.sub.oop = IVM_TRUE)
#define ivm_object_hasOop(obj) ((obj)->mark.sub.oop)

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

#define ivm_object_setSlotIfExist(obj, state, key, value) \
	(ivm_slot_table_setSlotIfExist((obj)->slots, (state), (key), (value)))

#define ivm_object_setSlotIfExist_cc(obj, state, key, value, instr) \
	(ivm_slot_table_setSlotIfExist_cc((obj)->slots, (state), (key), (value), (instr)))

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

void
ivm_object_setOop(ivm_object_t *obj,
				  struct ivm_vmstate_t_tag *state,
				  ivm_int_t op,
				  ivm_object_t *func);

#if 0
/* no prototype */
ivm_object_t *
ivm_object_getSlotValue_np(ivm_object_t *obj,
						   struct ivm_vmstate_t_tag *state,
						   const ivm_char_t *key);
#endif

void
ivm_object_printSlots(ivm_object_t *obj);

#define IVM_AS(obj, type) ((type *)(obj))
#define IVM_AS_OBJ(obj) ((ivm_object_t *)(obj))

IVM_COM_END

#endif
