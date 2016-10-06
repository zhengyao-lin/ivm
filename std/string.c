#include "pub/err.h"
#include "pub/vm.h"
#include "pub/type.h"

#include "mem.h"
#include "conv.h"
#include "string.h"
#include "heap.h"
#include "hash.h"
#include "bit.h"

ivm_char_t *
IVM_STRNDUP(const ivm_char_t *str,
			ivm_size_t len)
{
	ivm_char_t *ret = STD_ALLOC(sizeof(*ret) * (len + 1));

	IVM_MEMCHECK(ret);

	STD_MEMCPY(ret, str, sizeof(*ret) * len);
	ret[len] = '\0';

	return ret;
}

ivm_char_t *
ivm_strdup(const ivm_char_t *src)
{
	ivm_size_t size = sizeof(ivm_char_t) * (IVM_STRLEN(src) + 1);
	ivm_char_t *ret = STD_ALLOC(size);

	IVM_MEMCHECK(ret);

	STD_MEMCPY(ret, src, size);

	return ret;
}

ivm_char_t *
ivm_strdup_state(const ivm_char_t *src,
				 ivm_vmstate_t *state)
{
	ivm_size_t size;
	ivm_char_t *ret = IVM_NULL;

	if (src) {
		size = sizeof(ivm_char_t) * (IVM_STRLEN(src) + 1);
		ret = ivm_vmstate_alloc(state, size);
		STD_MEMCPY(ret, src, size);
	}

	return ret;
}

ivm_char_t *
ivm_strdup_heap(const ivm_char_t *src,
				ivm_heap_t *heap)
{
	ivm_size_t size;
	ivm_char_t *ret = IVM_NULL;

	if (src) {
		size = sizeof(ivm_char_t) * (IVM_STRLEN(src) + 1);
		ret = ivm_heap_alloc(heap, size);
		
		STD_MEMCPY(ret, src, size);
	}

	return ret;
}

const ivm_string_t *
ivm_string_copyNonConst(const ivm_string_t *str,
						ivm_heap_t *heap)
{
	ivm_size_t size;
	ivm_string_t *ret;

	if (str && !str->is_const) {
		size = IVM_STRING_GET_SIZE(ivm_string_length(str));
		ret = ivm_heap_alloc(heap, size);

		STD_MEMCPY(ret, str, size);

		return ret;
	}

	return str;
}

ivm_int_t
ivm_string_compareToRaw(const ivm_string_t *a,
						const ivm_char_t *b)
{
	return IVM_STRCMP(a->cont, b);
}

ivm_int_t
ivm_string_compareToRaw_n(const ivm_string_t *a,
						  const ivm_char_t *b,
						  ivm_size_t len)
{
	return IVM_STRNCMP(a->cont,
					   ivm_string_length(a),
					   b, len);
}

IVM_INLINE
void
_ivm_string_pool_init_c(ivm_string_pool_t *pool)
{
	ivm_ref_init(pool);
	pool->heap = ivm_heap_new(IVM_DEFAULT_STRING_POOL_BLOCK_SIZE);
	
	ivm_string_list_init(&pool->lst);
	pool->size = IVM_DEFAULT_STRING_POOL_BUFFER_SIZE;
	pool->mask = pool->size - 1;
	pool->table
	= STD_ALLOC_INIT(sizeof(*pool->table) * IVM_DEFAULT_STRING_POOL_BUFFER_SIZE);

	IVM_MEMCHECK(pool->table);

	return;
}

void
ivm_string_pool_init(ivm_string_pool_t *pool)
{
	_ivm_string_pool_init_c(pool);
	return;
}

void
ivm_string_pool_dump(ivm_string_pool_t *pool)
{
	ivm_heap_free(pool->heap);
	ivm_string_list_dump(&pool->lst);
	STD_FREE(pool->table);
	return;
}

ivm_string_pool_t *
ivm_string_pool_new()
{
	ivm_string_pool_t *ret = STD_ALLOC(sizeof(*ret));

	IVM_MEMCHECK(ret);

	_ivm_string_pool_init_c(ret);

	return ret;
}

IVM_PRIVATE
const ivm_string_t *
_ivm_string_pool_rehash(ivm_string_pool_t *pool,
						const ivm_string_t *str,
						ivm_string_id_t id);

ivm_string_pool_t *
ivm_string_pool_new_t(const ivm_string_t **lst,
					  ivm_size_t count,
					  ivm_heap_t *heap)
{
	ivm_string_pool_t *ret = STD_ALLOC(sizeof(*ret));
	ivm_string_id_t id;
	ivm_string_list_iterator_t iter;

	IVM_MEMCHECK(ret);

	ivm_ref_init(ret);
	ret->heap = heap;
	
	ivm_string_list_init_t(&ret->lst, lst, count);
	ret->size = count << 2;
	ret->mask = ret->size - 1;
	ret->table
	= STD_ALLOC_INIT(sizeof(*ret->table) * ret->size);

	IVM_MEMCHECK(ret->table);

	id = 0;
	IVM_STRING_LIST_EACHPTR(&ret->lst, iter) {
		_ivm_string_pool_rehash(ret, IVM_STRING_LIST_ITER_GET(iter), id);
		id++;
	}

	return ret;
}

void
ivm_string_pool_free(ivm_string_pool_t *pool)
{
	if (pool && !ivm_ref_dec(pool)) {
		ivm_heap_free(pool->heap);
		ivm_string_list_dump(&pool->lst);
		STD_FREE(pool->table);
		STD_FREE(pool);
	}

	return;
}

IVM_PRIVATE
ivm_size_t /* next empty index */
_ivm_string_pool_expand(ivm_string_pool_t *pool)
{
	ivm_size_t ret = 0;
	ivm_string_pos_t *otable;
	ivm_string_list_iterator_t iter;
	ivm_string_id_t id;

	otable = pool->table;

	pool->size <<= 1;
	pool->mask = pool->size - 1;
	pool->table = STD_ALLOC_INIT(sizeof(*pool->table) * pool->size);

	id = 0;
	IVM_STRING_LIST_EACHPTR(&pool->lst, iter) {
		_ivm_string_pool_rehash(pool, IVM_STRING_LIST_ITER_GET(iter), id);
		id++;
	}

	STD_FREE(otable);

	return ret;
}

#define CHECK_CONFLICT() \
	if (conflict >= IVM_DEFAULT_STRING_POOL_MAX_CONF_COUNT) {       \
		_ivm_string_pool_expand(pool);                              \
		goto AGAIN;                                                 \
	}

#define CHECK(cmp, copy) \
	if (!i->k) {                                                    \
		CHECK_CONFLICT();                                           \
		i->k = (copy);                                              \
		i->v = ivm_string_list_push(&pool->lst, i->k);              \
		return i->k;                                                \
	} else if (cmp) {                                               \
		CHECK_CONFLICT();                                           \
		return i->k;                                                \
	}

#define CHECK_I(cmp, copy) \
	if (!i->k) {                                                    \
		CHECK_CONFLICT();                                           \
		i->k = (copy);                                              \
		return i->v = ivm_string_list_push(&pool->lst, i->k);       \
	} else if (cmp) {                                               \
		CHECK_CONFLICT();                                           \
		return i->v;                                                \
	}

#define SET_RAW(val, id) \
	if (!i->k) {                                                    \
		CHECK_CONFLICT();                                           \
		i->v = id;                                                  \
		return i->k = (val);                                        \
	}

#define HASH(hashee, each) \
	{                                                                           \
		ivm_hash_val_t hash;                                                    \
		register ivm_string_pos_t *i, *end, *tmp;                               \
		register ivm_uint_t conflict;                                           \
		hash = ivm_hash_fromString(hashee);                                     \
	                                                                            \
		while (1) {                                                             \
		AGAIN:                                                                  \
			conflict = 0;                                                       \
			end = pool->table + pool->size;                                     \
			tmp = pool->table + (hash & pool->mask);                            \
	                                                                            \
			for (i = tmp; i != end; i++) {                                      \
				each;                                                           \
			}                                                                   \
	                                                                            \
			for (i = pool->table;                                               \
				 i != tmp; i++) {                                               \
				each;                                                           \
			}                                                                   \
	                                                                            \
			_ivm_string_pool_expand(pool);                                      \
		}                                                                       \
	} int dummy()

IVM_PRIVATE
IVM_INLINE
ivm_string_t *
_ivm_string_copy_heap(const ivm_string_t *str,
					  ivm_heap_t *heap)
{
	ivm_size_t size;
	ivm_string_t *ret;

	size = IVM_STRING_GET_SIZE(ivm_string_length(str));
	ret = ivm_heap_alloc(heap, size);
	STD_MEMCPY(ret, str, size);
	ret->is_const = IVM_TRUE;

	return ret;
}

IVM_PRIVATE
IVM_INLINE
ivm_string_t *
_ivm_string_new_heap(ivm_bool_t is_const,
					 const ivm_char_t *str,
					 ivm_heap_t *heap)
{
	ivm_size_t len = IVM_STRLEN(str);
	ivm_string_t *ret = ivm_heap_alloc(heap, IVM_STRING_GET_SIZE(len));

	IVM_ASSERT(IVM_STRING_LEGAL_LEN(len),
			   IVM_ERROR_MSG_ILLEGAL_STRING_LEN(len, IVM_STRING_MAX_LEN));

	ivm_string_initHead(ret, is_const, len);

	STD_MEMCPY(ret->cont, str, sizeof(ivm_char_t) * (len + 1));

	return ret;
}

IVM_PRIVATE
IVM_INLINE
ivm_string_t *
_ivm_string_new_heap_n(ivm_bool_t is_const,
					   const ivm_char_t *str,
					   ivm_size_t len,
					   ivm_heap_t *heap)
{
	ivm_string_t *ret = ivm_heap_alloc(heap, IVM_STRING_GET_SIZE(len));

	IVM_ASSERT(IVM_STRING_LEGAL_LEN(len),
			   IVM_ERROR_MSG_ILLEGAL_STRING_LEN(len, IVM_STRING_MAX_LEN));

	ivm_string_initHead(ret, is_const, len);

	STD_MEMCPY(ret->cont, str, sizeof(ivm_char_t) * len);
	ret->cont[len] = '\0';

	return ret;
}

const ivm_string_t *
ivm_string_pool_register(ivm_string_pool_t *pool,
						 const ivm_string_t *str)
HASH(ivm_string_trimHead(str),
	 CHECK(ivm_string_compare(i->k, str),
		   _ivm_string_copy_heap(str, pool->heap)));

const ivm_string_t *
ivm_string_pool_registerRaw(ivm_string_pool_t *pool,
							const ivm_char_t *str)
HASH(str, CHECK(!ivm_string_compareToRaw(i->k, str),
				_ivm_string_new_heap(IVM_TRUE, str, pool->heap)));

ivm_string_id_t
ivm_string_pool_registerRaw_i(ivm_string_pool_t *pool,
							  const ivm_char_t *str)
HASH(str, CHECK_I(!ivm_string_compareToRaw(i->k, str),
				  _ivm_string_new_heap(IVM_TRUE, str, pool->heap)));

const ivm_string_t *
ivm_string_pool_registerRaw_n(ivm_string_pool_t *pool,
							  const ivm_char_t *str,
							  ivm_size_t len)
HASH(str, CHECK(!ivm_string_compareToRaw_n(i->k, str, len),
				_ivm_string_new_heap_n(IVM_TRUE, str, len, pool->heap)));

IVM_PRIVATE
const ivm_string_t *
_ivm_string_pool_rehash(ivm_string_pool_t *pool,
						const ivm_string_t *str,
						ivm_string_id_t id)
HASH(ivm_string_trimHead(str), SET_RAW(str, id));

#undef HASH
