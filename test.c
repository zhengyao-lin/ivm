#include <stdio.h>
#include <time.h>
#include "pub/const.h"
#include "vm/vm.h"
#include "vm/dbg.h"
#include "vm/gc/heap.h"
#include "vm/gc/gc.h"

IVM_NATIVE_FUNC(test)
{
	printf("from native!!\n");
	return IVM_NULL;
}

#if IVM_PERF_PROFILE

extern clock_t ivm_perf_gc_time;
clock_t ivm_perf_program_start;

void profile_start()
{
	ivm_perf_program_start = clock();
	return;
}

void profile_output()
{
	clock_t now = clock();
	clock_t prog = now - ivm_perf_program_start;

	printf("\n***performance profile***\n\n");
	printf("program: %ld ticks(%fs)\n", prog, (double)prog / CLOCKS_PER_SEC);
	printf("gc: %ld ticks(%fs)\n", ivm_perf_gc_time, (double)ivm_perf_gc_time / CLOCKS_PER_SEC);
	printf("gc per program: %f%%\n", (double)ivm_perf_gc_time / prog * 100);
	printf("\n***performance profile***\n\n");

	return;
}

#endif

int main()
{
	ivm_vmstate_t *state;
	ivm_object_t *obj1, *obj2;
	ivm_exec_t *exec1, *exec2, *exec3, *exec4;
	ivm_ctchain_t *chain;
	ivm_function_t *func1, *func2, *func3;
	ivm_coro_t *coro1, *coro2;
	ivm_size_t addr1, addr2, addr3, addr4;
	ivm_string_pool_t *str_pool;

	state = ivm_vmstate_new(); ivm_vmstate_lockGCFlag(state); /* block gc for a while */
	obj1 = ivm_function_object_new_nc(state, ivm_function_newNative(IVM_GET_NATIVE_FUNC(test), IVM_INTSIG_NONE));
	obj2 = ivm_numeric_new(state, 110);
	str_pool = ivm_string_pool_new();
	exec1 = ivm_exec_new(str_pool);
	exec2 = ivm_exec_new(str_pool);
	exec3 = ivm_exec_new(str_pool);
	exec4 = ivm_exec_new(str_pool);
	chain = ivm_ctchain_new();

	printf("%f\n", IVM_AS(obj2, ivm_numeric_t)->val);

	ivm_object_setSlot(obj1, state, "a", obj2);
	ivm_object_setSlot(obj2, state, "b", obj1);
	ivm_object_setSlot(obj2, state, "c", obj1);
	ivm_object_setSlot(obj2, state, "d", obj1);

	/* add opcodes */
	ivm_exec_addCode(exec3, IVM_OP(TEST3), "$s", "this is exec3");
	ivm_exec_addOp(exec3, IVM_OP(SET_ARG), "arg");
	ivm_exec_addOp(exec3, IVM_OP(GET_CONTEXT_SLOT), "arg");
	ivm_exec_addOp(exec3, IVM_OP(PRINT_NUM));
	ivm_exec_addOp(exec3, IVM_OP(NEW_NUM_i), 5252);
	ivm_exec_addOp(exec3, IVM_OP(SET_CONTEXT_SLOT), "in_closure");
	ivm_exec_addOp(exec3, IVM_OP(NEW_FUNC), ivm_vmstate_registerExec(state, exec4));

	ivm_exec_addCode(exec4, IVM_OP(TEST3), "$s", "this is exec4");
	ivm_exec_addCode(exec4, IVM_OP(TEST3), "$s", "now print in_closure");
	ivm_exec_addCode(exec4, IVM_OP(GET_CONTEXT_SLOT), "$s", "in_closure");
	ivm_exec_addCode(exec4, IVM_OP(PRINT_NUM), "");

	ivm_exec_addCode(exec1, IVM_OP(NEW_NUM_i), "$i32", 10202);
	ivm_exec_addCode(exec1, IVM_OP(NEW_FUNC), "$i32", ivm_vmstate_registerExec(state, exec3));
	ivm_exec_addCode(exec1, IVM_OP(DUP), "");
	ivm_exec_addCode(exec1, IVM_OP(SET_CONTEXT_SLOT), "$s", "func");
	ivm_exec_addCode(exec1, IVM_OP(INVOKE), "$i32", 1);
	ivm_exec_addCode(exec1, IVM_OP(INVOKE), "$i32", 0);

	ivm_exec_addCode(exec1, IVM_OP(NEW_OBJ), "");
	ivm_exec_addCode(exec1, IVM_OP(NEW_OBJ), "");
	ivm_exec_addCode(exec1, IVM_OP(NEW_OBJ), "");
	ivm_exec_addCode(exec1, IVM_OP(NEW_OBJ), "");

	/*for (i = 0; i < 1000; i++) {
		ivm_exec_addCode(exec1, IVM_OP(NEW_OBJ), "");
		ivm_exec_addCode(exec1, IVM_OP(POP), "");
	}*/

	ivm_exec_addCode(exec1, IVM_OP(NEW_NUM_i), "$i32", 1002);
	ivm_exec_addCode(exec1, IVM_OP(PRINT_NUM), "");
	
	ivm_exec_addCode(exec1, IVM_OP(SET_SLOT), "$s", "slot_a");
	ivm_exec_addCode(exec1, IVM_OP(GET_SLOT), "$s", "slot_a");
	ivm_exec_addCode(exec1, IVM_OP(PRINT_OBJ), "");
	ivm_exec_addCode(exec1, IVM_OP(GET_SLOT), "$s", "slot_a");

	ivm_exec_addCode(exec1, IVM_OP(GET_CONTEXT_SLOT), "$s", "c");
	ivm_exec_addCode(exec1, IVM_OP(INVOKE), "$i32", 0);
	
	ivm_exec_addCode(exec1, IVM_OP(GET_CONTEXT_SLOT), "$s", "func");
	ivm_exec_addCode(exec1, IVM_OP(INVOKE), "$i32", 0);

	addr1 = ivm_exec_addCode(exec1, IVM_OP(JUMP_i), "$i32", 0);
	addr3 = ivm_exec_addCode(exec1, IVM_OP(NEW_NULL), "");
	addr4 = ivm_exec_addCode(exec1, IVM_OP(JUMP_IF_FALSE_i), "$i32", 0);
	addr2 = ivm_exec_addCode(exec1, IVM_OP(NOP), "");
	ivm_exec_rewriteArg(exec1, addr1, "$i32", addr2);

	ivm_exec_addCode(exec1, IVM_OP(JUMP_i), "$i32", addr3);
	addr2 = ivm_exec_addCode(exec1, IVM_OP(NOP), "");
	ivm_exec_rewriteArg(exec1, addr4, "$i32", addr2);
	
	ivm_exec_addCode(exec1, IVM_OP(PRINT_OBJ), "");
	ivm_exec_addCode(exec1, IVM_OP(NEW_OBJ), "");
	ivm_exec_addCode(exec1, IVM_OP(YIELD), "");
	ivm_exec_addCode(exec1, IVM_OP(NEW_OBJ), "");
	ivm_exec_addCode(exec1, IVM_OP(YIELD), "");
	ivm_exec_addCode(exec1, IVM_OP(TEST1), "");
	ivm_exec_addCode(exec1, IVM_OP(TEST2), "$i8$i8$i8", 0x7f, 0xff, 0x7f);

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

	/* some test on context chain */
	ivm_ctchain_addContext(chain, obj1);
	ivm_ctchain_addContext(chain, obj2);
	ivm_ctchain_removeContext(chain, obj2);
	ivm_ctchain_addContext(chain, obj2);

	/* init functions & coroutines */
	func1 = ivm_function_newNative(IVM_GET_NATIVE_FUNC(test), IVM_INTSIG_NONE);
	func2 = ivm_function_new(chain, exec1, IVM_INTSIG_NONE);
	func3 = ivm_function_new(chain, exec2, IVM_INTSIG_NONE);

	coro1 = ivm_coro_new();
	coro2 = ivm_coro_new();

	/* add coroutines to vm state */
	ivm_vmstate_addCoro(state, coro1);
	ivm_vmstate_addCoro(state, coro2);

	ivm_coro_setRoot(coro1, state, func2);
	ivm_coro_setRoot(coro2, state, func3);
	/*for (i = 0; i < 100000; i++) {
		ivm_object_new(state);
	}*/

	ivm_vmstate_unlockGCFlag(state);

#if IVM_PERF_PROFILE
	profile_start();
#endif

	/* start executing */
	ivm_vmstate_schedule(state);

#if IVM_PERF_PROFILE
	profile_output();
#endif

	fprintf(stderr, "disasm exec1:\n");
	ivm_dbg_disAsmExec_c(exec1, "  ", stderr);

	fprintf(stderr, "\n");

	fprintf(stderr, "disasm exec2:\n");
	ivm_dbg_disAsmExec_c(exec2, "  ", stderr);

	fprintf(stderr, "\n");

	fprintf(stderr, "disasm exec3:\n");
	ivm_dbg_disAsmExec_c(exec3, "  ", stderr);

	fprintf(stderr, "\n");

	fprintf(stderr, "disasm exec4:\n");
	ivm_dbg_disAsmExec_c(exec4, "  ", stderr);

	ivm_coro_free(coro1); ivm_coro_free(coro2);
	ivm_function_free(func1); ivm_function_free(func2); ivm_function_free(func3);
	ivm_ctchain_free(chain);
	ivm_exec_free(exec1); ivm_exec_free(exec2);
	ivm_exec_free(exec3); ivm_exec_free(exec4);
	ivm_string_pool_free(str_pool);
	ivm_vmstate_free(state);

	return 0;
}
