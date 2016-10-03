#ifndef _IVM_VM_TYPE_H_
#define _IVM_VM_TYPE_H_

#include "pub/com.h"
#include "pub/type.h"
#include "pub/const.h"

#include "std/string.h"
#include "std/list.h"

#include "slot.h"
#include "oprt.h"

IVM_COM_HEADER

#define _IVM_MARK_HEADER_BITS 4 // (2 + IVM_OOP_COUNT)

#define IVM_OBJECT_HEADER \
	struct ivm_type_t_tag *type;                               \
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

struct ivm_vmstate_t_tag;
struct ivm_object_t_tag;
struct ivm_traverser_arg_t_tag;

typedef void (*ivm_destructor_t)(struct ivm_object_t_tag *, struct ivm_vmstate_t_tag *);
typedef void (*ivm_traverser_t)(struct ivm_object_t_tag *, struct ivm_traverser_arg_t_tag *);
typedef ivm_bool_t (*ivm_bool_converter_t)(struct ivm_object_t_tag *, struct ivm_vmstate_t_tag *);
typedef void (*ivm_cloner_t)(struct ivm_object_t_tag *, struct ivm_vmstate_t_tag *);

typedef struct ivm_type_t_tag {
	ivm_binop_table_t binops[IVM_BINOP_COUNT];
	ivm_uniop_table_t uniops;

	ivm_destructor_t des;
	ivm_native_function_t cons;
	ivm_traverser_t trav;

	ivm_bool_converter_t to_bool;
	ivm_cloner_t clone;

	const ivm_char_t *name;
	ivm_size_t size;

	// ivm_type_t *init_type;
	// struct ivm_object_t_tag *init_proto; /* default prototype */
	struct {
		IVM_OBJECT_HEADER
	} header;

	ivm_type_tag_t tag;

	ivm_bool_t const_bool; /* if to_bool is null, this is the value returned */
	ivm_bool_t is_builtin;
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
#define ivm_type_setProto(type, p) ((type)->header.proto = (p))
#define ivm_type_getProto(type) ((type)->header.proto)
#define ivm_type_getHeader(type) (&(type)->header)

#define ivm_type_getName(type) ((type)->name)

#define ivm_type_getCons(type) ((type)->cons)
#define ivm_type_checkCons(type, val) ((type)->cons == (val))

#define ivm_type_isBuiltin(type) ((type)->is_builtin)

// #define ivm_type_setBinopTable(type, op, table) ((type)->binops[IVM_BINOP_ID(op)] = (table))
#define ivm_type_getBinopTable(type, op) ((type)->binops + IVM_BINOP_ID(op))
#define ivm_type_getBinopTable_r(type, i) ((type)->binops + (i))

#define ivm_type_getUniopTable(type) ((type)->uniops)

/* use in static initialization */
#define IVM_TPTYPE_BUILD(n, s, c, ...) \
	{                                       \
		.tag = -1,                          \
		.name = #n,                         \
		.size = (s),                        \
		.cons = (c),                        \
		.is_builtin = IVM_FALSE,            \
		__VA_ARGS__                         \
	}

typedef void (*ivm_type_init_proc_t)(ivm_type_t *, struct ivm_vmstate_t_tag *);

typedef ivm_ptlist_t ivm_type_list_t;
typedef IVM_PTLIST_ITER_TYPE(ivm_type_t *) ivm_type_list_iterator_t;

#define ivm_type_list_new() (ivm_ptlist_new_c(IVM_DEFAULT_TYPE_LIST_BUFFER_SIZE))
#define ivm_type_list_free ivm_ptlist_free
#define ivm_type_list_init(list) (ivm_ptlist_init_c((list), IVM_DEFAULT_TYPE_LIST_BUFFER_SIZE))
#define ivm_type_list_dump ivm_ptlist_dump
#define ivm_type_list_register ivm_ptlist_push
#define ivm_type_list_insert ivm_ptlist_insert
#define ivm_type_list_size ivm_ptlist_size
#define ivm_type_list_has ivm_ptlist_has
#define ivm_type_list_at(list, i) ((ivm_type_t *)ivm_ptlist_at((list), (i)))

#define IVM_TYPE_LIST_ITER_SET(iter, val) (IVM_PTLIST_ITER_SET((iter), (val)))
#define IVM_TYPE_LIST_ITER_GET(iter) ((ivm_type_t *)IVM_PTLIST_ITER_GET(iter))
#define IVM_TYPE_LIST_EACHPTR(list, iter) IVM_PTLIST_EACHPTR((list), iter, ivm_type_t *)

typedef struct {
	ivm_string_pool_t tnames;
	ivm_type_list_t types;
} ivm_type_pool_t;

void
ivm_type_pool_init(ivm_type_pool_t *pool);

void
ivm_type_pool_dump(ivm_type_pool_t *pool);

IVM_INLINE
ivm_type_t *
ivm_type_pool_register(ivm_type_pool_t *pool,
					   const ivm_char_t *name,
					   ivm_type_t *init)
{
	ivm_string_id_t id = ivm_string_pool_registerRaw_i(&pool->tnames, name);
	ivm_type_t *copy;

	if (ivm_type_list_has(&pool->types, id) &&
		ivm_type_list_at(&pool->types, id)) {
		// conflict types
		return IVM_NULL;
	}

	copy = ivm_type_new(*init);
	ivm_type_list_insert(&pool->types, id, copy);

	return copy;
}

IVM_INLINE
ivm_type_t *
ivm_type_pool_get(ivm_type_pool_t *pool,
				  const ivm_char_t *name)
{
	ivm_string_id_t id = ivm_string_pool_registerRaw_i(&pool->tnames, name);

	if (ivm_type_list_has(&pool->types, id)) {
		return ivm_type_list_at(&pool->types, id);
	}

	return IVM_NULL;
}

#define IVM_TYPE_POOL_ITER_SET IVM_TYPE_LIST_ITER_SET
#define IVM_TYPE_POOL_ITER_GET IVM_TYPE_LIST_ITER_GET
#define IVM_TYPE_POOL_EACHPTR(pool, iter) IVM_PTLIST_EACHPTR(&(pool)->types, iter, ivm_type_t *)

IVM_COM_END

#endif
