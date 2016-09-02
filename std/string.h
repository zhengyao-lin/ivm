#ifndef _IVM_STD_STRING_H_
#define _IVM_STD_STRING_H_

#include <string.h>

#include "pub/com.h"
#include "pub/const.h"
#include "pub/type.h"

#include "mem.h"
#include "conv.h"
#include "list.h"
#include "heap.h"
#include "ref.h"
#include "hash.h"

IVM_COM_HEADER

struct ivm_vmstate_t_tag;
struct ivm_heap_t_tag;
struct ivm_string_pool_t_tag;

#define IVM_STRDUP ivm_strdup
#define IVM_STRCMP strcmp
#define IVM_STRLEN strlen
#define IVM_STRDUP_STATE(str, state) (ivm_strdup_state((str), (state)))
#define IVM_STRDUP_HEAP(str, heap) (ivm_strdup_heap((str), (heap)))

ivm_char_t *
IVM_STRNDUP(const ivm_char_t *str,
			ivm_size_t len);

IVM_INLINE
ivm_int_t
IVM_STRNCMP(const ivm_char_t *a, ivm_size_t alen,
			const ivm_char_t *b, ivm_size_t blen)
{
	if (alen != blen) return alen - blen;
	return STD_MEMCMP(a, b, alen);
}

ivm_char_t *
ivm_strdup(const ivm_char_t *src);

ivm_char_t *
ivm_strdup_state(const ivm_char_t *src,
				 struct ivm_vmstate_t_tag *state);

ivm_char_t *
ivm_strdup_heap(const ivm_char_t *src,
				ivm_heap_t *heap);

typedef struct {
	ivm_uint_t len;

	ivm_uint_t is_const: 1;
	ivm_uint_t is_ascii: 1;
	ivm_uint_t wlen: sizeof(ivm_uint_t) * 8 - 2; // length of wide char str(display length)

	ivm_char_t cont[];
} IVM_NOALIGN ivm_string_t;

#define IVM_STRING_GET_SIZE(len) \
	(sizeof(ivm_string_t) + sizeof(ivm_char_t) * ((len) + 1))

#define IVM_STRING_MAX_LEN IVM_UINT_MAX(ivm_uint_t)

#define IVM_STRING_LEGAL_LEN(len) \
	((len) < IVM_STRING_MAX_LEN)

IVM_INLINE
void
ivm_string_initHead(ivm_string_t *str,
					ivm_bool_t is_const,
					ivm_size_t len)
{
	str->len = len;
	str->is_const = is_const;
	str->is_ascii = IVM_FALSE;
	str->wlen = 0;
	
	return;
}

const ivm_string_t *
ivm_string_copyNonConst(const ivm_string_t *str,
						ivm_heap_t *heap);

#define ivm_string_length(str) \
	((str)->len)

#define ivm_string_isConst(str) \
	((str)->is_const)

IVM_INLINE
ivm_size_t
ivm_string_realLength(const ivm_string_t *str)
{
	ivm_string_t *head = (ivm_string_t *)str;

	if (str->is_ascii) {
		return str->len;
	} else if (str->wlen) {
		return str->wlen;
	}

	// gen cache
	head->is_ascii = ivm_conv_isAllAscii(str->cont);
	return head->wlen = ivm_conv_mbstowcs_len(str->cont);
}

#define ivm_string_size(str) \
	(IVM_STRING_GET_SIZE((str)->len))

#define ivm_string_trimHead(str) \
	((str)->cont)

ivm_hash_val_t
ivm_string_hash(const ivm_string_t *str);

IVM_INLINE
ivm_bool_t
ivm_string_compare(const ivm_string_t *a,
				   const ivm_string_t *b)
{
	return (a == b)
		   || ((ivm_string_length(a) == ivm_string_length(b))
				&& (!STD_MEMCMP(ivm_string_trimHead(a),
								 ivm_string_trimHead(b),
								 ivm_string_length(a))));
}

IVM_INLINE
ivm_int_t
ivm_string_compare_c(const ivm_string_t *a,
					 const ivm_string_t *b)
{
	if (a == b) return 0;
	return IVM_STRCMP(ivm_string_trimHead(a), ivm_string_trimHead(b));
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
	struct ivm_heap_t_tag *heap;

	ivm_size_t size;
	const ivm_string_t **table;

	IVM_REF_HEADER
	ivm_bool_t is_fixed;
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
	STD_FREE(pool->table);
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
