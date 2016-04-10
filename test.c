#include <stdio.h>
#include "vm/vm.h"
#include "vm/gc/heap.h"

int main()
{
	ivm_vmstate_t *state = ivm_state_new();

	ivm_object_t *obj1 = ivm_object_new(state);
	ivm_object_t *obj2 = ivm_object_new(state);
	
	/*
	ivm_new_object(state);
	ivm_new_object(state);
	ivm_new_object(state);
	ivm_new_object(state);
	*/

	ivm_object_setSlot(state, obj1, "a", obj2);
	ivm_object_setSlot(state, obj2, "b", obj1);

	printf("obj1: %p\n", (void *)obj1);
	printf("obj2: %p\n", (void *)obj2);

	printf("obj1.a: %p\n", (void *)ivm_object_getSlotValue(state, obj1, "a"));
	printf("obj2.b: %p\n", (void *)ivm_object_getSlotValue(state, obj2, "b"));

	ivm_object_free(state, obj1);
	ivm_object_free(state, obj2);
	ivm_state_free(state);

	return 0;
}
