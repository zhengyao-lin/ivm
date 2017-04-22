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

struct _aio_read_arg {
	const ivm_char_t *path;
	const ivm_char_t *mode;

	ivm_char_t *ret;
	ivm_size_t len;
};

IVM_PRIVATE
void *_aio_read_th(void *rarg)
{
	struct _aio_read_arg *arg = (struct _aio_read_arg *)rarg;
	ivm_file_t *fp = ivm_file_new(arg->path, arg->mode);

	if (!fp) {
		arg->ret = "failed to open file";
		return IVM_NULL;
	}

	arg->ret = ivm_file_readAll(fp, &arg->len);

	ivm_file_free(fp);

	if (!arg->ret) {
		arg->ret = "failed to read";
		return IVM_NULL;
	}

	return (void *)arg->ret; 
}

IVM_NATIVE_FUNC(_aio_read)
{
	ivm_thread_t th;
	struct _aio_read_arg arg = { IVM_NULL, IVM_FMODE_READ_BINARY, IVM_NULL /* dat */, 0 /* size */ };
	void *thret;
	ivm_object_t *ret;

	RTM_ASSERT(ivm_vmstate_hasThread(NAT_STATE()), "thread not enabled");

	MATCH_ARG("r*r", &arg.path, &arg.mode);

	ivm_thread_init(&th, _aio_read_th, &arg);

	while (!arg.ret) {
		// IVM_TRACE("here you go!");
		ivm_vmstate_threadYield(NAT_STATE());
	}

	thret = ivm_thread_wait(&th);
	if (!thret) {
		RTM_FATAL("%s", arg.ret);
	}

	ret = ivm_string_object_new_rl(NAT_STATE(), arg.ret, arg.len);
	STD_FREE(arg.ret);

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
