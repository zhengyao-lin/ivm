#ifndef _IVM_STD_THREAD_H_
#define _IVM_STD_THREAD_H_

#include "pub/const.h"
#include "pub/com.h"

#if IVM_USE_MULTITHREAD

#include <pthread.h>

IVM_COM_HEADER

typedef void *(*ivm_thread_start_func_t)(void *);

typedef pthread_t ivm_thread_t;

#define ivm_thread_equal(a, b) pthread_equal((a), (b))
#define ivm_thread_self() pthread_self()

ivm_thread_t *
ivm_thread_new(ivm_thread_start_func_t func,
			   void *arg);

#define ivm_thread_init(thread, func, arg) pthread_create((thread), IVM_NULL, (func), (arg))

void
ivm_thread_free(ivm_thread_t *thread);

#define ivm_thread_dump(thread)

IVM_INLINE
void *
ivm_thread_wait(ivm_thread_t *thread)
{
	void *ret = IVM_NULL;
	
	if (pthread_join(*thread, &ret))
		return IVM_NULL;
	
	return ret;
}

#define ivm_thread_cancelPoint() pthread_testcancel()
#define ivm_thread_cancel(thread) (pthread_cancel(*(thread)) == 0)

typedef pthread_mutex_t ivm_thread_mutex_t;

#define IVM_THREAD_MUTEXT_INITVAL PTHREAD_MUTEX_INITIALIZER

#define ivm_thread_mutex_init(mut) pthread_mutex_init((mut), IVM_NULL)
#define ivm_thread_mutex_dump(mut) pthread_mutex_destroy(mut)

#define ivm_thread_mutex_lock(mut) pthread_mutex_lock(mut)
#define ivm_thread_mutex_unlock(mut) pthread_mutex_unlock(mut)

IVM_COM_END

#endif

#endif
