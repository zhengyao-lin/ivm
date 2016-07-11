#include "pub/mem.h"
#include "pub/err.h"
#include "pub/vm.h"
#include "pub/type.h"

#include "string.h"
#include "heap.h"
#include "hash.h"
#include "bit.h"

ivm_char_t *
ivm_strdup(const ivm_char_t *src)
{
	ivm_size_t size = sizeof(ivm_char_t) * (IVM_STRLEN(src) + 1);
	ivm_char_t *ret = MEM_ALLOC(size, ivm_char_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("new string"));

	MEM_COPY(ret, src, size);

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
		MEM_COPY(ret, src, size);
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
		MEM_COPY(ret, src, size);
	}

	return ret;
}

ivm_string_t *
ivm_string_new_state(ivm_bool_t is_const,
					 const ivm_char_t *str,
					 ivm_vmstate_t *state)
{
	ivm_size_t len = IVM_STRLEN(str);
	ivm_string_t *ret = ivm_vmstate_alloc(state, IVM_STRING_GET_SIZE(len));
	
	IVM_ASSERT(IVM_STRING_LEGAL_LEN(len),
			   IVM_ERROR_MSG_ILLEGAL_STRING_LEN(len, IVM_STRING_MAX_LEN));
	
	ret->is_const = is_const;
	ret->len = len;
	MEM_COPY(ivm_string_trimHead(ret), str,
			 sizeof(ivm_char_t) * len);

	return ret;
}

ivm_string_t *
ivm_string_new_heap(ivm_bool_t is_const,
					const ivm_char_t *str,
					ivm_heap_t *heap)
{
	ivm_size_t len = IVM_STRLEN(str);
	ivm_string_t *ret = ivm_heap_alloc(heap, IVM_STRING_GET_SIZE(len));

	IVM_ASSERT(IVM_STRING_LEGAL_LEN(len),
			   IVM_ERROR_MSG_ILLEGAL_STRING_LEN(len, IVM_STRING_MAX_LEN));

	ret->is_const = is_const;
	ret->len = len;
	MEM_COPY(ivm_string_trimHead(ret), str,
			 sizeof(ivm_char_t) * (len + 1));

	return ret;
}

ivm_string_t *
_ivm_string_new_n_heap(ivm_bool_t is_const,
					   const ivm_char_t *str,
					   ivm_size_t len,
					   ivm_heap_t *heap)
{
	ivm_string_t *ret = ivm_heap_alloc(heap, IVM_STRING_GET_SIZE(len));

	IVM_ASSERT(IVM_STRING_LEGAL_LEN(len),
			   IVM_ERROR_MSG_ILLEGAL_STRING_LEN(len, IVM_STRING_MAX_LEN));

	ret->is_const = is_const;
	ret->len = len;
	MEM_COPY(ivm_string_trimHead(ret), str,
			 sizeof(ivm_char_t) * (len + 1));

	return ret;
}

void
ivm_string_initHead(ivm_string_t *str,
					ivm_bool_t is_const,
					ivm_size_t len)
{
	str->is_const = is_const;
	str->len = len;
	
	return;
}

IVM_INLINE
ivm_string_t *
_ivm_string_copy_heap(ivm_bool_t is_const,
					  const ivm_string_t *str,
					  ivm_heap_t *heap)
{
	ivm_size_t size;
	ivm_string_t *ret;

	size = IVM_STRING_GET_SIZE(ivm_string_length(str));
	ret = ivm_heap_alloc(heap, size);
	MEM_COPY(ret, str, size);
	ret->is_const = is_const;

	return ret;
}

const ivm_string_t *
ivm_string_copyIfNotConst_state(const ivm_string_t *str,
								ivm_vmstate_t *state)
{
	ivm_size_t size;
	ivm_string_t *ret;

	if (str && !str->is_const) {
		size = IVM_STRING_GET_SIZE(ivm_string_length(str));
		ret = ivm_vmstate_alloc(state, size);
		MEM_COPY(ret, str, size);

		return ret;
	}

	return str;
}

const ivm_string_t *
ivm_string_copyIfNotConst_heap(const ivm_string_t *str,
							   ivm_heap_t *heap)
{
	ivm_size_t size;
	ivm_string_t *ret;

	if (str && !str->is_const) {
		size = IVM_STRING_GET_SIZE(ivm_string_length(str));
		ret = ivm_heap_alloc(heap, size);
		MEM_COPY(ret, str, size);

		return ret;
	}

	return str;
}

const ivm_string_t *
ivm_string_copyIfNotConst_pool(const ivm_string_t *str,
							   ivm_vmstate_t *state)
{
	ivm_size_t size;
	ivm_string_t *ret;

	if (str && !str->is_const) {
		if (ivm_string_length(str) <= IVM_DEFAULT_CONST_THRESHOLD) {
			return
			(const ivm_string_t *)
			ivm_string_pool_register(IVM_VMSTATE_GET(state, CONST_POOL), str);
		}

		size = IVM_STRING_GET_SIZE(ivm_string_length(str));
		ret = ivm_vmstate_alloc(state, size);
		MEM_COPY(ret, str, size);

		return ret;
	}

	return str;
}

ivm_int_t
ivm_string_compareToRaw(const ivm_string_t *a,
						const ivm_char_t *b)
{
	return IVM_STRCMP(ivm_string_trimHead(a), b);
}

ivm_int_t
ivm_string_compareToRaw_n(const ivm_string_t *a,
						  const ivm_char_t *b,
						  ivm_size_t len)
{
	return IVM_STRNCMP(ivm_string_trimHead(a),
					   ivm_string_length(a),
					   b, len);
}

ivm_string_pool_t *
ivm_string_pool_new(ivm_bool_t is_fixed)
{
	ivm_string_pool_t *ret = MEM_ALLOC(sizeof(*ret),
									   ivm_string_pool_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("string pool"));

	ivm_ref_init(ret);
	ret->heap = ivm_heap_new(IVM_DEFAULT_STRING_POOL_BLOCK_SIZE);
	
	ret->is_fixed = is_fixed;
	ret->size = IVM_DEFAULT_STRING_POOL_BUFFER_SIZE;
	ret->table = MEM_ALLOC_INIT(sizeof(*ret->table)
								* IVM_DEFAULT_STRING_POOL_BUFFER_SIZE,
								const ivm_string_t **);

	IVM_ASSERT(ret->table, IVM_ERROR_MSG_FAILED_ALLOC_NEW("string pool data"));

	return ret;
}

void
ivm_string_pool_free(ivm_string_pool_t *pool)
{
	if (pool && !ivm_ref_dec(pool)) {
		ivm_heap_free(pool->heap);
		MEM_FREE(pool->table);
		MEM_FREE(pool);
	}

	return;
}

IVM_PRIVATE
ivm_size_t /* next empty index */
_ivm_string_pool_expand(ivm_string_pool_t *pool)
{
	ivm_size_t osize = pool->size;
	ivm_size_t ret = 0;
	const ivm_string_t **otable;
	const ivm_string_t **i, **end;

	if (pool->is_fixed) {
		pool->size <<= 1;
		pool->table = MEM_REALLOC(pool->table,
								  sizeof(*pool->table) * pool->size,
								  const ivm_string_t **);

		IVM_ASSERT(pool->table, IVM_ERROR_MSG_FAILED_ALLOC_NEW("string pool data"));

		MEM_INIT(pool->table + (ret = osize),
				 sizeof(ivm_string_t *) * (pool->size - osize));
	} else {
		otable = pool->table;

		pool->size <<= 1;
		pool->table = MEM_ALLOC_INIT(sizeof(*pool->table) * pool->size,
									 const ivm_string_t **);

		for (i = otable, end = otable + osize;
			 i != end; i++) {
			ivm_string_pool_register(pool, *i);
		}

		MEM_FREE(otable);
	}

	return ret;
}

#define HASH(hashee, cmp, copy) \
	{                                                                               \
		ivm_hash_val_t hash;                                                        \
		const ivm_string_t **i, **end, **tmp;                                       \
		ivm_ptr_t ret;                                                              \
	                                                                                \
		if (pool->is_fixed) {                                                       \
			end = pool->table + pool->size;                                         \
			for (ret = 0, i = pool->table; i != end;                                \
				 i++, ret++) {                                                      \
				if (!*i) {                                                          \
					*i = copy;                                                      \
					return ret;                                                     \
				} else if (cmp) {                                                   \
					return ret;                                                     \
				}                                                                   \
			}                                                                       \
	                                                                                \
			ret = _ivm_string_pool_expand(pool);                                    \
			pool->table[ret]                                                        \
			= copy;                                                                 \
		} else {                                                                    \
			hash = ivm_hash_fromString(hashee);                                     \
	                                                                                \
			while (1) {                                                             \
				end = pool->table + pool->size;                                     \
				tmp = pool->table + hash % pool->size;                              \
                                                                                    \
				for (i = tmp; i != end; i++) {                                      \
					if (!*i) {                                                      \
						return (ivm_ptr_t)(*i = copy);                              \
					} else if (cmp) {                                               \
						return (ivm_ptr_t)*i;                                       \
					}                                                               \
				}                                                                   \
	                                                                                \
				for (i = pool->table;                                               \
					 i != tmp; i++) {                                               \
					if (!*i) {                                                      \
						return (ivm_ptr_t)(*i = copy);                              \
					} else if (cmp) {                                               \
						return (ivm_ptr_t)*i;                                       \
					}                                                               \
				}                                                                   \
	                                                                                \
				_ivm_string_pool_expand(pool);                                      \
			}                                                                       \
		}                                                                           \
	                                                                                \
		return ret;                                                                 \
	} int dummy()

ivm_ptr_t
ivm_string_pool_register(ivm_string_pool_t *pool,
						 const ivm_string_t *str)
HASH(ivm_string_trimHead(str),
	 ivm_string_compare(*i, str),
	 _ivm_string_copy_heap(IVM_TRUE, str, pool->heap));

ivm_ptr_t
ivm_string_pool_register_nc(ivm_string_pool_t *pool,
							const ivm_string_t *str)
HASH(ivm_string_trimHead(str),
	 ivm_string_compare(*i, str), str);

ivm_ptr_t
ivm_string_pool_registerRaw(ivm_string_pool_t *pool,
							const ivm_char_t *str)
HASH(str, !ivm_string_compareToRaw(*i, str),
	 ivm_string_new_heap(IVM_TRUE, str, pool->heap));

ivm_ptr_t
ivm_string_pool_registerRaw_n(ivm_string_pool_t *pool,
							  const ivm_char_t *str,
							  ivm_size_t len)
HASH(str, !ivm_string_compareToRaw_n(*i, str, len),
	 _ivm_string_new_n_heap(IVM_TRUE, str, len, pool->heap));


#undef HASH
