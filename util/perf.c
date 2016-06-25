#include <time.h>

#include "pub/const.h"
#include "pub/type.h"
#include "pub/err.h"

#include "perf.h"

#if IVM_USE_PERF_PROFILE

extern clock_t ivm_perf_gc_time;
extern ivm_size_t ivm_perf_gc_count;

clock_t ivm_perf_program_start;
clock_t ivm_perf_program_duration = 0;

void
ivm_perf_reset()
{
	ivm_perf_gc_time = 0;
	ivm_perf_gc_count = 0;
	ivm_perf_program_duration = 0;
	return;
}

void
ivm_perf_startProfile()
{
	ivm_perf_program_start = clock();
	return;
}

void
ivm_perf_stopProfile()
{
	ivm_perf_program_duration += clock() - ivm_perf_program_start;
	return;
}

clock_t
ivm_perf_printElapsed()
{
	clock_t prog = ivm_perf_program_duration;

	IVM_TRACE("\n***performance profile***\n\n");

	IVM_TRACE("program: %ld ticks(%fs)\n",
			  prog, (double)prog / CLOCKS_PER_SEC);

	IVM_TRACE("gc times: %zd\n", ivm_perf_gc_count);

	IVM_TRACE("gc: %ld ticks(%fs)\n",
			  ivm_perf_gc_time,
			  (double)ivm_perf_gc_time / CLOCKS_PER_SEC);

	IVM_TRACE("gc per program: %.4f%%\n",
			  (double)ivm_perf_gc_time / prog * 100);

	IVM_TRACE("gc per time: %.3f ticks\n",
			  ivm_perf_gc_count ? (double)ivm_perf_gc_time / ivm_perf_gc_count : 0.0);

	IVM_TRACE("\n***performance profile***\n\n");

	return prog;
}

#endif
