#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "pub/type.h"
#include "pub/vm.h"
#include "pub/inlines.h"

#include "std/time.h"

#include "vm/native/native.h"
#include "vm/native/priv.h"

#define TIME_ERROR_MSG_FAILED_SLEEP(ms)					"failed to sleep %ld ms", (ms)

IVM_NATIVE_FUNC(_time_sleep)
{
	ivm_ulong_t sec;
	CHECK_ARG_1(IVM_NUMERIC_T);
	sec = ivm_numeric_getValue(NAT_ARG_AT(1));
	RTM_ASSERT(ivm_time_sleep(sec), TIME_ERROR_MSG_FAILED_SLEEP(sec * 1000));
	return IVM_NONE(NAT_STATE());
}

IVM_NATIVE_FUNC(_time_msleep)
{
	ivm_ulong_t ms;
	CHECK_ARG_1(IVM_NUMERIC_T);
	ms = ivm_numeric_getValue(NAT_ARG_AT(1));
	RTM_ASSERT(ivm_time_msleep(ms), TIME_ERROR_MSG_FAILED_SLEEP(ms));
	return IVM_NONE(NAT_STATE());
}

ivm_object_t *
ivm_mod_main(ivm_vmstate_t *state,
			 ivm_coro_t *coro,
			 ivm_context_t *context)
{
	ivm_object_t *mod = ivm_object_new_c(state, 2);

	ivm_object_setSlot_r(mod, state, "sleep", IVM_NATIVE_WRAP(state, _time_sleep));
	ivm_object_setSlot_r(mod, state, "msleep", IVM_NATIVE_WRAP(state, _time_msleep));

	return mod;
}
