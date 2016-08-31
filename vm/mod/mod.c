#include <stdlib.h>

#include "pub/const.h"
#include "pub/type.h"
#include "pub/com.h"
#include "pub/err.h"
#include "pub/mem.h"
#include "pub/inlines.h"
#include "pub/vm.h"

#include "std/string.h"
#include "std/io.h"
#include "std/env.h"
#include "std/sys.h"
#include "std/list.h"

#include "vm/serial.h"

#include "mod.h"
#include "dll.h"

typedef ivm_ptlist_t ivm_rawstr_list_t;
typedef IVM_PTLIST_ITER_TYPE(ivm_char_t *) ivm_rawstr_list_iteartor_t;

#define ivm_rawstr_list_init(list) ivm_ptlist_init_c((list), IVM_DEFAULT_RAWSTR_LIST_BUFFER_SIZE)
#define ivm_rawstr_list_dump ivm_ptlist_dump
#define ivm_rawstr_list_push ivm_ptlist_push
#define ivm_rawstr_list_pop(list) ((ivm_char_t *)ivm_ptlist_pop(list))

#define IVM_RAWSTR_LIST_ITER_GET(iter) IVM_PTLIST_ITER_GET(iter)
#define IVM_RAWSTR_LIST_EACHPTR(list, iter) IVM_PTLIST_EACHPTR((list), iter, ivm_char_t *)
#define IVM_RAWSTR_LIST_EACHPTR_R(list, iter) IVM_PTLIST_EACHPTR_R((list), iter, ivm_char_t *)

typedef ivm_list_t ivm_dll_list_t;
typedef IVM_LIST_ITER_TYPE(ivm_dll_t) ivm_dll_list_iteartor_t;

#define ivm_dll_list_init(list) ivm_list_init_c((list), sizeof(ivm_dll_t), IVM_DEFAULT_DLL_LIST_BUFFER_SIZE)
#define ivm_dll_list_dump(list) ivm_list_dump(list)
#define ivm_dll_list_push(list, val) ivm_list_push((list), (val))

#define IVM_DLL_LIST_ITER_GET(iter) IVM_LIST_ITER_GET(iter, ivm_dll_t)
#define IVM_DLL_LIST_EACHPTR(list, iter) IVM_LIST_EACHPTR((list), iter, ivm_dll_t)

typedef struct {
	const ivm_char_t *suf;
	ivm_size_t len;
	ivm_mod_loader_t loader;
} ivm_mod_suffix_t;

typedef ivm_list_t ivm_mod_suffix_list_t;
typedef IVM_LIST_ITER_TYPE(ivm_mod_suffix_t) ivm_mod_suffix_list_iteartor_t;

#define ivm_mod_suffix_list_init(list) ivm_list_init_c((list), sizeof(ivm_mod_suffix_t), 2)
#define ivm_mod_suffix_list_dump(list) ivm_list_dump(list)
#define ivm_mod_suffix_list_push(list, val) ivm_list_push((list), (val))

#define IVM_MOD_SUFFIX_LIST_ITER_GET(iter) IVM_LIST_ITER_GET(iter, ivm_mod_suffix_t)
#define IVM_MOD_SUFFIX_LIST_EACHPTR_R(list, iter) IVM_LIST_EACHPTR_R((list), iter, ivm_mod_suffix_t)

IVM_PRIVATE
ivm_rawstr_list_t _mod_path_list;

IVM_PRIVATE
ivm_size_t _mod_path_max_len = 0;

IVM_PRIVATE
ivm_dll_list_t _dll_list;

IVM_PRIVATE
ivm_mod_suffix_list_t _mod_suffix_list;

IVM_PRIVATE
ivm_size_t _mod_suffix_max_len = 0;

// BUG: need thread lock

IVM_PRIVATE
void
_ivm_mod_addModPath(const ivm_char_t *path)
{
	ivm_size_t len = IVM_STRLEN(path);

	if (len > _mod_path_max_len)
		_mod_path_max_len = len;

	ivm_rawstr_list_push(&_mod_path_list, IVM_STRNDUP(path, len));

	return;
}

IVM_PRIVATE
void
_ivm_mod_addModPath_n(ivm_char_t *path)
{
	ivm_size_t len;

	if (path) {
		len = IVM_STRLEN(path);

		if (len > _mod_path_max_len)
			_mod_path_max_len = len;
	}

	ivm_rawstr_list_push(&_mod_path_list, path);

	return;
}

IVM_PRIVATE
void
_ivm_mod_addModPath_l(const ivm_char_t *path,
					  ivm_size_t len)
{
	if (len > _mod_path_max_len)
		_mod_path_max_len = len;

	ivm_rawstr_list_push(&_mod_path_list, IVM_STRNDUP(path, len));

	return;
}

IVM_PRIVATE
void
_ivm_mod_popModPath_n()
{
	ivm_rawstr_list_pop(&_mod_path_list);
	return;
}

void
ivm_mod_addModSuffix(const ivm_char_t *suffix,
					 ivm_mod_loader_t loader)
{
	ivm_size_t len = IVM_STRLEN(suffix);
	ivm_mod_suffix_t suf = {
		suffix, len, loader
	};

	if (len > _mod_suffix_max_len)
		_mod_suffix_max_len = len;

	ivm_mod_suffix_list_push(&_mod_suffix_list, &suf);

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

	// _ivm_mod_addModPath(".");

	return;
}

ivm_int_t
ivm_mod_init()
{
	ivm_rawstr_list_init(&_mod_path_list);
	ivm_dll_list_init(&_dll_list);
	ivm_mod_suffix_list_init(&_mod_suffix_list);

	ivm_mod_addModSuffix(IVM_DLL_SUFFIX, ivm_mod_loadNative);
	ivm_mod_addModSuffix(IVM_CACHE_FILE_SUFFIX, ivm_mod_loadCache);

	_ivm_mod_initModPath();

	return 0;
}

void
ivm_mod_clean()
{
	ivm_rawstr_list_iteartor_t piter;
	ivm_dll_list_iteartor_t diter;

	IVM_RAWSTR_LIST_EACHPTR(&_mod_path_list, piter) {
		// IVM_TRACE("mod path: %s\n",IVM_RAWSTR_LIST_ITER_GET(piter));
		MEM_FREE(IVM_RAWSTR_LIST_ITER_GET(piter));
	}
	ivm_rawstr_list_dump(&_mod_path_list);

	IVM_DLL_LIST_EACHPTR(&_dll_list, diter) {
		ivm_dll_close(IVM_DLL_LIST_ITER_GET(diter));
	}
	ivm_dll_list_dump(&_dll_list);

	ivm_mod_suffix_list_dump(&_mod_suffix_list);

	return;
}

IVM_PRIVATE
IVM_INLINE
ivm_size_t
_get_max_buf_size(ivm_size_t mod_name_len)
{
	return _mod_path_max_len + 1 + mod_name_len + 5 + _mod_suffix_max_len + 1;
}

/* buffer guaranteed to have enough size */
IVM_PRIVATE
IVM_INLINE
ivm_mod_loader_t
_ivm_mod_search_c(const ivm_char_t *mod_name,
				  ivm_size_t mod_len,
				  ivm_char_t *buf)
{
	ivm_rawstr_list_iteartor_t iter;
	ivm_mod_suffix_list_iteartor_t siter;
	const ivm_char_t *tmp;
	ivm_char_t *brk;
	ivm_size_t len1, len2;
	ivm_int_t i;
	ivm_mod_suffix_t tmp_suf;

/*
	SUF(".ivc",						IVC)
	SUF(IVM_DLL_SUFFIX,				DLL)
	SUF("/init.ivc",				IVC)
	SUF("/init" IVM_DLL_SUFFIX,		DLL)
 */

	struct {
		const ivm_char_t *suf;
		ivm_size_t len;
	} sufs[] = {
#define SUF(val) \
	{ (val), sizeof(val) - 1 },

		SUF("")
		SUF("/init")

#undef SUF
	};

	len2 = mod_len;

	/*
		max situation:

		len1          1              len2        5         ?      1
		tmp + IVM_FILE_SEPARATOR + mod_name + "/init" + ".suf" + "\0"
	 */
	// ivm_char_t buf[_get_max_buf_size(len2)];

	IVM_RAWSTR_LIST_EACHPTR_R(&_mod_path_list, iter) {
		tmp = IVM_RAWSTR_LIST_ITER_GET(iter);

		if (!tmp) continue;

		len1 = IVM_STRLEN(tmp);

		MEM_COPY(buf, tmp, len1);
		buf[len1] = IVM_FILE_SEPARATOR;
		MEM_COPY(buf + len1 + 1, mod_name, len2);
		brk = buf + len1 + 1 + len2;

		for (i = 0; i < IVM_ARRLEN(sufs); i++) {
			MEM_COPY(brk, sufs[i].suf, sufs[i].len);

			IVM_MOD_SUFFIX_LIST_EACHPTR_R(&_mod_suffix_list, siter) {
				tmp_suf = IVM_MOD_SUFFIX_LIST_ITER_GET(siter);
				MEM_COPY(brk + sufs[i].len, tmp_suf.suf, tmp_suf.len + 1);

				// IVM_TRACE("mod search for %s %ld %s\n", buf, _mod_suffix_max_len, tmp_suf.suf);
				if (ivm_file_access(buf, IVM_FMODE_READ_BINARY)) {
					return tmp_suf.loader;
				}
			}
		}
	}

	return IVM_NULL;
}

ivm_mod_loader_t
ivm_mod_search(const ivm_char_t *mod_name,
			   ivm_char_t *path_buf,
			   ivm_size_t buf_size)
{
	/*
		max situation:

		len1          1              len2        5         ?      1
		tmp + IVM_FILE_SEPARATOR + mod_name + "/init" + ".suf" + "\0"
	 */
	ivm_size_t mod_len = IVM_STRLEN(mod_name), len;
	ivm_char_t buf[_get_max_buf_size(mod_len)];
	ivm_mod_loader_t ret;

	ret = _ivm_mod_search_c(mod_name, mod_len, buf);

	if (ret) {
		len = IVM_STRLEN(buf) + 1;

		len = buf_size >= len ? len : buf_size;
		MEM_COPY(path_buf, buf, len);
		path_buf[len - 1] = '\0'; /* in case buf_size is smaller than string length */
	}

	return ret;
}

ivm_object_t *
ivm_mod_load(const ivm_string_t *mod_name,
			 ivm_vmstate_t *state,
			 ivm_coro_t *coro,
			 ivm_context_t *context)
{
	ivm_mod_loader_t loader;
	ivm_size_t len = ivm_string_length(mod_name);
	ivm_char_t buf[_get_max_buf_size(len)];
	const ivm_char_t *err = IVM_NULL, *mod = ivm_string_trimHead(mod_name);
	ivm_object_t *ret;
	ivm_char_t *path_backup, *path;

	path_backup = ivm_vmstate_curPath(state);

	_ivm_mod_addModPath_n(path_backup);
	loader = _ivm_mod_search_c(mod, len, buf);
	_ivm_mod_popModPath_n();

	IVM_CORO_NATIVE_ASSERT(coro, state, loader, IVM_ERROR_MSG_MOD_NOT_FOUND(mod));

	ret = ivm_object_getSlot_r(ivm_vmstate_getLoadedMod(state), state, buf);
	if (ret) return ret;

	path = ivm_sys_getBasePath(buf);
	ivm_vmstate_setPath(state, path);

	// IVM_TRACE("========> cur path: %s %s\n", path, buf);
	ivm_object_setSlot_r(ivm_vmstate_getLoadedMod(state), state, buf, IVM_NONE(state));
	
	ret = loader(buf, &err, state, coro, context);

	ivm_object_setSlot_r(ivm_vmstate_getLoadedMod(state), state, buf, ret);
	ivm_vmstate_setPath(state, path_backup);
	MEM_FREE(path);

	if (!ret) {
		if (!ivm_vmstate_getException(state)) {
			IVM_CORO_NATIVE_FATAL(
				coro, state,
				IVM_ERROR_MSG_MOD_LOAD_ERROR(mod, buf, err)
			);
		}
	}

	return ret;
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
			tmp_err = IVM_ERROR_MSG_UNKNOWN_ERROR;
		}

		goto END;
	}

	init = ivm_dll_getFunc(handler, IVM_MOD_NATIVE_INIT_FUNC, ivm_mod_native_init_t);

	if (!init) {
		ivm_dll_close(handler);

		tmp_err = ivm_dll_error(handler);
		if (!tmp_err) {
			tmp_err = IVM_ERROR_MSG_FAILED_LOAD_INIT_FUNC;
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

ivm_object_t *
ivm_mod_loadCache(const ivm_char_t *path,
				  const ivm_char_t **err,
				  ivm_vmstate_t *state,
				  ivm_coro_t *coro,
				  ivm_context_t *context)
{
	const ivm_char_t *tmp_err = IVM_NULL;
	ivm_object_t *ret = IVM_NULL;
	ivm_file_t *cache = ivm_file_new(path, IVM_FMODE_READ_BINARY);
	ivm_exec_unit_t *unit = IVM_NULL;
	ivm_function_t *root;
	ivm_context_t *dest;

	if (!cache) {
		tmp_err = IVM_ERROR_MSG_FAILED_OPEN_FILE;
		goto END;
	}

	unit = ivm_serial_parseCacheFile(cache);

	ivm_file_free(cache);
	
	if (!unit) {
		tmp_err = IVM_ERROR_MSG_FAILED_PARSE_CACHE;
		goto END;
	}

	ivm_exec_unit_setOffset(unit, ivm_vmstate_getLinkOffset(state));
	root = ivm_exec_unit_mergeToVM(unit, state);

	ivm_exec_unit_free(unit);

	if (!root) {
		tmp_err = IVM_ERROR_MSG_CACHE_NO_ROOT;
		goto END;
	}

	ivm_function_invoke_r(root, state, coro, IVM_NULL);
	dest = ivm_context_addRef(ivm_coro_getRuntimeGlobal(coro));
	ivm_coro_resume(coro, state, IVM_NULL);

	ret = ivm_object_new_t(state, ivm_context_getSlotTable(dest));

	ivm_context_free(dest, state);

END:

	if (err) {
		*err = tmp_err;
	}

	return ret;
}
