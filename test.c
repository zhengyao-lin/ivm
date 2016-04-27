#include <stdio.h>
#include "vm/vm.h"
#include "vm/gc/heap.h"
#include "vm/gc/gc.h"

IVM_NATIVE_FUNC(test)
{
	printf("from native!!\n");
	return IVM_NULL;
}

int main()
{
	ivm_vmstate_t *state = ivm_vmstate_new();

	ivm_object_t *obj1 = ivm_object_new(state);
	ivm_object_t *obj2 = ivm_object_new(state);

	ivm_exec_t *exec1 = ivm_exec_new(),
			   *exec2 = ivm_exec_new();
	ivm_ctchain_t *chain = ivm_ctchain_new();
	ivm_function_t *func1, *func2, *func3;
	ivm_coro_t *coro1, *coro2;

	ivm_object_setSlot(obj1, state, "a", obj2);
	ivm_object_setSlot(obj2, state, "b", obj1);
	ivm_object_setSlot(obj2, state, "c", obj1);

	ivm_exec_addCode(exec1, IVM_OP(NEW_OBJ), "");
	ivm_exec_addCode(exec1, IVM_OP(NEW_OBJ), "");
	ivm_exec_addCode(exec1, IVM_OP(NEW_OBJ), "");
	ivm_exec_addCode(exec1, IVM_OP(NEW_OBJ), "");
	
	ivm_exec_addCode(exec1, IVM_OP(SET_SLOT), "$s", "slot_a");
	ivm_exec_addCode(exec1, IVM_OP(GET_SLOT), "$s", "slot_a");
	ivm_exec_addCode(exec1, IVM_OP(GET_SLOT), "$s", "slot_a");
	
	ivm_exec_addCode(exec1, IVM_OP(PRINT_OBJ), "$i8$i8", IVM_OP(PRINT_OBJ), IVM_OP(PRINT_OBJ));
	ivm_exec_addCode(exec1, IVM_OP(NEW_OBJ), "");
	ivm_exec_addCode(exec1, IVM_OP(YIELD), "");
	ivm_exec_addCode(exec1, IVM_OP(NEW_OBJ), "");
	ivm_exec_addCode(exec1, IVM_OP(YIELD), "");
	ivm_exec_addCode(exec1, IVM_OP(TEST1), "");

	ivm_exec_addCode(exec2, IVM_OP(NEW_OBJ), "");
	ivm_exec_addCode(exec2, IVM_OP(PRINT_OBJ), "");
	ivm_exec_addCode(exec2, IVM_OP(TEST3), "$s", "yes, I am");
	ivm_exec_addCode(exec2, IVM_OP(NEW_OBJ), "");
	ivm_exec_addCode(exec2, IVM_OP(YIELD), "");
	ivm_exec_addCode(exec2, IVM_OP(NEW_OBJ), "");
	ivm_exec_addCode(exec2, IVM_OP(YIELD), "");
	ivm_exec_addCode(exec2, IVM_OP(TEST3), "$s", "hello?");

	printf("obj1: %p\n", (void *)obj1);
	printf("obj2: %p\n", (void *)obj2);

	ivm_ctchain_addContext(chain, obj1);
	ivm_ctchain_addContext(chain, obj2);
	ivm_ctchain_removeContext(chain, obj2);
	ivm_ctchain_addContext(chain, obj2);

	printf("slot a in context chain: %p\n",
		   (void *)ivm_ctchain_search(chain, state, "a"));
	printf("slot c in context chain: %p\n",
		   (void *)ivm_ctchain_search(chain, state, "c"));

	func1 = ivm_function_newNative(IVM_GET_NATIVE_FUNC(test), IVM_INTSIG_NONE);
	func2 = ivm_function_new(chain, exec1, IVM_INTSIG_NONE);
	func3 = ivm_function_new(chain, exec2, IVM_INTSIG_NONE);

	coro1 = ivm_coro_new();
	coro2 = ivm_coro_new();

	ivm_vmstate_addCoro(state, coro1);
	ivm_vmstate_addCoro(state, coro2);

#if 1
	ivm_coro_setRoot(coro1, func2);
	ivm_coro_setRoot(coro2, func3);
	ivm_vmstate_schedule(state);

#else
	printf("***start***\n\n");

	printf("***starting coro1***\n");
	ivm_coro_start(coro1, state, func2);
	printf("***starting coro2***\n");
	ivm_coro_start(coro2, state, func3);
	printf("***resume coro1***\n");
	ivm_coro_resume(coro1, state);
	printf("***resume coro2***\n");
	ivm_coro_resume(coro2, state);
	printf("***resume coro1***\n");
	ivm_coro_resume(coro1, state);
	printf("***resume coro2***\n");
	ivm_coro_resume(coro2, state);

	printf("\n***all end***\n");

	ivm_coro_start(coro1, state, IVM_NULL);
#endif

	ivm_coro_free(coro1); ivm_coro_free(coro2);
	ivm_function_free(func1); ivm_function_free(func2); ivm_function_free(func3);
	ivm_ctchain_free(chain);
	ivm_exec_free(exec1); ivm_exec_free(exec2);
	ivm_vmstate_free(state);

	return 0;
}
