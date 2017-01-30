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
			ivm_int_t dummy1;                                  \
			ivm_int_t dummy2: 32 - _IVM_MARK_HEADER_BITS;      \
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
struct ivm_function_t_tag;

typedef void (*ivm_destructor_t)(struct ivm_object_t_tag *, struct ivm_vmstate_t_tag *);
typedef void (*ivm_traverser_t)(struct ivm_object_t_tag *, struct ivm_traverser_arg_t_tag *);
typedef ivm_bool_t (*ivm_bool_converter_t)(struct ivm_object_t_tag *, struct ivm_vmstate_t_tag *);
typedef void (*ivm_cloner_t)(struct ivm_object_t_tag *, struct ivm_vmstate_t_tag *);

typedef struct ivm_type_t_tag {
	ivm_binop_table_t binops;
	ivm_uniop_table_t uniops;
	struct ivm_function_t_tag *def_oops[IVM_OOP_COUNT];

	struct ivm_function_t_tag *cons; // constructor
	ivm_int_t *uid;

	ivm_destructor_t des;
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

#define ivm_type_getDefaultOop(type, op) ((type)->def_oops[op])
#define ivm_type_setDefaultOop(type, op, func) ((type)->def_oops[op] = (func))

#define ivm_type_getCons(type) ((type)->cons)
#define ivm_type_getUID(type) ((type)->uid)

#define ivm_type_checkUID(type, id) ((type)->uid = (id))

#define ivm_type_isBuiltin(type) ((type)->is_builtin)

// #define ivm_type_setBinopTable(type, op, table) ((type)->binops[IVM_BINOP_ID(op)] = (table))
#define ivm_type_getBinopTable(type) ((type)->binops)
#define ivm_type_getUniopTable(type) ((type)->uniops)

/* use in static initialization */
#define IVM_TPTYPE_BUILD(n, s, c, u, ...) \
	{                                       \
		.tag = -1,                          \
		.name = (n),                        \
		.size = (s),                        \
		.cons = (c),                        \
		.uid = (u),                         \
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

typedef ivm_pthash_t ivm_type_pool_t;
typedef ivm_pthash_iterator_t ivm_type_pool_iterator_t;

#define ivm_type_pool_init(pool) ivm_pthash_init(pool)

#define ivm_type_pool_register(pool, key, type) \
	ivm_type_pool_register_c((pool), (void *)(key), (type))

IVM_INLINE
ivm_type_t *
ivm_type_pool_register_c(ivm_type_pool_t *pool,
						 void *key,
						 ivm_type_t *type)
{
	ivm_type_t *copy = ivm_type_new(*type);

	if (!ivm_pthash_insertEmpty(pool, key, (void *)copy)) {
		ivm_type_free(copy);
		return IVM_NULL;
	}

	return copy;
}

#define ivm_type_pool_get(pool, key) ((ivm_type_t *)ivm_pthash_find((pool), (void *)(key)))

#define IVM_TYPE_POOL_ITER_GET(iter) ((ivm_type_t *)IVM_PTHASH_ITER_GET_VAL(iter))
#define IVM_TYPE_POOL_EACHPTR(pool, iter) IVM_PTHASH_EACHPTR((pool), iter)

IVM_INLINE
void
ivm_type_pool_dump(ivm_type_pool_t *pool)
{
	ivm_type_pool_iterator_t iter;
	ivm_type_t *type;

	IVM_TYPE_POOL_EACHPTR(pool, iter) {
		type = IVM_TYPE_POOL_ITER_GET(iter);
		ivm_type_free(type);
	}

	ivm_pthash_dump(pool);

	return;
}

IVM_COM_END

#endif
