#include "pub/const.h"
#include "pub/err.h"

#include "mem.h"
#include "thread.h"

#if IVM_USE_MULTITHREAD

ivm_thread_t *
ivm_thread_new(ivm_thread_start_func_t func,
			   void *arg)
{
	ivm_thread_t *ret = STD_ALLOC(sizeof(*ret));

	IVM_MEMCHECK(ret);

	if (pthread_create(ret, IVM_NULL, func, arg)) {
		STD_FREE(ret);
		return IVM_NULL;
	}

	return ret;
}

void
ivm_thread_free(ivm_thread_t *thread)
{
	if (thread) {
		STD_FREE(thread);
	}

	return;
}

#endif
