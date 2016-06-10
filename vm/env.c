#include "pub/const.h"

#include "env.h"
#include "opcode.h"

int
ivm_env_init()
{
#if IVM_DISPATCH_METHOD_DIRECT_THREAD
	ivm_opcode_table_initOpEntry();
#endif

	return 0;
}
