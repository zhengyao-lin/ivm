#ifndef _IVM_STD_UID_H_
#define _IVM_STD_UID_H_

#include "pub/type.h"
#include "pub/com.h"

IVM_COM_HEADER

typedef ivm_ptr_t ivm_uid_t;

typedef struct {
	ivm_uid_t cur;
} ivm_uid_gen_t;

#define ivm_uid_gen_init(gen) ((gen)->cur = 0)
#define ivm_uid_gen_nextPtr(gen) (++(gen)->cur)
#define ivm_uid_gen_curPtr(gen) ((gen)->cur)

IVM_COM_END

#endif
