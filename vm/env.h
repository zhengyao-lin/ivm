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

void
ivm_env_setArg(const ivm_char_t **argv,
			   ivm_argc_t argc);

const ivm_char_t **
ivm_env_getArgv();

ivm_int_t
ivm_env_getArgc();

IVM_COM_END

#endif
