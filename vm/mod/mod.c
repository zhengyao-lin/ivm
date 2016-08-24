#include <stdlib.h>

#include "pub/const.h"
#include "pub/type.h"
#include "pub/com.h"
#include "pub/err.h"
#include "pub/mem.h"
#include "pub/vm.h"

#include "std/string.h"
#include "std/io.h"
#include "std/env.h"
#include "std/list.h"

#include "mod.h"
#include "dll.h"

typedef ivm_ptlist_t ivm_mod_path_list_t;
typedef IVM_PTLIST_ITER_TYPE(ivm_char_t *) ivm_mod_path_list_iteartor_t;

#define ivm_mod_path_list_init(list) ivm_ptlist_init_c((list), IVM_DEFAULT_MOD_PATH_LIST_BUFFER_SIZE)
#define ivm_mod_path_list_dump(list) ivm_ptlist_dump(list)
#define ivm_mod_path_list_push(list, val) ivm_ptlist_push((list), (val))

#define IVM_MOD_PATH_LIST_ITER_GET(iter) IVM_PTLIST_ITER_GET(iter)
#define IVM_MOD_PATH_LIST_EACHPTR(list, iter) IVM_PTLIST_EACHPTR((list), iter, ivm_char_t *)
#define IVM_MOD_PATH_LIST_EACHPTR_R(list, iter) IVM_PTLIST_EACHPTR_R((list), iter, ivm_char_t *)

typedef ivm_list_t ivm_dll_list_t;
typedef IVM_LIST_ITER_TYPE(ivm_dll_t) ivm_dll_list_iteartor_t;

#define ivm_dll_list_init(list) ivm_list_init_c((list), sizeof(ivm_dll_t), IVM_DEFAULT_DLL_LIST_BUFFER_SIZE)
#define ivm_dll_list_dump(list) ivm_list_dump(list)
#define ivm_dll_list_push(list, val) ivm_list_push((list), (val))

#define IVM_DLL_LIST_ITER_GET(iter) IVM_LIST_ITER_GET(iter, ivm_dll_t)
#define IVM_DLL_LIST_EACHPTR(list, iter) IVM_LIST_EACHPTR((list), iter, ivm_dll_t)

IVM_PRIVATE
ivm_mod_path_list_t
_mod_path_list;

IVM_PRIVATE
ivm_dll_list_t
_dll_list;

// BUG: need thread lock

IVM_PRIVATE
void
_ivm_mod_addModPath(const ivm_char_t *path)
{
	ivm_mod_path_list_push(&_mod_path_list, IVM_STRDUP(path));
	return;
}

IVM_PRIVATE
void
_ivm_mod_addModPath_l(const ivm_char_t *path,
					  ivm_size_t len)
{
	ivm_mod_path_list_push(&_mod_path_list, IVM_STRNDUP(path, len));
	return;
}

IVM_PRIVATE
IVM_INLINE
void
_ivm_mod_initModPath()
{
	const ivm_char_t *paths = ivm_env_getVar("IVM_MOD_PATH"),
					 *start, *end;
	ivm_size_t len;

#if IVM_DEBUG
	_ivm_mod_addModPath(IVM_LIB_PATH);
#endif
	_ivm_mod_addModPath(IVM_MOD_DEFAULT_SEARCH_PATH);

	if (paths) {
		for (len = 0, start = end = paths;
			 *end; end++) {
			if (*end == ';') {
				if (len) {
					_ivm_mod_addModPath_l(start, len);
					len = 0;
				}
				start = end + 1;
			} else {
				len++;
			}
		}

		if (len) {
			// last path
			_ivm_mod_addModPath_l(start, len);
		}
	}

	/* current path */
	_ivm_mod_addModPath(".");

	return;
}

ivm_int_t
ivm_mod_init()
{
	ivm_mod_path_list_init(&_mod_path_list);
	ivm_dll_list_init(&_dll_list);
	_ivm_mod_initModPath();
	return 0;
}

void
ivm_mod_clean()
{
	ivm_mod_path_list_iteartor_t miter;
	ivm_dll_list_iteartor_t diter;

	IVM_MOD_PATH_LIST_EACHPTR(&_mod_path_list, miter) {
		// IVM_TRACE("mod path: %s\n",IVM_MOD_PATH_LIST_ITER_GET(iter));
		MEM_FREE(IVM_MOD_PATH_LIST_ITER_GET(miter));
	}
	ivm_mod_path_list_dump(&_mod_path_list);

	IVM_DLL_LIST_EACHPTR(&_dll_list, diter) {
		ivm_dll_close(IVM_DLL_LIST_ITER_GET(diter));
	}
	ivm_dll_list_dump(&_dll_list);

	return;
}

ivm_int_t
ivm_mod_search(const ivm_char_t *mod_name,
			   ivm_char_t *path_buf,
			   ivm_size_t buf_size)
{
	ivm_mod_path_list_iteartor_t iter;
	const ivm_char_t *tmp;
	ivm_char_t *brk;
	ivm_size_t len1, len2, len3;
	ivm_int_t i;

	struct {
		const ivm_char_t *suf;
		ivm_size_t alloc;
		ivm_int_t ret;
	} sufs[] = {
		{ ".ivc",		5,		IVM_MOD_TYPE_IVC },
		{ ".so",		4,		IVM_MOD_TYPE_DLL },
		{ "/init.so",	9,		IVM_MOD_TYPE_DLL },
		{ "/init.ivc",	10,		IVM_MOD_TYPE_IVC },
	};

	len2 = IVM_STRLEN(mod_name);

	IVM_MOD_PATH_LIST_EACHPTR_R(&_mod_path_list, iter) {
		tmp = IVM_MOD_PATH_LIST_ITER_GET(iter);
		len1 = IVM_STRLEN(tmp);

		/*
			len1          1              len2          9        1
			tmp + IVM_FILE_SEPARATOR + mod_name + "/init.ivc"   \0
		 */
		ivm_char_t buf[len1 + 1 + len2 + 9 + 1];

		MEM_COPY(buf, tmp, len1);
		buf[len1] = IVM_FILE_SEPARATOR;
		MEM_COPY(buf + len1 + 1, mod_name, len2);
		brk = buf + len1 + 1 + len2;

		for (i = 0; i < IVM_ARRLEN(sufs); i++) {
			MEM_COPY(brk, sufs[i].suf, sufs[i].alloc);
			IVM_TRACE("mod search for %s\n", buf);
			if (ivm_file_access(buf, IVM_FMODE_READ_BINARY)) {
				len3 = len1 + 1 + len2 + sufs[i].alloc /* include '\0' */;
				len3 = buf_size > len3 ? len3 : buf_size;
				MEM_COPY(path_buf, buf, len3);
				path_buf[len3] = '\0'; /* in case buf_size is smaller than string length */

				return sufs[i].ret;
			}
		}
	}

	return IVM_FALSE;
}

ivm_object_t *
ivm_mod_loadNative(const ivm_char_t *path,
				   const ivm_char_t **err,
				   ivm_vmstate_t *state,
				   ivm_coro_t *coro,
				   ivm_context_t *context)
{
	const ivm_char_t *tmp_err = IVM_NULL;
	ivm_dll_t handler;
	ivm_mod_native_init_t init;
	ivm_object_t *ret = IVM_NULL;

	if (!ivm_dll_open(&handler, path)) {
		tmp_err = ivm_dll_error(handler);
		if (!tmp_err) {
			tmp_err = "unknown error";
		}

		goto END;
	}

	init = ivm_dll_getFunc(handler, IVM_MOD_NATIVE_INIT_FUNC, ivm_mod_native_init_t);

	if (!init) {
		ivm_dll_close(handler);

		tmp_err = ivm_dll_error(handler);
		if (!tmp_err) {
			tmp_err = "failed to load init function";
		}

		goto END;
	}

	ivm_dll_list_push(&_dll_list, &handler);

	ret = init(state, coro, context);

END:

	if (err) {
		*err = tmp_err;
	}

	return ret;
}
