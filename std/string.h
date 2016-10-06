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
#include "enc.h"

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
	ivm_uint_t is_const: 1;
	ivm_uint_t is_ascii: 1;
	ivm_uint_t ulen: sizeof(ivm_uint_t) * 8 - 2; // length of utf-8 decoded length

	ivm_uint_t len;

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
	str->is_const = is_const;
	str->is_ascii = IVM_FALSE;
	str->ulen = 0;

	str->len = len;
	
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
ivm_string_utf8Length(const ivm_string_t *str)
{
	ivm_string_t *head = (ivm_string_t *)str;
	ivm_size_t len;

	if (str->is_ascii ||
		str->ulen == (ivm_uint_t)-1) {
		return str->len;
	} else if (str->ulen) {
		return str->ulen;
	}

	// gen cache
	head->is_ascii = ivm_enc_isAllAscii_n(str->cont, str->len);
	head->ulen = len = ivm_enc_utf8_strlen_n(str->cont, str->len);

	if (len == -1) return str->len;
	return len;
}

#define ivm_string_size(str) \
	(IVM_STRING_GET_SIZE((str)->len))

#define ivm_string_trimHead(str) \
	((str)->cont)

#define ivm_string_charAt(str, i) \
	((str)->cont[i])

IVM_INLINE
ivm_hash_val_t
ivm_string_hash(const ivm_string_t *str)
{
	return ivm_hash_fromString_c(str->cont, ivm_string_length(str));
}

IVM_INLINE
ivm_bool_t
ivm_string_compare(const ivm_string_t *a,
				   const ivm_string_t *b)
{
	return (a == b) ||
		   ((a->len == b->len) &&
		   	(!STD_MEMCMP(a->cont, b->cont, a->len)));
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
typedef IVM_PTLIST_ITER_TYPE(const ivm_string_t *) ivm_string_list_iterator_t;

#define ivm_string_list_init(list) ivm_ptlist_init_c((list), IVM_DEFAULT_STRING_POOL_BUFFER_SIZE)
#define ivm_string_list_init_t(list, init, size) ivm_ptlist_init_t((list), (void **)(init), (size))
#define ivm_string_list_dump ivm_ptlist_dump

#define ivm_string_list_push(list, str) ivm_ptlist_push((list), (void *)(str))
#define ivm_string_list_size ivm_ptlist_size
#define ivm_string_list_core(list) ((const ivm_string_t **)ivm_ptlist_core(list))

#define ivm_string_list_at(list, i) ((ivm_string_t *)ivm_ptlist_at((list), (i)))

#define IVM_STRING_LIST_ITER_SET(iter, val) (IVM_PTLIST_ITER_SET((iter), (val)))
#define IVM_STRING_LIST_ITER_GET(iter) ((const ivm_string_t *)IVM_PTLIST_ITER_GET(iter))
#define IVM_STRING_LIST_ITER_GET(iter) ((const ivm_string_t *)IVM_PTLIST_ITER_GET(iter))
#define IVM_STRING_LIST_EACHPTR(list, iter) IVM_PTLIST_EACHPTR((list), iter, const ivm_string_t *)

typedef ivm_size_t ivm_string_id_t;

typedef struct {
	const ivm_string_t *k;
	ivm_string_id_t v;
} ivm_string_pos_t;

typedef struct ivm_string_pool_t_tag {
	ivm_heap_t *heap;

	ivm_string_list_t lst;
	ivm_size_t size;
	ivm_size_t mask;
	ivm_string_pos_t *table; // hash table

	IVM_REF_HEADER
} ivm_string_pool_t;

ivm_string_pool_t *
ivm_string_pool_new();

void
ivm_string_pool_init(ivm_string_pool_t *pool);

void
ivm_string_pool_dump(ivm_string_pool_t *pool);

// given a string list and the heap it was allocated in
// return a new string pool generated by the given strings(usually used in serialization)
ivm_string_pool_t *
ivm_string_pool_new_t(const ivm_string_t **lst,
					  ivm_size_t count,
					  ivm_heap_t *heap);

void
ivm_string_pool_free(ivm_string_pool_t *pool);

#define ivm_string_pool_size(pool) ivm_string_list_size(&(pool)->lst)
#define ivm_string_pool_core(pool) ivm_string_list_core(&(pool)->lst)
#define ivm_string_pool_get(pool, i) ivm_string_list_at(&(pool)->lst, (i))

const ivm_string_t *
ivm_string_pool_register(ivm_string_pool_t *pool,
						 const ivm_string_t *str);

const ivm_string_t *
ivm_string_pool_registerRaw(ivm_string_pool_t *pool,
							const ivm_char_t *str);

ivm_string_id_t
ivm_string_pool_registerRaw_i(ivm_string_pool_t *pool,
							  const ivm_char_t *str);

const ivm_string_t *
ivm_string_pool_registerRaw_n(ivm_string_pool_t *pool,
							  const ivm_char_t *str,
							  ivm_size_t len);

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
