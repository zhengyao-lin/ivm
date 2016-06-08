#include "pub/mem.h"
#include "pub/err.h"
#include "pub/vm.h"
#include "pub/type.h"

#include "string.h"
#include "heap.h"

#include "../bit.h"

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

	if (is_const) IVM_BIT_SET_TRUE(ret->is_const);
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

	if (is_const) IVM_BIT_SET_TRUE(ret->is_const);
	ret->len = len;
	MEM_COPY(ivm_string_trimHead(ret), str,
			 sizeof(ivm_char_t) * (len + 1));

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
	} else {
		return str;
	}

	return ret;
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
	} else {
		return str;
	}

	return ret;
}

ivm_int_t
ivm_string_compareToRaw(const ivm_string_t *a,
						const ivm_char_t *b)
{
	return IVM_STRCMP(ivm_string_trimHead(a), b);
}

ivm_string_pool_t *
ivm_string_pool_new()
{
	ivm_string_pool_t *ret = MEM_ALLOC(sizeof(*ret),
									   ivm_string_pool_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("string pool"));

	ret->heap = ivm_heap_new(IVM_DEFAULT_STRING_POOL_BLOCK_SIZE);
	ret->set = ivm_string_list_new();

	return ret;
}

void
ivm_string_pool_free(ivm_string_pool_t *pool)
{
	if (pool) {
		ivm_heap_free(pool->heap);
		ivm_string_list_free(pool->set);
		MEM_FREE(pool);
	}

	return;
}

ivm_size_t
ivm_string_pool_register(ivm_string_pool_t *pool,
						 const ivm_char_t *str)
{
	ivm_size_t ret = ivm_string_list_indexOf(pool->set, str);

	if (ret == (ivm_size_t)-1) {
		ret = ivm_string_list_register(pool->set,
									   ivm_string_new_heap(IVM_TRUE, str, pool->heap));
	}

	return ret;
}

const ivm_string_t *
ivm_string_pool_store(ivm_string_pool_t *pool,
					  const ivm_char_t *str)
{
	ivm_size_t i = ivm_string_list_indexOf(pool->set, str);
	ivm_string_t *ret;

	if (i == (ivm_size_t)-1) {
		ivm_string_list_register(pool->set,
								 ret = ivm_string_new_heap(IVM_TRUE, str, pool->heap));
	} else {
		ret = ivm_string_list_at(pool->set, i);
	}

	return ret;
}
