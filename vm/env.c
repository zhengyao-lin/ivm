#include "pub/const.h"
#include "std/mem.h"
#include "pub/vm.h"

#include "std/sys.h"

// #include "thread/sched.h"

#include "mod/mod.h"

#include "env.h"

ivm_int_t
ivm_env_init()
{
	ivm_sys_setDefaultLocal();
	
#if IVM_DISPATCH_METHOD_DIRECT_THREAD
	ivm_opcode_table_initOpEntry();
#endif

	ivm_mod_init();

	return 0;
}

void
ivm_env_clean()
{
	ivm_mod_clean();
	// ivm_thread_clean();

	return;
}
