#include <stdio.h>
#include <stdlib.h>

#include "pub/type.h"
#include "pub/vm.h"
#include "pub/inlines.h"

#include "vm/mod/mod.h"
#include "vm/native/native.h"
#include "vm/native/priv.h"

IVM_NATIVE_FUNC(_mod_addPath)
{
	const ivm_string_t *path;
	const ivm_char_t *cur;
	ivm_char_t *buf;
	ivm_size_t len, tmp;

	MATCH_ARG("s", &path);
	
	cur = ivm_vmstate_curPath(NAT_STATE());
	tmp = cur ? IVM_STRLEN(cur) : 0;
	len = tmp + ivm_string_length(path);

	buf = STD_ALLOC(sizeof(ivm_char_t) * (len + 2));

	RTM_MEMCHECK(buf);

	STD_MEMCPY(buf, cur, tmp);
	buf[tmp] = IVM_FILE_SEPARATOR;
	STD_MEMCPY(buf + tmp + 1, ivm_string_trimHead(path), ivm_string_length(path));

	buf[len + 1] = '\0';

	ivm_mod_addModPath(buf);

	STD_FREE(buf);

	return IVM_NONE(NAT_STATE());
}

ivm_object_t *
ivm_mod_main(ivm_vmstate_t *state,
			 ivm_coro_t *coro,
			 ivm_context_t *context)
{
	ivm_object_t *mod = ivm_object_new(state);

	ivm_object_setSlot_r(mod, state, "addPath", IVM_NATIVE_WRAP(state, _mod_addPath));

	return mod;
}
