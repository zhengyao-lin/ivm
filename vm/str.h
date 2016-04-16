#ifndef _IVM_VM_STR_H_
#define _IVM_VM_STR_H_

#include <string.h>
#include "type.h"

#define IVM_STRDUP(str) (ivm_strdup(str))
#define IVM_STRCMP(s1, s2) (strcmp(s1, s2))
#define IVM_STRLEN(str) (strlen(str))

ivm_char_t *ivm_strdup(const ivm_char_t *src);

#endif
