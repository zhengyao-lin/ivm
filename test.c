#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "pub/const.h"
#include "vm/vm.h"
#include "vm/dbg.h"
#include "vm/gc/heap.h"
#include "vm/gc/gc.h"
#include "vm/std/pool.h"
#include "vm/std/chain.h"
#include "vm/std/hash.h"

IVM_NATIVE_FUNC(test)
{
	printf("from native!!\n");
	return IVM_NULL;
}

IVM_NATIVE_FUNC(call_func)
{
	ivm_function_object_t *func = IVM_AS(argv[0], ivm_function_object_t);
	printf("call function!!\n");

	ivm_function_invoke(func->val, state,
						func->closure, IVM_VMSTATE_GET(state, CUR_CORO));
	ivm_coro_resume(IVM_VMSTATE_GET(state, CUR_CORO), state, IVM_NULL);

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

	fprintf(stderr, "\n***performance profile***\n\n");
	fprintf(stderr, "program: %ld ticks(%fs)\n", prog, (double)prog / CLOCKS_PER_SEC);
	fprintf(stderr, "gc: %ld ticks(%fs)\n", ivm_perf_gc_time, (double)ivm_perf_gc_time / CLOCKS_PER_SEC);
	fprintf(stderr, "gc per program: %.4f%%\n", (double)ivm_perf_gc_time / prog * 100);
	fprintf(stderr, "\n***performance profile***\n\n");

	return;
}

#define print_type(type) (fprintf(stderr, #type ": %ld\n", sizeof(type)))

void profile_type()
{
	/*
	fprintf(stderr, "object_t: %ld\n", sizeof(ivm_object_t));
	fprintf(stderr, "function_t: %ld\n", sizeof(ivm_function_t));
	fprintf(stderr, "fun_t: %ld\n", sizeof(ivm_function_object_t));
	fprintf(stderr, "_t: %ld\n", sizeof(ivm_type_t));
	fprintf(stderr, "_t: %ld\n", sizeof(ivm_vmstate_t));
	fprintf(stderr, "_t: %ld\n", sizeof(ivm_exec_t));
	fprintf(stderr, "_t: %ld\n", sizeof(ivm_ctchain_t));
	*/
	fprintf(stderr, "***size of types***\n\n");
	print_type(ivm_object_t);
	print_type(ivm_function_t);
	print_type(ivm_function_object_t);
	print_type(ivm_type_t);
	print_type(ivm_vmstate_t);
	print_type(ivm_exec_t);
	print_type(ivm_ctchain_t);
	print_type(ivm_frame_t);
	fprintf(stderr, "\n***size of types***\n");

	return;
}

#endif

int test_vm()
{
	int i;
	ivm_vmstate_t *state;
	ivm_object_t *obj1, *obj2, *obj3, *proto;
	ivm_ctchain_t *chain;
	ivm_function_t *func1, *func2, *func3, *func4;
	ivm_coro_t *coro1, *coro2;
	ivm_size_t addr1, addr2, addr3, addr4;
	ivm_string_pool_t *str_pool;
	ivm_exec_t *exec1, *exec2, *exec3, *exec4;
	char *chartab[] = { "a", "b", "c", "d",
						"e", "f", "g", "h",
						"i", "j", "k", "l",
						"m", "n", "o", "p",
						"q", "r", "s", "t",
						"u", "v", "w", "x",
						"y", "z", "aa", "bb",
						"cc", "dd", "ee", "ff",
						"gg", "hh", "ii", "jj",
						"kk", "ll", "mm", "nn",
						"oo", "pp", "qq", "rr",
						"ss", "tt", "uu", "vv",
						"ww", "xx", "yy", "zz",
						"aaa", "bbb", "ccc", "ddd",
						"fff", "ggg", "hhh", "iii",
						"jjj", "kkk", "lll", "mmm",
						"nnn", "ooo", "ppp", "qqq",
						"rrr", "sss", "ttt", "uuu",
						"vvv", "www", "xxx", "yyy",
						"zzz" };

	state = ivm_vmstate_new(); ivm_vmstate_lockGCFlag(state); /* block gc for a while */
	obj1 = ivm_function_object_new_nc(state, IVM_NULL, ivm_function_newNative(state, IVM_GET_NATIVE_FUNC(test), IVM_INTSIG_NONE));
	obj2 = ivm_numeric_new(state, 110);
	obj3 = ivm_function_object_new_nc(state, IVM_NULL, ivm_function_newNative(state, IVM_GET_NATIVE_FUNC(call_func), IVM_INTSIG_NONE));
	str_pool = ivm_string_pool_new();

	exec1 = ivm_exec_new(str_pool);
	exec2 = ivm_exec_new(str_pool);
	exec3 = ivm_exec_new(str_pool);
	exec4 = ivm_exec_new(str_pool);

	proto = ivm_vmstate_getTypeProto(state, IVM_OBJECT_T);

	for (i = 0; i < sizeof(chartab) / sizeof(chartab[0]); i++) {
		ivm_object_setSlot(proto, state, chartab[i], obj3);
	}
	ivm_object_setSlot(proto, state, "proto_func", obj3);

	ivm_object_printSlots(proto);

	func1 = ivm_function_new(state, exec1, IVM_INTSIG_NONE);
	func2 = ivm_function_new(state, exec2, IVM_INTSIG_NONE);
	func3 = ivm_function_new(state, exec3, IVM_INTSIG_NONE);
	func4 = ivm_function_new(state, exec4, IVM_INTSIG_NONE);
	
	chain = ivm_ctchain_new(state, 2);
	ivm_ctchain_setAt(chain, 0, obj1);
	ivm_ctchain_setAt(chain, 1, obj2);

	printf("%f\n", IVM_AS(obj2, ivm_numeric_t)->val);

	ivm_object_setSlot(obj1, state, "a", obj2);
	ivm_object_setSlot(obj1, state, "call_func", obj3);
	ivm_object_setSlot(obj2, state, "b", obj1);
	ivm_object_setSlot(obj2, state, "c", obj1);
	ivm_object_setSlot(obj2, state, "d", obj1);

	/* add opcodes */
	ivm_exec_addOp(exec3, TEST3, "this is exec3");
	ivm_exec_addOp(exec3, NEW_FUNC, ivm_vmstate_registerFunc(state, func4));

	ivm_exec_addOp(exec1, NEW_NUM_I, 1022);

	ivm_exec_addOp(exec1, NEW_FUNC, ivm_vmstate_registerFunc(state, func3));
	ivm_exec_addOp(exec1, SET_CONTEXT_SLOT, "func");

	for (i = 0; i < 100; i++) {
		ivm_exec_addOp(exec1, GET_CONTEXT_SLOT, "func");
		ivm_exec_addOp(exec1, INVOKE, 0);
		ivm_exec_addOp(exec1, INVOKE, 0);
		ivm_exec_addOp(exec1, POP);
	}

	ivm_exec_addOp(exec1, PRINT_STR, "***************start******************");
	ivm_exec_addOp(exec1, GET_CONTEXT_SLOT, "func");
	ivm_exec_addOp(exec1, GET_CONTEXT_SLOT, "call_func");
	ivm_exec_addOp(exec1, INVOKE, 1);
	ivm_exec_addOp(exec1, PRINT_STR, "****************end*****************");

	for (i = 0; i < 100; i++) {
		ivm_exec_addOp(exec1, GET_CONTEXT_SLOT, "func");
		ivm_exec_addOp(exec1, NEW_NUM_I, 2);
		ivm_exec_addOp(exec1, GET_SLOT, "proto_func");
		ivm_exec_addOp(exec1, INVOKE, 1);
		ivm_exec_addOp(exec1, POP);
	}

	ivm_exec_addOp(exec1, NEW_OBJ);

	for (i = 0; i < 100; i++) {
		ivm_exec_addOp(exec1, NEW_OBJ);
		ivm_exec_addOp(exec1, DUP, 1);
		ivm_exec_addOp(exec1, SET_SLOT, "a");
		ivm_exec_addOp(exec1, POP);
	}

	ivm_exec_addOp(exec1, NEW_OBJ);
	ivm_exec_addOp(exec1, NEW_OBJ);
	ivm_exec_addOp(exec1, NEW_OBJ);
	ivm_exec_addOp(exec1, NEW_OBJ);

	/*for (i = 0; i < 1000; i++) {
		ivm_exec_addOp(exec1, NEW_OBJ, "");
		ivm_exec_addOp(exec1, POP, "");
	}*/

	ivm_exec_addOp(exec1, NEW_NUM_I, 1002);
	ivm_exec_addOp(exec1, PRINT_NUM);
	
	ivm_exec_addOp(exec1, SET_SLOT, "slot_a");
	ivm_exec_addOp(exec1, GET_SLOT, "slot_a");
	ivm_exec_addOp(exec1, PRINT_OBJ);
	ivm_exec_addOp(exec1, GET_SLOT, "slot_a");

	ivm_exec_addOp(exec1, GET_CONTEXT_SLOT, "c");
	ivm_exec_addOp(exec1, INVOKE, 0);
	
	ivm_exec_addOp(exec1, GET_CONTEXT_SLOT, "func");
	ivm_exec_addOp(exec1, INVOKE, 0);

	addr1 = ivm_exec_addOp(exec1, JUMP, 0);
	addr3 = ivm_exec_addOp(exec1, NEW_NULL);
	addr4 = ivm_exec_addOp(exec1, JUMP_IF_FALSE, 0);
	addr2 = ivm_exec_addOp(exec1, NOP);
	ivm_exec_setArgAt(exec1, addr1, addr2 - addr1);

	ivm_exec_addOp(exec1, JUMP, ivm_exec_cur(exec1) - addr3);
	addr2 = ivm_exec_addOp(exec1, NOP);
	ivm_exec_setArgAt(exec1, addr4, addr2 - addr4);
	
	ivm_exec_addOp(exec1, PRINT_OBJ);
	ivm_exec_addOp(exec1, NEW_OBJ);
	ivm_exec_addOp(exec1, YIELD);
	ivm_exec_addOp(exec1, NEW_NUM_I, 9990);
	ivm_exec_addOp(exec1, YIELD);
	ivm_exec_addOp(exec1, TEST1);

	ivm_exec_addOp(exec2, NEW_OBJ);
	ivm_exec_addOp(exec2, PRINT_OBJ);
	ivm_exec_addOp(exec2, TEST3, "yes, I am");
	ivm_exec_addOp(exec2, NEW_OBJ);
	ivm_exec_addOp(exec2, YIELD);
	ivm_exec_addOp(exec2, NEW_OBJ);
	ivm_exec_addOp(exec2, YIELD);
	ivm_exec_addOp(exec2, PRINT_NUM);
	ivm_exec_addOp(exec2, TEST3, "hello?");

	printf("obj1: %p\n", (void *)obj1);
	printf("obj2: %p\n", (void *)obj2);

	/* init coroutines */

	coro1 = ivm_coro_new();
	coro2 = ivm_coro_new();

	/* add coroutines to vm state */
	ivm_vmstate_addCoro(state, coro1);
	ivm_vmstate_addCoro(state, coro2);

	ivm_coro_setRoot(coro1, state,
					 IVM_AS(ivm_function_object_new_nc(state, chain, func1), ivm_function_object_t));
	ivm_coro_setRoot(coro2, state,
					 IVM_AS(ivm_function_object_new_nc(state, chain, func2), ivm_function_object_t));
	/*for (i = 0; i < 100000; i++) {
		ivm_object_new(state);
	}*/

	ivm_vmstate_unlockGCFlag(state);

#if IVM_PERF_PROFILE
	profile_start();
#endif

	/* start executing */
	fprintf(stderr, "start\n");
	ivm_vmstate_schedule(state);

#if IVM_PERF_PROFILE
	profile_output();
	profile_type();
#endif

	ivm_dbg_heapState(state, stderr);

#if 0
	fprintf(stderr, "disasm exec1:\n");
	ivm_dbg_disAsmExec(exec1, "  ", stderr);

	fprintf(stderr, "\n");

	fprintf(stderr, "disasm exec2:\n");
	ivm_dbg_disAsmExec(exec2, "  ", stderr);

	fprintf(stderr, "\n");

	fprintf(stderr, "disasm exec3:\n");
	ivm_dbg_disAsmExec(exec3, "  ", stderr);

	fprintf(stderr, "\n");

	fprintf(stderr, "disasm exec4:\n");
	ivm_dbg_disAsmExec(exec4, "  ", stderr);
#endif

	ivm_coro_free(coro1, state); ivm_coro_free(coro2, state);
	ivm_function_free(func1, state);
	ivm_function_free(func2, state);
	ivm_function_free(func3, state);
	ivm_function_free(func4, state);
	ivm_ctchain_free(chain, state);
	
	ivm_exec_free(exec1);
	ivm_exec_free(exec2);
	ivm_exec_free(exec3);
	ivm_exec_free(exec4);

	ivm_string_pool_free(str_pool);
	ivm_vmstate_free(state);

	return 0;
}

ivm_hash_val_t
strhash(const char *key)
{
	register ivm_hash_val_t hash = 5381;
	ivm_size_t len = strlen(key);
 
	for (; len >= 8; len -= 8) {
		hash = ((hash << 5) + hash) + *key++;
		hash = ((hash << 5) + hash) + *key++;
		hash = ((hash << 5) + hash) + *key++;
		hash = ((hash << 5) + hash) + *key++;
		hash = ((hash << 5) + hash) + *key++;
		hash = ((hash << 5) + hash) + *key++;
		hash = ((hash << 5) + hash) + *key++;
		hash = ((hash << 5) + hash) + *key++;
	}
 
	switch (len) {
		case 7: hash = ((hash << 5) + hash) + *key++;
		case 6: hash = ((hash << 5) + hash) + *key++;
		case 5: hash = ((hash << 5) + hash) + *key++;
		case 4: hash = ((hash << 5) + hash) + *key++;
		case 3: hash = ((hash << 5) + hash) + *key++;
		case 2: hash = ((hash << 5) + hash) + *key++;
		case 1: hash = ((hash << 5) + hash) + *key++; break;
		case 0: break;
		default: ;
	}

	return hash;
}

int dump()
{
	return 23;
}

int main()
{
	test_vm();

#if 0
	ivm_ptchain_t *chain = ivm_ptchain_new();

	profile_start();

	ivm_ptchain_addTail(chain, (void *)1);
	ivm_ptchain_addTail(chain, (void *)1);
	ivm_ptchain_addTail(chain, (void *)1);
	ivm_ptchain_removeTail(chain);
	ivm_ptchain_addTail(chain, (void *)2);

	printf("%d\n", (int)ivm_ptchain_removeTail(chain));
	printf("%ld\n", sizeof(ivm_object_t));

	profile_output();

	ivm_ptchain_free(chain);

	profile_type();

#endif

#if 0

	ivm_hash_table_t *table =
			ivm_hash_table_new(2,
							  (ivm_hash_table_comparer_t)strcmp,
							  (ivm_hash_function_t)strhash);
	char *chartab[] = { "a", "b", "c", "d",
						"e", "f", "g", "h",
						"i", "j", "k", "l",
						"m", "n", "o", "p",
						"q", "r", "s", "t",
						"u", "v", "w", "x",
						"y", "z" };

	int i;

	profile_start();

	for (i = 0; i < 10000000; i++) {
		ivm_hash_table_insert(table, chartab[i % 26], (void *)chartab[i % 26]);
	}

	for (i = 0; i < 26; i++) {
		/* printf("%s -> %s\n", chartab[i], (char *)ivm_hash_table_getValue(table, chartab[i], IVM_NULL)); */
		assert(ivm_hash_table_getValue(table, chartab[i % 26], IVM_NULL) == chartab[i % 26]);
	}

	profile_output();

	ivm_hash_table_free(table);

#endif

#if 0
	ivm_ptpool_t *pool = ivm_ptpool_new(2, sizeof(ivm_function_t));

	ivm_function_t *f1 = ivm_ptpool_alloc(pool);
	ivm_function_t *f2 = ivm_ptpool_alloc(pool);
	ivm_function_t *f3 = ivm_ptpool_alloc(pool);

	ivm_ptpool_free(pool);
#endif

	return 0;
}
