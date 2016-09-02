#ifndef _IVM_VM_MOD_DLL_H_
#define _IVM_VM_MOD_DLL_H_

#include "pub/type.h"
#include "pub/com.h"

#include "std/string.h"

IVM_COM_HEADER

/*
	type:
		1. ivm_dll_t

	common interfaces:
		1. bool ivm_dll_open(handler_ptr, path)
		2. void ivm_dll_close(handler)
		3. func ivm_dll_getFunc(handler, fn_name, fn_type)
		4. string ivm_dll_error(handler, is_const) // if the message need free, is_const will be set to false
		5. string ivm_dll_freeError(str) // free the message string

	const:
		1. IVM_DLL_SUFFIX
 */

#if defined(IVM_OS_LINUX)

	#include <dlfcn.h>

	typedef void *ivm_dll_t;

	#define ivm_dll_open(handler, path) ((*(handler) = dlopen((path), RTLD_NOW | RTLD_LOCAL)) != IVM_NULL)
	#define ivm_dll_close(handler) dlclose(handler)
	#define ivm_dll_getFunc(handler, name, type) \
		((type)(ivm_ptr_t)dlsym((handler), (name)))

	IVM_INLINE
	ivm_char_t *
	ivm_dll_error(ivm_dll_t handler, ivm_bool_t *is_const)
	{
		*is_const = IVM_TRUE;
		return dlerror();
	}

	#define ivm_dll_freeError(str) STD_FREE(str)

	#define IVM_DLL_SUFFIX ".so"

#elif defined(IVM_OS_WIN32)

	#include <windows.h>

	typedef HMODULE ivm_dll_t;

	#define ivm_dll_open(handler, path) ((*(handler) = LoadLibrary(path)) != IVM_NULL)
	#define ivm_dll_close(handler) FreeLibrary(handler)
	#define ivm_dll_getFunc(handler, name, type) \
		((type)GetProcAddress((handler), (name)))

	IVM_INLINE
	ivm_char_t *
	ivm_dll_error(ivm_dll_t handler, ivm_bool_t *is_const)
	{
		LPVOID buf;
		DWORD err = GetLastError();

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			IVM_NULL,
			err,
			0, (LPTSTR)&buf, 0, NULL
		);

		*is_const = IVM_FALSE;

		return buf;
	}

	#define ivm_dll_freeError(str) LocalFree(str)

	#define IVM_DLL_SUFFIX ".dll"

#else
	#error no support for dynamically linked library on this os
#endif

IVM_COM_END

#endif
