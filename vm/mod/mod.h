#ifndef _IVM_VM_MOD_MOD_H_
#define _IVM_VM_MOD_MOD_H_

#include "pub/type.h"
#include "pub/com.h"

IVM_COM_HEADER

/*
	module search order:
		1. current path(specified by the front-end)
		2. paths specified in environment var "IVM_MOD_PATH"(separated by semicolons)

	module structure:
		1. directory with the name of the module and an init.ivc or an init.so file in it
		   e.g.
				- mymod
					- util.so
					- std.so
					- init.so <- start exec

		2. a single ivc or so file with the module name
 */

IVM_COM_END

#endif
