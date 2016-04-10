#include <stdio.h>
#include "vm/vm.h"
#include "vm/gc/gc.h"

int main()
{
	ivm_vmstate_t *state = ivm_new_state();
	ivm_cell_set_t *set1 = ivm_new_cell_set();
	ivm_cell_set_t *set2 = ivm_new_cell_set();

	ivm_object_t *obj1 = ivm_new_object(state);
	ivm_object_t *obj2 = ivm_new_object(state);

	ivm_cell_t *cell1;

	ivm_cell_set_add_cell(set1, ivm_new_cell(obj1));
	ivm_cell_set_add_cell(set2, cell1 = ivm_new_cell(obj2));

	ivm_cell_move_to_set(cell1, set2, set1);

	ivm_obj_set_slot(state, obj1, "a", obj2);
	ivm_obj_set_slot(state, obj2, "b", obj1);

	printf("obj1: %p\n", (void *)obj1);
	printf("obj2: %p\n", (void *)obj2);

	printf("obj1.a: %p\n", (void *)ivm_obj_get_slot_value(state, obj1, "a"));
	printf("obj2.b: %p\n", (void *)ivm_obj_get_slot_value(state, obj2, "b"));

	ivm_dispose_cell_set(state, set1);
	ivm_dispose_cell_set(state, set2);
	ivm_free_state(state);

	return 0;
}
