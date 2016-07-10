#ifndef _IVM_STD_STRING_H_
#define _IVM_STD_STRING_H_

#include <string.h>

#include "pub/com.h"
#include "pub/const.h"
#include "pub/type.h"
#include "pub/mem.h"

#include "list.h"
#include "heap.h"
#include "ref.h"

IVM_COM_HEADER

struct ivm_vmstate_t_tag;
struct ivm_heap_t_tag;
struct ivm_string_pool_t_tag;

#define IVM_STRDUP ivm_strdup
#define IVM_STRCMP strcmp
#define IVM_STRLEN strlen
#define IVM_STRDUP_STATE(str, state) (ivm_strdup_state((str), (state)))
#define IVM_STRDUP_HEAP(str, heap) (ivm_strdup_heap((str), (heap)))

IVM_INLINE
ivm_int_t
IVM_STRNCMP(const ivm_char_t *a, ivm_size_t alen,
			const ivm_char_t *b, ivm_size_t blen)
{
	if (alen != blen) return alen - blen;
	return MEM_COMPARE(a, b, alen);
}

ivm_char_t *
ivm_strdup(const ivm_char_t *src);
ivm_char_t *
ivm_strdup_state(const ivm_char_t *src,
				 struct ivm_vmstate_t_tag *state);
ivm_char_t *
ivm_strdup_heap(const ivm_char_t *src,
				struct ivm_heap_t_tag *heap);

typedef struct {
	ivm_bool_t is_const;
	ivm_uint_t len;
} ivm_string_t;

#define IVM_STRING_GET_SIZE(len) \
	(sizeof(ivm_string_t) + sizeof(ivm_char_t) * ((len) + 1))

#define IVM_STRING_MAX_LEN IVM_UINT_MAX(ivm_uint_t)

#define IVM_STRING_LEGAL_LEN(len) \
	((len) < IVM_STRING_MAX_LEN)

ivm_string_t *
ivm_string_new_state(ivm_bool_t is_const,
					 const ivm_char_t *str,
					 struct ivm_vmstate_t_tag *state);

ivm_string_t *
ivm_string_new_heap(ivm_bool_t is_const,
					const ivm_char_t *str,
					struct ivm_heap_t_tag *heap);

void
ivm_string_initHead(ivm_string_t *str,
					ivm_bool_t is_const,
					ivm_size_t len);

const ivm_string_t *
ivm_string_copyIfNotConst_state(const ivm_string_t *str,
								struct ivm_vmstate_t_tag *state);

const ivm_string_t *
ivm_string_copyIfNotConst_heap(const ivm_string_t *str,
							   struct ivm_heap_t_tag *heap);

/* if the length of string is bigger than IVM_DEFAULT_CONST_THRESHOLD, alloc in the state; else in the pool */
const ivm_string_t *
ivm_string_copyIfNotConst_pool(const ivm_string_t *str,
							   struct ivm_vmstate_t_tag *state);

#define ivm_string_length(str) \
	((str)->len)

#define ivm_string_size(str) \
	(IVM_STRING_GET_SIZE((str)->len))

#define ivm_string_trimHead(str) \
	((ivm_char_t *)(((ivm_string_t *)str) + 1))

IVM_INLINE
ivm_bool_t
ivm_string_compare(const ivm_string_t *a,
				   const ivm_string_t *b)
{
	return (a == b)
		   || ((ivm_string_length(a) == ivm_string_length(b))
				&& (!MEM_COMPARE(ivm_string_trimHead(a),
								 ivm_string_trimHead(b),
								 ivm_string_length(a))));
}

ivm_int_t /* same as strcmp */
ivm_string_compareToRaw(const ivm_string_t *a,
						const ivm_char_t *b);

ivm_int_t
ivm_string_compareToRaw_n(const ivm_string_t *a,
						  const ivm_char_t *b,
						  ivm_size_t len);

typedef ivm_ptlist_t ivm_string_list_t;

#define ivm_string_list_new ivm_ptlist_new
#define ivm_string_list_free ivm_ptlist_free

#define ivm_string_list_register ivm_ptlist_push

#define ivm_string_list_at(list, i) ((ivm_string_t *)ivm_ptlist_at((list), (i)))
#define ivm_string_list_indexOf(list, str) (ivm_ptlist_indexOf((list), (void *)(str), ivm_string_compareToRaw))

typedef struct ivm_string_pool_t_tag {
	IVM_REF_HEADER

	struct ivm_heap_t_tag *heap;

	ivm_bool_t is_fixed;
	ivm_size_t size;
	const ivm_string_t **table;
} ivm_string_pool_t;

ivm_string_pool_t *
ivm_string_pool_new(ivm_bool_t is_fixed);

void
ivm_string_pool_free(ivm_string_pool_t *pool);

ivm_ptr_t
ivm_string_pool_register(ivm_string_pool_t *pool,
						 const ivm_string_t *str);

/* no copy */
ivm_ptr_t
ivm_string_pool_register_nc(ivm_string_pool_t *pool,
							const ivm_string_t *str);

ivm_ptr_t
ivm_string_pool_registerRaw(ivm_string_pool_t *pool,
							const ivm_char_t *str);

ivm_ptr_t
ivm_string_pool_registerRaw_n(ivm_string_pool_t *pool,
							  const ivm_char_t *str,
							  ivm_size_t len);

#define ivm_string_pool_get(pool, i) ((pool)->table[i])

#define ivm_string_pool_isFixed(pool) ((pool)->is_fixed)
#define ivm_string_pool_size(pool) ((pool)->size)
#define ivm_string_pool_table(pool) ((pool)->table)
#define ivm_string_pool_setSize(pool, val) ((pool)->size = (val))

IVM_INLINE
void
ivm_string_pool_setTable(ivm_string_pool_t *pool,
						 const ivm_string_t **table)
{
	MEM_FREE(pool->table);
	pool->table = table;
	return;
}

#define ivm_string_pool_isIllegalSize(pool, size) \
	(ivm_heap_isIllegalSize((pool)->heap, (size)))

#define ivm_string_pool_alloc(pool, size) \
	(ivm_heap_alloc((pool)->heap, (size)))

IVM_INLINE
void *
ivm_string_pool_alloc_s(ivm_string_pool_t *pool,
						ivm_size_t size)
{
	if (ivm_string_pool_isIllegalSize(pool, size)) {
		return ivm_string_pool_alloc(pool, size);
	}

	return IVM_NULL;
}

IVM_INLINE
ivm_size_t
ivm_string_pool_find(ivm_string_pool_t *pool,
					 const ivm_string_t *str)
{
	const ivm_string_t **i, **end;

	for (i = pool->table,
		 end = i + pool->size;
		 i != end; i++) {
		if (*i == str)
			return ((ivm_ptr_t)i - (ivm_ptr_t)pool->table) / sizeof(*i);
	}

	return -1;
}

IVM_INLINE
const ivm_string_t *
ivm_string_pool_store(ivm_string_pool_t *pool,
					  const ivm_char_t *str)
{
	ivm_size_t i = ivm_string_pool_registerRaw(pool, str);
	return ivm_string_pool_get(pool, i);
}

typedef ivm_ptlist_t ivm_string_pool_list_t;
typedef IVM_PTLIST_ITER_TYPE(ivm_string_pool_t *) ivm_string_pool_list_iterator_t;

#define ivm_string_pool_list_new ivm_ptlist_new

IVM_INLINE
ivm_size_t
ivm_string_pool_list_register(ivm_string_pool_list_t *list,
							  ivm_string_pool_t *pool)
{
	ivm_ref_inc(pool);
	return ivm_ptlist_push(list, pool);
}

#define ivm_string_pool_list_find ivm_ptlist_find
#define ivm_string_pool_list_size ivm_ptlist_size
#define ivm_string_pool_list_at ivm_ptlist_at

#define IVM_STRING_POOL_LIST_ITER_SET(iter, val) IVM_PTLIST_ITER_SET((iter), (val))
#define IVM_STRING_POOL_LIST_ITER_GET(iter) IVM_PTLIST_ITER_GET(iter)
#define IVM_STRING_POOL_LIST_EACHPTR(list, iter) IVM_PTLIST_EACHPTR((list), iter, ivm_string_pool_t *)

IVM_INLINE
void
ivm_string_pool_list_free(ivm_string_pool_list_t *list)
{
	ivm_string_pool_list_iterator_t siter;

	if (list) {
		IVM_STRING_POOL_LIST_EACHPTR(list, siter) {
			ivm_string_pool_free(
				IVM_STRING_POOL_LIST_ITER_GET(siter)
			);
		}
		ivm_ptlist_free(list);
	}

	return;
}

IVM_COM_END

#endif
