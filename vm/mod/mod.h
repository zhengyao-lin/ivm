#ifndef _IVM_VM_MOD_MOD_H_
#define _IVM_VM_MOD_MOD_H_

#include "pub/type.h"
#include "pub/com.h"

IVM_COM_HEADER

struct ivm_object_t_tag;
struct ivm_vmstate_t_tag;
struct ivm_coro_t_tag;
struct ivm_context_t_tag;

/*
	module search order:
		1. current path(specified by the front-end)
		2. paths specified in environment var "IVM_MOD_PATH"(separated by semicolons)
		3. IVM_LIB_PATH "/ivm/mod"

	module structure:
		1. directory with the name of the module and an init.ivc or an init.so file in it
		   e.g.
				- mymod
					- util.so
					- std.so
					- init.so <- start exec

		2. a single ivc or so file with the module name

	native modules:
		1. must have "ivm_mod_main" function with the signatue:
			ivm_object_t *ivm_mod_main(ivm_vmstate_t *state,
									   ivm_coro_t *coro,
									   ivm_context_t *context);
 */

#define IVM_MOD_DEFAULT_SEARCH_PATH (IVM_LIB_PATH "/ivm/mod")
#define IVM_MOD_NATIVE_INIT_FUNC "ivm_mod_main"

ivm_int_t
ivm_mod_init();

void
ivm_mod_clean();

enum {
	IVM_MOD_TYPE_IVC = 1,
	IVM_MOD_TYPE_DLL
};

typedef
struct ivm_object_t_tag *
(*ivm_mod_native_init_t)(struct ivm_vmstate_t_tag *state,
						 struct ivm_coro_t_tag *coro,
						 struct ivm_context_t_tag *context);

typedef
struct ivm_object_t_tag *
(*ivm_mod_loader_t)(const ivm_char_t *path,
					ivm_char_t **err,
					ivm_bool_t *is_const,
					struct ivm_vmstate_t_tag *state,
					struct ivm_coro_t_tag *coro,
					struct ivm_context_t_tag *context);

void
ivm_mod_addModSuffix(const ivm_char_t *suffix,
					 ivm_mod_loader_t loader);

/*
	return the loader function pointer
 */
ivm_mod_loader_t
ivm_mod_search(const ivm_char_t *mod_name,
			   ivm_char_t *path_buf,
			   ivm_size_t buf_size);

ivm_object_t *
ivm_mod_load(const ivm_string_t *mod_name,
			 ivm_vmstate_t *state,
			 ivm_coro_t *coro,
			 ivm_context_t *context);

IVM_COM_END

#endif
