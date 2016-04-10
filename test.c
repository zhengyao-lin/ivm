#include <stdio.h>
#include "vm/vm.h"
#include "vm/gc/heap.h"

int main()
{
	ivm_vmstate_t *state = ivm_new_state();

	ivm_object_t *obj1 = ivm_new_object(state);
	ivm_object_t *obj2 = ivm_new_object(state);
	
	/*
	ivm_new_object(state);
	ivm_new_object(state);
	ivm_new_object(state);
	ivm_new_object(state);
	*/

	ivm_obj_set_slot(state, obj1, "a", obj2);
	ivm_obj_set_slot(state, obj2, "b", obj1);

	printf("obj1: %p\n", (void *)obj1);
	printf("obj2: %p\n", (void *)obj2);

	printf("obj1.a: %p\n", (void *)ivm_obj_get_slot_value(state, obj1, "a"));
	printf("obj2.b: %p\n", (void *)ivm_obj_get_slot_value(state, obj2, "b"));

	ivm_free_object(state, obj1);
	ivm_free_object(state, obj2);
	ivm_free_state(state);

	return 0;
}
