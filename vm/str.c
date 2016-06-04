#include "pub/mem.h"
#include "inline/obj.h"
#include "str.h"
#include "type.h"
#include "vm.h"
#include "bit.h"
#include "gc/heap.h"
#include "gc/gc.h"
#include "err.h"

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
				 struct ivm_vmstate_t_tag *state)
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
				struct ivm_heap_t_tag *heap)
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
									   IVM_STRDUP_HEAP(str, pool->heap));
	}

	return ret;
}

ivm_object_t *ivm_string_object_new(ivm_vmstate_t *state,
									ivm_bool_t is_const,
									const ivm_char_t *val)
{
	ivm_string_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), state, IVM_STRING_OBJECT_T);

	if (is_const) {
		IVM_BIT_SET_TRUE(ret->is_const);
		ret->val = (ivm_char_t *)val;
	} else {
		IVM_BIT_SET_FALSE(ret->is_const);
		ret->val = IVM_STRDUP_STATE(val, state);
	}

	return IVM_AS_OBJ(ret);
}

void
ivm_string_object_traverser(ivm_object_t *obj,
							ivm_traverser_arg_t *arg)
{
	ivm_string_object_t *str = IVM_AS(obj, ivm_string_object_t);

	if (!str->is_const) {
		str->val = IVM_STRDUP_HEAP(str->val, arg->heap);
	}

	return;
}
