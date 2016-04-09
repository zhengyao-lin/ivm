#include <stdio.h>
#include "vm/vm.h"

int main()
{
	ivm_object_t *obj = ivm_new_obj();

	ivm_free_obj(obj);

	return 0;
}
