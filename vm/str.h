#ifndef _IVM_VM_STR_H_
#define _IVM_VM_STR_H_

#include <string.h>
#include "type.h"

struct ivm_vmstate_t_tag;
struct ivm_heap_t_tag;

#define IVM_STRDUP(str) (ivm_strdup(str))
#define IVM_STRCMP(s1, s2) (strcmp(s1, s2))
#define IVM_STRLEN(str) (strlen(str))
#define IVM_STRLEN_STATE(str, state) (ivm_strdup_state((str), (state)))
#define IVM_STRLEN_HEAP(str, heap) (ivm_strdup_heap((str), (heap)))

ivm_char_t *
ivm_strdup(const ivm_char_t *src);
ivm_char_t *
ivm_strdup_state(const ivm_char_t *src,
				 struct ivm_vmstate_t_tag *state);
ivm_char_t *
ivm_strdup_heap(const ivm_char_t *src,
				struct ivm_heap_t_tag *heap);

#endif
