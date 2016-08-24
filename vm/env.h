#ifndef _IVM_VM_ENV_H_
#define _IVM_VM_ENV_H_

#include "pub/com.h"
#include "pub/type.h"
#include "pub/vm.h"

#include "std/list.h"

IVM_COM_HEADER

ivm_int_t
ivm_env_init();

void
ivm_env_clean();

IVM_COM_END

#endif
