#include "pub/com.h"
#include "pub/type.h"
#include "pub/vm.h"

#include "std/time.h"
#include "std/thread.h"

#include "vm/coro.h"

#if IVM_USE_MULTITHREAD

IVM_PRIVATE
void *
_ivm_thread_inst_clock(void *arg)
{
	while (1) {
		ivm_coro_unsetCSL();
		ivm_time_msleep(1);
		// IVM_TRACE("########signal!\n");
		ivm_coro_setInt(IVM_CORO_INT_THREAD_YIELD);
		while (!ivm_coro_getCSL());
	}

	return IVM_NULL;
}

IVM_PRIVATE
ivm_bool_t _thread_enabled = IVM_FALSE;

IVM_PRIVATE
ivm_thread_t _t_clock;

void
ivm_thread_enableThread()
{
	// ivm_coro_lockCSL();
	_thread_enabled = IVM_TRUE;
	ivm_thread_init(&_t_clock, _ivm_thread_inst_clock, IVM_NULL);
	
	return;
}

void
ivm_thread_clean()
{
	if (_thread_enabled) {
		ivm_thread_cancel(&_t_clock);
		ivm_thread_wait(&_t_clock);
		ivm_thread_dump(&_t_clock);
	}

	return;
}

void
ivm_thread_mainThreadStart()
{
	if (_thread_enabled) {
		ivm_coro_lockGIL();
	}
	
	return;
}

void
ivm_thread_mainThreadEnd()
{
	if (_thread_enabled) {
		ivm_coro_setCSL();
		ivm_coro_unlockGIL();
	}
	
	return;
}

#else

void
ivm_thread_mainThreadStart()
{
	return;
}

void
ivm_thread_mainThreadEnd()
{
	return;
}

#endif
