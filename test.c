#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "pub/const.h"
#include "pub/inlines.h"

#include "std/pool.h"
#include "std/chain.h"
#include "std/hash.h"
#include "std/heap.h"

#include "vm/vm.h"
#include "vm/dbg.h"
#include "vm/err.h"
#include "vm/env.h"
#include "vm/opcode.h"
#include "vm/gc/gc.h"

#include "std/io.h"

#include "util/parser.h"
#include "util/perf.h"

#include "app/ias/parser.h"
#include "app/ias/gen.h"

IVM_NATIVE_FUNC(test)
{
	IVM_OUT("from native!!\n");
	return IVM_NULL;
}

IVM_NATIVE_FUNC(call_func)
{
	ivm_function_object_t *func = IVM_AS(arg.argv[0], ivm_function_object_t);
	IVM_OUT("call function!!\n");

	// ivm_dbg_stackState(IVM_VMSTATE_GET(state, CUR_CORO), stderr);

	ivm_function_invoke(func->val, state,
						func->closure, IVM_VMSTATE_GET(state, CUR_CORO));
	ivm_coro_resume(IVM_VMSTATE_GET(state, CUR_CORO), state, IVM_NULL);

	return ivm_numeric_new(state, 10016);
}

#define print_type(type) (IVM_TRACE(#type ": %zd\n", sizeof(type)))

void profile_type()
{
	IVM_TRACE("***size of types***\n\n");
	print_type(ivm_object_t);
	print_type(ivm_function_t);
	print_type(ivm_function_object_t);
	print_type(ivm_type_t);
	print_type(ivm_vmstate_t);
	print_type(ivm_exec_t);
	print_type(ivm_ctchain_t);
	print_type(ivm_frame_t);
	IVM_TRACE("\n***size of types***\n");

	return;
}

int test_fib()
{
	ivm_vmstate_t *state;
	ivm_function_t *fib, *top;
	ivm_exec_t *exec1, *exec2;
	ivm_string_pool_t *str_pool;
	ivm_coro_t *coro;
	ivm_ctchain_t *chain;
	ivm_size_t addr1, addr2;

	str_pool = ivm_string_pool_new(IVM_TRUE);
	state = ivm_vmstate_new(); ivm_vmstate_lockGCFlag(state);

	exec1 = ivm_exec_new(str_pool); ivm_ref_inc(exec1);
	exec2 = ivm_exec_new(str_pool); ivm_ref_inc(exec2);

	top = ivm_function_new(state, IVM_NULL);
	fib = ivm_function_new(state, IVM_NULL);
	ivm_vmstate_registerFunc(state, top);
	ivm_vmstate_registerFunc(state, fib);
	
	coro = ivm_coro_new(state);
	chain = ivm_ctchain_new(state, 1);
	ivm_ctchain_setAt(chain, 0, ivm_object_new(state));
	ivm_ctchain_addRef(chain);

	ivm_vmstate_addCoro_c(state, coro);

#if 1
	/********** top **********/
	ivm_exec_addInstr(exec1, NEW_FUNC, ivm_vmstate_registerFunc(state, fib));
	ivm_exec_addInstr(exec1, SET_CONTEXT_SLOT, "fib");

	ivm_exec_addInstr(exec1, NEW_NUM_I, 30);
	ivm_exec_addInstr(exec1, GET_CONTEXT_SLOT, "fib");
	ivm_exec_addInstr(exec1, INVOKE, 1);
	ivm_exec_addInstr(exec1, OUT_NUM);
	/********** top **********/

	/********** fib **********/
	ivm_exec_addInstr(exec2, SET_ARG, "n");

	ivm_exec_addInstr(exec2, GET_CONTEXT_SLOT, "n");
	ivm_exec_addInstr(exec2, NEW_NUM_I, 2);

	ivm_exec_addInstr(exec2, LT);
	
	addr1 = ivm_exec_addInstr(exec2, JUMP_FALSE, 0); /* if (n < 2) */

		ivm_exec_addInstr(exec2, NEW_NUM_I, 1);
		ivm_exec_addInstr(exec2, RETURN);
	
	addr2 = ivm_exec_addInstr(exec2, GET_CONTEXT_SLOT, "n");
	ivm_exec_setArgAt(exec2, addr1, addr2 - addr1);

	ivm_exec_addInstr(exec2, NEW_NUM_I, 1);
	ivm_exec_addInstr(exec2, SUB);
	ivm_exec_addInstr(exec2, GET_CONTEXT_SLOT, "fib");
	ivm_exec_addInstr(exec2, INVOKE, 1);

	// ivm_exec_addInstr(exec2, PRINT_NUM);

	ivm_exec_addInstr(exec2, GET_CONTEXT_SLOT, "n");
	ivm_exec_addInstr(exec2, NEW_NUM_I, 2);
	ivm_exec_addInstr(exec2, SUB);
	ivm_exec_addInstr(exec2, GET_CONTEXT_SLOT, "fib");
	ivm_exec_addInstr(exec2, INVOKE, 1);

	// ivm_exec_addInstr(exec2, PRINT_NUM);

	ivm_exec_addInstr(exec2, ADD);
	ivm_exec_addInstr(exec2, RETURN);
	/********** fib **********/

	ivm_function_setExec(top, state, exec1);
	ivm_function_setExec(fib, state, exec2);

#endif

	ivm_coro_setRoot(coro, state,
					 IVM_AS(ivm_function_object_new(state, IVM_NULL, top),
					 		ivm_function_object_t));

	ivm_vmstate_unlockGCFlag(state);

	ivm_perf_startProfile();

	/* start executing */
	IVM_TRACE("start\n");
	ivm_vmstate_schedule(state);

	ivm_perf_stopProfile();

	// ivm_dbg_heapState(state, stderr);

	ivm_ctchain_free(chain, state);
	ivm_vmstate_free(state);
	ivm_exec_free(exec1);
	ivm_exec_free(exec2);

	return 0;
}

int test_call()
{
	int i;
	ivm_vmstate_t *state;
	ivm_function_t *top, *empty;
	ivm_exec_t *exec1;
	ivm_string_pool_t *str_pool;
	ivm_coro_t *coro;
	ivm_ctchain_t *chain;
	ivm_size_t addr1, addr2, addr3;

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
						"zzz", "aaaa", "bbbb", "cccc",
						"dddd", "eeee", "ffff", "gggg",
						"hhhh", "iiii", "jjjj", "kkkk",
						"llll", "mmmm", "nnnn", "oooo",
						"pppp", "qqqq", "rrrr", "ssss",
						"tttt", "uuuu", "vvvv", "wwww",
						"xxxx", "yyyy", "zzzz", "aaaaa",
						"bbbbb", "ccccc", "ddddd", "eeeee",
						"fffff", "ggggg", "hhhhh", "iiiii",
						"jjjjj", "kkkkk", "lllll", "mmmmm",
						"nnnnn", "ooooo", "ppppp", "qqqqq",
						"rrrrr", "sssss", "ttttt", "uuuuu",
						"vvvvv", "wwwww", "xxxxx", "yyyyy",
						"zzzzz" };

	str_pool = ivm_string_pool_new(IVM_TRUE);
	state = ivm_vmstate_new(); ivm_vmstate_lockGCFlag(state);

	exec1 = ivm_exec_new(str_pool); ivm_ref_inc(exec1);

	top = ivm_function_new(state, IVM_NULL);
	empty = ivm_function_new(state, IVM_NULL);
	ivm_vmstate_registerFunc(state, top);
	ivm_vmstate_registerFunc(state, empty);
	
	coro = ivm_coro_new(state);
	chain = ivm_ctchain_new(state, 1);
	ivm_ctchain_setAt(chain, 0, ivm_object_new(state));
	ivm_ctchain_addRef(chain);

	ivm_vmstate_addCoro_c(state, coro);

	/********************** code ***********************/
/*
	for (i = 0; i < sizeof(chartab) / sizeof(chartab[0]); i++) {
		ivm_exec_addInstr(exec1, NEW_NUM_I, 0);
		ivm_exec_addInstr(exec1, SET_CONTEXT_SLOT, chartab[i]);
	}
*/
	ivm_exec_addInstr(exec1, NEW_FUNC, ivm_vmstate_registerFunc(state, empty));
	ivm_exec_addInstr(exec1, SET_CONTEXT_SLOT, "do_nothing");

	ivm_exec_addInstr(exec1, NEW_NUM_I, 0);
	ivm_exec_addInstr(exec1, SET_CONTEXT_SLOT, "i");

	addr1 = ivm_exec_addInstr(exec1, GET_CONTEXT_SLOT, "i");
	ivm_exec_addInstr(exec1, NEW_NUM_I, 1000000);
	ivm_exec_addInstr(exec1, LT);

	addr2 = ivm_exec_addInstr(exec1, JUMP_FALSE, 0);

		ivm_exec_addInstr(exec1, GET_CONTEXT_SLOT, "do_nothing");
		ivm_exec_addInstr(exec1, INVOKE, 0);
		ivm_exec_addInstr(exec1, POP);

		ivm_exec_addInstr(exec1, GET_CONTEXT_SLOT, "i");
		ivm_exec_addInstr(exec1, NEW_NUM_I, 1);
		ivm_exec_addInstr(exec1, ADD);
		ivm_exec_addInstr(exec1, SET_CONTEXT_SLOT, "i");
		ivm_exec_addInstr(exec1, JUMP, addr1 - ivm_exec_cur(exec1));
	addr3 = ivm_exec_addInstr(exec1, NOP);
	ivm_exec_setArgAt(exec1, addr2, addr3 - addr2);

	/********************** code ***********************/

	ivm_function_setExec(top, state, exec1);

	ivm_coro_setRoot(coro, state,
					 IVM_AS(ivm_function_object_new(state, IVM_NULL, top),
					 ivm_function_object_t));

	ivm_vmstate_unlockGCFlag(state);

	ivm_perf_startProfile();

	/* start executing */
	IVM_TRACE("start\n");
	ivm_vmstate_schedule(state);

	ivm_perf_stopProfile();

	// ivm_dbg_heapState(state, stderr);
	// IVM_TRACE("\nstack state:\n");
	// ivm_dbg_stackState(coro, stderr);

	ivm_ctchain_free(chain, state);
	ivm_vmstate_free(state);
	ivm_exec_free(exec1);

	return 0;
}

#define STR(str, pool) (ivm_string_pool_store((pool), (str)))

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
	obj1 = ivm_function_object_new(state, IVM_NULL, ivm_function_newNative(state, IVM_GET_NATIVE_FUNC(test)));
	obj2 = ivm_numeric_new(state, 110);
	obj3 = ivm_function_object_new(state, IVM_NULL, ivm_function_newNative(state, IVM_GET_NATIVE_FUNC(call_func)));
	str_pool = ivm_string_pool_new(IVM_TRUE);

	exec1 = ivm_exec_new(str_pool); ivm_ref_inc(exec1);
	exec2 = ivm_exec_new(str_pool); ivm_ref_inc(exec2);
	exec3 = ivm_exec_new(str_pool); ivm_ref_inc(exec3);
	exec4 = ivm_exec_new(str_pool); ivm_ref_inc(exec4);

	proto = ivm_vmstate_getTypeProto(state, IVM_OBJECT_T);

	for (i = 0; i < sizeof(chartab) / sizeof(chartab[0]); i++) {
		ivm_object_setSlot(proto, state, STR(chartab[i], str_pool), obj3);
	}
	ivm_object_setSlot(proto, state, STR("proto_func", str_pool), obj3);

	ivm_object_printSlots(proto);

	func1 = ivm_function_new(state, IVM_NULL);
	func2 = ivm_function_new(state, IVM_NULL);
	func3 = ivm_function_new(state, IVM_NULL);
	func4 = ivm_function_new(state, IVM_NULL);
	ivm_vmstate_registerFunc(state, func1);
	ivm_vmstate_registerFunc(state, func2);
	ivm_vmstate_registerFunc(state, func3);
	ivm_vmstate_registerFunc(state, func4);
	
	chain = ivm_ctchain_new(state, 2);
	ivm_ctchain_setAt(chain, 0, obj1);
	ivm_ctchain_setAt(chain, 1, obj2);
	ivm_ctchain_addRef(chain);

	IVM_TRACE("%f\n", IVM_AS(obj2, ivm_numeric_t)->val);

	ivm_object_setSlot(obj1, state, STR("a", str_pool), obj2);
	ivm_object_setSlot(obj1, state, STR("call_func", str_pool), obj3);
	ivm_object_setSlot(obj2, state, STR("b", str_pool), obj1);
	ivm_object_setSlot(obj2, state, STR("c", str_pool), obj1);
	ivm_object_setSlot(obj2, state, STR("d", str_pool), obj1);

	/* add opcodes */
	ivm_exec_addInstr(exec3, NOP);
	ivm_exec_addInstr(exec3, NEW_FUNC, ivm_vmstate_registerFunc(state, func4));
	/* ivm_exec_addInstr(exec3, GET_CONTEXT_SLOT, "func2"); */

	for (i = 0; i < 1; i++) {
		ivm_exec_addInstr(exec1, NEW_STR, "hello, ");
		ivm_exec_addInstr(exec1, NEW_STR, "world");
		ivm_exec_addInstr(exec1, ADD);
		ivm_exec_addInstr(exec1, POP);
	}

	ivm_exec_addInstr(exec1, NEW_NUM_I, 0);
	ivm_exec_addInstr(exec1, NOT);
	ivm_exec_addInstr(exec1, PRINT_NUM);

	ivm_exec_addInstr(exec1, NEW_NUM_I, 1022);
	ivm_exec_addInstr(exec1, NEW_NUM_I, 1022);

	ivm_exec_addInstr(exec1, ADD);
	ivm_exec_addInstr(exec1, PRINT_NUM);

	ivm_exec_addInstr(exec1, NEW_NUM_I, 1023);
	ivm_exec_addInstr(exec1, NEW_NUM_I, 1022);
	addr1 = ivm_exec_addInstr(exec1, JUMP_LT, 0);
	ivm_exec_addInstr(exec1, OUT, "op1 is greater than or equal op2");
	addr2 = ivm_exec_addInstr(exec1, NOP);
	ivm_exec_setArgAt(exec1, addr1, addr2 - addr1);

	// ivm_exec_addInstr(exec1, NEW_NUM_I, 1022);
	//

	ivm_exec_addInstr(exec1, NEW_FUNC, ivm_vmstate_registerFunc(state, func3));
	ivm_exec_addInstr(exec1, SET_CONTEXT_SLOT, "func");

	ivm_exec_addInstr(exec1, NEW_FUNC, ivm_vmstate_registerFunc(state, func4));
	ivm_exec_addInstr(exec1, SET_CONTEXT_SLOT, "func2");

	ivm_exec_addInstr(exec1, NEW_FUNC, ivm_vmstate_registerFunc(state, func4));
	ivm_exec_addInstr(exec1, SET_CONTEXT_SLOT, "do_nothing");

	ivm_exec_addInstr(exec1, NEW_NUM_I, 0);
	ivm_exec_addInstr(exec1, SET_CONTEXT_SLOT, "i");

	ivm_exec_addInstr(exec1, NEW_NUM_I, 0);
	ivm_exec_addInstr(exec1, SET_CONTEXT_SLOT, "i");

	/* while i < n */
	addr1 = ivm_exec_addInstr(exec1, NEW_NUM_I, 1000000);
	ivm_exec_addInstr(exec1, GET_CONTEXT_SLOT, "i");
	addr2 = ivm_exec_addInstr(exec1, JUMP_LT, 0);
		ivm_exec_addInstr(exec1, NEW_STR, "hello, ");
		ivm_exec_addInstr(exec1, NEW_STR, "world");
		ivm_exec_addInstr(exec1, ADD);
		ivm_exec_addInstr(exec1, SET_CONTEXT_SLOT, "s");
		// ivm_exec_addInstr(exec1, POP);

	#if 1
		/* call test */

		ivm_exec_addInstr(exec1, GET_CONTEXT_SLOT, "do_nothing");
		ivm_exec_addInstr(exec1, INVOKE, 0);
		ivm_exec_addInstr(exec1, POP);
		
		ivm_exec_addInstr(exec1, GET_CONTEXT_SLOT, "func");
		ivm_exec_addInstr(exec1, NEW_NUM_I, 2);
		ivm_exec_addInstr(exec1, GET_SLOT, "proto_func");
		ivm_exec_addInstr(exec1, INVOKE, 1);
		ivm_exec_addInstr(exec1, POP);

		ivm_exec_addInstr(exec1, GET_CONTEXT_SLOT, "func");
		ivm_exec_addInstr(exec1, INVOKE, 0);
		ivm_exec_addInstr(exec1, INVOKE, 0);
		ivm_exec_addInstr(exec1, POP);

		/* slot test */
		ivm_exec_addInstr(exec1, NEW_OBJ);
		ivm_exec_addInstr(exec1, NEW_NUM_I, 1022);
		ivm_exec_addInstr(exec1, SET_SLOT, "a");
		ivm_exec_addInstr(exec1, POP);

		/* string test */
		ivm_exec_addInstr(exec1, NEW_STR, "hey!");
		ivm_exec_addInstr(exec1, PRINT_STR);
	#endif

		ivm_exec_addInstr(exec1, GET_CONTEXT_SLOT, "i");
		ivm_exec_addInstr(exec1, NEW_NUM_I, 1);
		ivm_exec_addInstr(exec1, ADD);
		ivm_exec_addInstr(exec1, SET_CONTEXT_SLOT, "i");
		ivm_exec_addInstr(exec1, JUMP, addr1 - ivm_exec_cur(exec1));
	addr3 = ivm_exec_addInstr(exec1, NOP);
	ivm_exec_setArgAt(exec1, addr2, addr3 - addr2);
	/* end while */

	ivm_exec_addInstr(exec1, OUT, "***************start******************");
	ivm_exec_addInstr(exec1, GET_CONTEXT_SLOT, "func");
	ivm_exec_addInstr(exec1, GET_CONTEXT_SLOT, "call_func");
	ivm_exec_addInstr(exec1, INVOKE, 1);
	ivm_exec_addInstr(exec1, POP);
	ivm_exec_addInstr(exec1, OUT, "****************end*****************");

	ivm_exec_addInstr(exec1, NEW_OBJ);

	ivm_exec_addInstr(exec1, NEW_OBJ);
	ivm_exec_addInstr(exec1, NEW_OBJ);
	ivm_exec_addInstr(exec1, NEW_OBJ);
	ivm_exec_addInstr(exec1, NEW_OBJ);

	/*for (i = 0; i < 1000; i++) {
		ivm_exec_addInstr(exec1, NEW_OBJ, "");
		ivm_exec_addInstr(exec1, POP, "");
	}*/

	ivm_exec_addInstr(exec1, NEW_NUM_I, 1002);
	ivm_exec_addInstr(exec1, PRINT_NUM);
	
	ivm_exec_addInstr(exec1, SET_SLOT, "slot_a");
	ivm_exec_addInstr(exec1, GET_SLOT, "slot_a");
	ivm_exec_addInstr(exec1, PRINT_OBJ);
	ivm_exec_addInstr(exec1, GET_SLOT, "slot_a");

	ivm_exec_addInstr(exec1, GET_CONTEXT_SLOT, "c");
	ivm_exec_addInstr(exec1, INVOKE, 0);
	
	ivm_exec_addInstr(exec1, GET_CONTEXT_SLOT, "func");
	ivm_exec_addInstr(exec1, INVOKE, 0);

	addr1 = ivm_exec_addInstr(exec1, JUMP, 0);
	addr3 = ivm_exec_addInstr(exec1, NEW_NULL);
	addr4 = ivm_exec_addInstr(exec1, JUMP_FALSE, 0);
	addr2 = ivm_exec_addInstr(exec1, NOP);
	ivm_exec_setArgAt(exec1, addr1, addr2 - addr1);

	ivm_exec_addInstr(exec1, JUMP, addr3 - ivm_exec_cur(exec1));
	addr2 = ivm_exec_addInstr(exec1, NOP);
	ivm_exec_setArgAt(exec1, addr4, addr2 - addr4);

	ivm_exec_addInstr(exec1, PRINT_OBJ);
	ivm_exec_addInstr(exec1, NEW_OBJ);
	ivm_exec_addInstr(exec1, YIELD);
	ivm_exec_addInstr(exec1, NEW_NUM_I, 9990);
	ivm_exec_addInstr(exec1, YIELD);

	ivm_exec_addInstr(exec2, NEW_OBJ);
	ivm_exec_addInstr(exec2, PRINT_OBJ);
	ivm_exec_addInstr(exec2, OUT, "yes, I am");
	ivm_exec_addInstr(exec2, NEW_OBJ);
	ivm_exec_addInstr(exec2, YIELD);
	ivm_exec_addInstr(exec2, NEW_OBJ);
	ivm_exec_addInstr(exec2, YIELD);
	ivm_exec_addInstr(exec2, PRINT_NUM);
	ivm_exec_addInstr(exec2, NOP);

	IVM_TRACE("obj1: %p\n", (void *)obj1);
	IVM_TRACE("obj2: %p\n", (void *)obj2);

	ivm_function_setExec(func1, state, exec1);
	ivm_function_setExec(func2, state, exec2);
	ivm_function_setExec(func3, state, exec3);
	ivm_function_setExec(func4, state, exec4);

	/* init coroutines */

	coro1 = ivm_coro_new(state);
	coro2 = ivm_coro_new(state);

	/* add coroutines to vm state */
	ivm_vmstate_addCoro_c(state, coro1);
	ivm_vmstate_addCoro_c(state, coro2);

	ivm_coro_setRoot(coro1, state,
					 IVM_AS(ivm_function_object_new(state, chain, func1),
					 		ivm_function_object_t));
	ivm_coro_setRoot(coro2, state,
					 IVM_AS(ivm_function_object_new(state, chain, func2),
					 		ivm_function_object_t));
	/*for (i = 0; i < 100000; i++) {
		ivm_object_new(state);
	}*/

	ivm_vmstate_unlockGCFlag(state);

	ivm_perf_startProfile();

	/* start executing */
	IVM_TRACE("start\n");
	ivm_vmstate_schedule(state);

	ivm_perf_stopProfile();

	// ivm_dbg_heapState(state, stderr);

#if 0
	IVM_TRACE("disasm exec1:\n");
	ivm_dbg_printExec(exec1, "  ", stderr);

	IVM_TRACE("\n");

	IVM_TRACE("disasm exec2:\n");
	ivm_dbg_printExec(exec2, "  ", stderr);

	IVM_TRACE("\n");

	IVM_TRACE("disasm exec3:\n");
	ivm_dbg_printExec(exec3, "  ", stderr);

	IVM_TRACE("\n");

	IVM_TRACE("disasm exec4:\n");
	ivm_dbg_printExec(exec4, "  ", stderr);
#endif

	ivm_ctchain_free(chain, state);
	ivm_exec_free(exec1); ivm_exec_free(exec2);
	ivm_exec_free(exec3); ivm_exec_free(exec4);
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

int main(int argc, const char **argv)
{
	ivm_env_init();

#if 1

	test_call();
	test_vm();
	test_fib();

	ivm_perf_printElapsed();
	// profile_type();

#endif

#if 1
	const char num[] = "0b1010101";
	// const char str[] = "\\\"sdssd\\p\\n";
	ivm_bool_t err = IVM_FALSE;
	ias_gen_env_t *env;
	ivm_vmstate_t *state;
	ivm_file_t *file;
	ivm_char_t *src = "root { \
		new_func fib; \
		set_context_slot \"fib\"; \
		new_num_i 30; \
		get_context_slot \"fib\"; \
		invoke 1; \
		out_num; \
	} fib { \
		set_arg \"n\"; \
		get_context_slot \"n\"; \
		new_num_i 2; \
		lt; \
		jump_false else; \
			new_num_i 1; \
			return; \
		else:\n \
			get_context_slot \"n\"; \
			new_num_i 1; \
			sub; \
			get_context_slot \"fib\"; \
			invoke 1; \
			get_context_slot \"n\"; \
			new_num_i 2; \
			sub; \
			get_context_slot \"fib\"; \
			invoke 1; \
			add; \
			return; \
	}";

	if (argc == 2) {
		file = ivm_file_new(argv[1], "rb");
		src = ivm_file_readAll(file);
		ivm_file_free(file);
	}

	// ivm_list_free(_ivm_parser_getTokens("h\"\\\"i wow\\\" \",,\ns2as2.3\"ss\"2hey 2.3 \"hey\" yeah 2.3"));

	IVM_TRACE("****************************************\n");

	IVM_TRACE("%f %d\n",
			  ivm_parser_parseNum(num, sizeof(num) - 1, IVM_NULL, &err), err);

	IVM_TRACE("****************************************\n");

ivm_perf_reset();
ivm_perf_startProfile();

	// tokens = _ivm_parser_getTokens("hi{}\n");
	// tokens = _ivm_parser_getTokens("\n\nhi { get_slot \"hi\"\n\npop;;pop;;pop } wow { } hey { ;a: s;pop 2.|||; } wowow{}");
	env = ias_parser_parseSource(src);
	state = ias_gen_env_generateVM(env);
	ias_gen_env_free(env);

ivm_perf_stopProfile();
ivm_perf_printElapsed();

ivm_perf_reset();
ivm_perf_startProfile();

	ivm_vmstate_schedule(state);

ivm_perf_stopProfile();
ivm_perf_printElapsed();

	ivm_vmstate_free(state);

	if (argc == 2) {
		MEM_FREE(src);
	}

	/*

	ivm_string_pool_free(exec->pool);
	ivm_exec_free(exec);*/
#endif

#if 0
	ivm_string_pool_t *pool = ivm_string_pool_new(IVM_FALSE);
	ivm_string_t *ret;
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

	for (int i = 0; i < sizeof(chartab) / sizeof(char *); i++) {
		ivm_string_pool_registerRaw(pool, chartab[i]);
	}


	for (int i = 0; i < 1000000; i++) {
		ret = (ivm_string_t *)ivm_string_pool_registerRaw(pool, "r");
	}

	printf("%s\n", ivm_string_trimHead(ret));

	ivm_string_pool_free(pool);

#endif

#if 0
	ivm_ptchain_t *chain = ivm_ptchain_new();

	profile_start();

	ivm_ptchain_addTail(chain, (void *)1);
	ivm_ptchain_addTail(chain, (void *)1);
	ivm_ptchain_addTail(chain, (void *)1);
	ivm_ptchain_removeTail(chain);
	ivm_ptchain_addTail(chain, (void *)2);

	IVM_TRACE("%d\n", (int)ivm_ptchain_removeTail(chain));
	IVM_TRACE("%zd\n", sizeof(ivm_object_t));

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
		/* IVM_TRACE("%s -> %s\n", chartab[i], (char *)ivm_hash_table_getValue(table, chartab[i], IVM_NULL)); */
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
