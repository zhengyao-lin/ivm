#ifndef _IVM_STD_ENV_H_
#define _IVM_STD_ENV_H_

#include <stdlib.h>

#include "pub/com.h"
#include "pub/type.h"

IVM_COM_HEADER

#define ivm_env_getVar(name) getenv(name)

IVM_COM_END

#endif
