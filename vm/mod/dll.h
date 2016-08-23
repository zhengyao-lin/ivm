#ifndef _IVM_VM_MOD_DLL_H_
#define _IVM_VM_MOD_DLL_H_

#include "pub/type.h"
#include "pub/com.h"

IVM_COM_HEADER

/*
	types:
		1. ivm_dll_t

	common interfaces:
		1. handler ivm_dll_open(path)
		2. void ivm_dll_close(handler)
		3. func ivm_dll_getFunc(handler, fn_name, fn_type)
		4. string ivm_dll_error(handler)
 */

#if defined(IVM_OS_LINUX)

	#include <dlfcn.h>

	typedef void *ivm_dll_t;

	#define ivm_dll_open(path) dlopen((path), RTLD_NOW | RTLD_LOCAL)
	#define ivm_dll_close(handler) dlclose(handler)
	#define ivm_dll_getFunc(handler, name, type) \
		((type)dlsym((handler), (name)))
	#define ivm_dll_error(handler) dlerror()

#elif defined(IVM_OS_WIN32)
#else
	#error doesn't support dynamically linked library on this platform
#endif

IVM_COM_END

#endif
