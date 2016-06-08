#include "pub/const.h"

#include "env.h"
#include "op.h"

int
ivm_env_init()
{
#if IVM_DISPATCH_METHOD_DIRECT_THREAD
	ivm_op_table_initOpEntry();
#endif

	return 0;
}
