#ifndef _IVM_UTIL_PERF_H_
#define _IVM_UTIL_PERF_H_

#include <time.h>

#include "pub/com.h"
#include "pub/const.h"
#include "pub/type.h"

IVM_COM_HEADER

#if IVM_USE_PERF_PROFILE

extern clock_t ivm_perf_gc_time;
extern ivm_size_t ivm_perf_gc_count;

void
ivm_perf_reset();

void
ivm_perf_startProfile();

void
ivm_perf_stopProfile();

clock_t
ivm_perf_printElapsed();

#else

#define ivm_perf_startProfile()
#define ivm_perf_stopProfile()
#define ivm_perf_printElapsed()

#endif

IVM_COM_END

#endif
