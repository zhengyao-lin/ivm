#ifndef _IVM_VM_STD_STRING_H_
#define _IVM_VM_STD_STRING_H_

#include <string.h>
#include "pub/com.h"
#include "pub/const.h"
#include "pub/type.h"
#include "list.h"

IVM_COM_HEADER

struct ivm_vmstate_t_tag;
struct ivm_heap_t_tag;

#define IVM_STRDUP ivm_strdup
#define IVM_STRCMP strcmp
#define IVM_STRLEN strlen
#define IVM_STRDUP_STATE(str, state) (ivm_strdup_state((str), (state)))
#define IVM_STRDUP_HEAP(str, heap) (ivm_strdup_heap((str), (heap)))

ivm_char_t *
ivm_strdup(const ivm_char_t *src);
ivm_char_t *
ivm_strdup_state(const ivm_char_t *src,
				 struct ivm_vmstate_t_tag *state);
ivm_char_t *
ivm_strdup_heap(const ivm_char_t *src,
				struct ivm_heap_t_tag *heap);

typedef struct {
	ivm_bool_t is_const: 1;
	ivm_uint16_t len;
} ivm_string_t;

#define IVM_STRING_GET_SIZE(len) \
	(sizeof(ivm_string_t) + sizeof(ivm_char_t) * ((len) + 1))

ivm_string_t *
ivm_string_new_state(ivm_bool_t is_const,
					 const ivm_char_t *str,
					 struct ivm_vmstate_t_tag *state);

ivm_string_t *
ivm_string_new_heap(ivm_bool_t is_const,
					const ivm_char_t *str,
					struct ivm_heap_t_tag *heap);

const ivm_string_t *
ivm_string_copyIfNotConst_state(const ivm_string_t *str,
								struct ivm_vmstate_t_tag *state);

const ivm_string_t *
ivm_string_copyIfNotConst_heap(const ivm_string_t *str,
							   struct ivm_heap_t_tag *heap);

#define ivm_string_length(str) \
	((str)->len)

#define ivm_string_trimHead(str) \
	((ivm_char_t *)(((ivm_string_t *)str) + 1))

IVM_INLINE
ivm_bool_t
ivm_string_compare(const ivm_string_t *a,
				   const ivm_string_t *b)
{
	return (ivm_string_length(a) == ivm_string_length(b))
		   && (!MEM_COMPARE(ivm_string_trimHead(a),
							ivm_string_trimHead(b),
							ivm_string_length(a)));
}

ivm_int_t /* same as strcmp */
ivm_string_compareToRaw(const ivm_string_t *a,
						const ivm_char_t *b);

typedef ivm_ptlist_t ivm_string_list_t;

#define ivm_string_list_new ivm_ptlist_new
#define ivm_string_list_free ivm_ptlist_free

#define ivm_string_list_register ivm_ptlist_push

#define ivm_string_list_at(list, i) ((ivm_string_t *)ivm_ptlist_at((list), (i)))
#define ivm_string_list_indexOf(list, str) (ivm_ptlist_indexOf((list), (void *)(str), ivm_string_compareToRaw))

typedef struct {
	struct ivm_heap_t_tag *heap;
	ivm_string_list_t *set;
} ivm_string_pool_t;

ivm_string_pool_t *
ivm_string_pool_new();

void
ivm_string_pool_free(ivm_string_pool_t *pool);

ivm_size_t
ivm_string_pool_register(ivm_string_pool_t *pool,
						 const ivm_char_t *str);

const ivm_string_t *
ivm_string_pool_store(ivm_string_pool_t *pool,
					  const ivm_char_t *str);

#define ivm_string_pool_get(pool, i) (ivm_string_list_at((pool)->set, (i)))

IVM_COM_END

#endif
