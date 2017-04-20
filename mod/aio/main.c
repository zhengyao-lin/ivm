#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "pub/type.h"
#include "pub/vm.h"
#include "pub/inlines.h"

#include "std/io.h"
#include "std/thread.h"
#include "std/time.h"
#include "std/mem.h"

#include "vm/native/native.h"
#include "vm/native/priv.h"

IVM_PRIVATE
void *_aio_read_th(void *rarg)
{
	const ivm_char_t **arg = (const ivm_char_t **)rarg;
	ivm_file_t *fp = ivm_file_new(arg[0], arg[1]);

	if (!fp) {
		arg[2] = (void *)(ivm_ptr_t)1;
		return "failed to open file";
	}

	arg[2] = ivm_file_readAll(fp);

	ivm_file_free(fp);

	if (!arg[2]) {
		arg[2] = (void *)(ivm_ptr_t)1;
		return "failed to read";
	}

	return IVM_NULL; 
}

IVM_NATIVE_FUNC(_aio_read)
{
	ivm_thread_t th;
	const ivm_char_t *arg[] = { IVM_NULL, IVM_FMODE_READ_BINARY, IVM_NULL /* return */ };
	void *thret;
	ivm_object_t *ret;

	RTM_ASSERT(ivm_vmstate_hasThread(NAT_STATE()), "thread not enabled");

	MATCH_ARG("r*r", &arg[0], &arg[1]);

	ivm_thread_init(&th, _aio_read_th, arg);

	while (!arg[2]) {
		// IVM_TRACE("here you go!");
		ivm_vmstate_threadYield(NAT_STATE());
	}

	thret = ivm_thread_wait(&th);
	if (thret) {
		RTM_FATAL("%s\n", (const ivm_char_t *)thret);
	}

	ret = ivm_string_object_new_r(NAT_STATE(), arg[2]);
	STD_FREE((ivm_char_t *)arg[2]);

	return ret;
}

ivm_object_t *
ivm_mod_main(ivm_vmstate_t *state,
			 ivm_coro_t *coro,
			 ivm_context_t *context)
{
	ivm_object_t *mod = ivm_object_new_c(state, 2);

	ivm_object_setSlot_r(mod, state, "read", IVM_NATIVE_WRAP(state, _aio_read));

	return mod;
}
