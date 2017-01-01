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

IVM_PRIVATE
const ivm_char_t **_ivm_global_argv = IVM_NULL;

IVM_PRIVATE
ivm_int_t _ivm_global_argc = 0;

void
ivm_env_setArg(const ivm_char_t **argv,
			   ivm_argc_t argc)
{
	_ivm_global_argv = argv;
	_ivm_global_argc = argc;
	return;
}

const ivm_char_t **
ivm_env_getArgv()
{
	return _ivm_global_argv;
}

ivm_int_t
ivm_env_getArgc()
{
	return _ivm_global_argc;
}
