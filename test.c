#include <stdio.h>
#include "vm/vm.h"

int main()
{
	ivm_object_t *obj1 = ivm_new_obj();
	ivm_object_t *obj2 = ivm_new_obj();

	ivm_obj_set_slot(obj1, "a", obj2);
	ivm_obj_set_slot(obj2, "b", obj1);

	printf("obj1: %p\n", (void *)obj1);
	printf("obj2: %p\n", (void *)obj2);

	printf("obj1.a: %p\n", (void *)ivm_slot_get_value(ivm_obj_get_slot(obj1, "a")));
	printf("obj2.b: %p\n", (void *)ivm_slot_get_value(ivm_obj_get_slot(obj2, "b")));

	ivm_free_obj(obj1);
	ivm_free_obj(obj2);

	return 0;
}
