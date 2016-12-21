#ifndef _IVM_STD_TIME_H_
#define _IVM_STD_TIME_H_

#include <time.h>

#include "pub/type.h"
#include "pub/com.h"

IVM_COM_HEADER

#if defined(IVM_OS_WIN32)

	#include <windows.h>

	IVM_INLINE
	ivm_bool_t
	ivm_time_msleep(ivm_ulong_t ms)
	{
		Sleep(ms);
		return IVM_TRUE;
	}

#else

	struct timespec {
		time_t tv_sec;
		long tv_nsec;
	};

	int nanosleep(const struct timespec *req, struct timespec *rem);

	// millisecond
	IVM_INLINE
	ivm_bool_t
	ivm_time_msleep(ivm_ulong_t ms)
	{
		ivm_ulong_t sec;
		struct timespec ts;

		if (ms >= 1000) {
			sec = ms / 1000;
			ms -= sec * 1000;
		} else sec = 0;

		ts.tv_sec = sec;
		ts.tv_nsec = ms * 1000000; // 1e6

		return nanosleep(&ts, IVM_NULL) == 0;
	}

#endif

#define ivm_time_getCur() time(IVM_NULL)

#define ivm_time_sleep(sec) ivm_time_msleep((sec) * 1000)
#define ivm_time_hwclock() ((ivm_long_t)clock())

IVM_COM_END

#endif
