#ifndef _IVM_STD_UID_H_
#define _IVM_STD_UID_H_

#include "pub/type.h"
#include "pub/com.h"

IVM_COM_HEADER

typedef ivm_ptr_t ivm_uid_t;

typedef struct {
	ivm_uid_t cur;
} ivm_uid_gen_t;

#define ivm_uid_gen_init(state) ((state)->cur = 0)
#define ivm_uid_gen_nextPtr(state) (++(state).cur)

IVM_COM_END

#endif
