#include "pub/const.h"
#include "pub/mem.h"
#include "pub/vm.h"

#include "std/string.h"

#include "env.h"

ivm_gen_env_t *
ivm_gen_env_new()
{
	ivm_gen_env_t *ret = MEM_ALLOC(sizeof(*ret),
								   ivm_gen_env_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("generator environment"));

	ret->str_pool = ivm_string_pool_new(IVM_TRUE);
	ret->block_list = ivm_gen_block_list_new();

	return ret;
}

void
ivm_gen_env_free(ivm_gen_env_t *env)
{
	if (env) {
		ivm_string_pool_free(env->str_pool);
		ivm_gen_block_list_free(env->block_list);
		MEM_FREE(env);
	}

	return;
}

ivm_exec_t *
ivm_gen_env_newExec(ivm_gen_env_t *env,
					const ivm_char_t *label)
{
	ivm_gen_block_t block = {
		.label = label,
		.exec = ivm_exec_new(env->str_pool)
	};

	ivm_gen_block_list_push(env->block_list, &block);

	return block.exec;
}

int
ivm_env_init()
{
#if IVM_DISPATCH_METHOD_DIRECT_THREAD
	ivm_opcode_table_initOpEntry();
#endif

	return 0;
}
