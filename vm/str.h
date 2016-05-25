#ifndef _IVM_VM_STR_H_
#define _IVM_VM_STR_H_

#include <string.h>
#include "pub/com.h"
#include "pub/const.h"
#include "type.h"
#include "std/list.h"

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

typedef ivm_ptlist_t ivm_string_list_t;

#define ivm_string_list_new ivm_ptlist_new
#define ivm_string_list_free ivm_ptlist_free

#define ivm_string_list_register ivm_ptlist_push

#define ivm_string_list_at(list, i) ((ivm_char_t *)ivm_ptlist_at((list), (i)))
#define ivm_string_list_indexOf(list, str) (ivm_ptlist_indexOf((list), (void *)(str), IVM_STRCMP))

typedef struct {
	struct ivm_heap_t_tag *heap;
	ivm_string_list_t *set;
} ivm_string_pool_t;

ivm_string_pool_t *
ivm_string_pool_new();

void
ivm_string_pool_free(ivm_string_pool_t *pool);

ivm_size_t
ivm_string_pool_register(ivm_string_pool_t *list,
						 const ivm_char_t *str);

#define ivm_string_pool_get(pool, i) (ivm_string_list_at((pool)->set, (i)))

IVM_COM_END

#endif
