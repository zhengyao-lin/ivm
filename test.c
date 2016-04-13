#include <stdio.h>
#include "vm/vm.h"
#include "vm/gc/heap.h"

int main()
{
	ivm_vmstate_t *state = ivm_vmstate_new();

	ivm_object_t *obj1 = ivm_object_new(state);
	ivm_object_t *obj2 = ivm_object_new(state);

	ivm_exec_t *exec = ivm_exec_new();

	ivm_vmstack_t *stack = ivm_vmstack_new();

	ivm_ctchain_t *chain = ivm_ctchain_new();

	ivm_exec_addCode(exec, IVM_OP_NEW_OBJ, 0);
	ivm_exec_addCode(exec, IVM_OP_NEW_OBJ, 0);

	ivm_object_setSlot(obj1, state, "a", obj2);
	ivm_object_setSlot(obj2, state, "b", obj1);

	printf("obj1: %p\n", (void *)obj1);
	printf("obj2: %p\n", (void *)obj2);

	ivm_ctchain_addContext(chain, obj1);
	ivm_ctchain_addContext(chain, obj2);
	ivm_ctchain_removeContext(chain, obj2);
	ivm_ctchain_addContext(chain, obj2);

	printf("slot a in context chain: %p\n",
		   (void *)ivm_ctchain_search(chain, state, "a"));

	ivm_vmstack_push(stack, obj1);
	ivm_vmstack_push(stack, obj2);
	printf("obj at stack top: %p\n", (void *)ivm_vmstack_top(stack));
	printf("obj at stack top(pop): %p\n", (void *)ivm_vmstack_pop(stack));
	printf("obj at stack top(pop): %p\n", (void *)ivm_vmstack_pop(stack));

	printf("obj1.a: %p\n", (void *)ivm_object_getSlotValue(obj1, state, "a"));
	printf("obj2.b: %p\n", (void *)ivm_object_getSlotValue(obj2, state, "b"));

	ivm_ctchain_free(chain);
	ivm_exec_free(exec);
	ivm_vmstack_free(stack);
	ivm_object_free(obj1, state);
	ivm_object_free(obj2, state);
	ivm_vmstate_free(state);

	return 0;
}
