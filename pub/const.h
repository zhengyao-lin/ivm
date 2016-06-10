#ifndef _IVM_PUB_CONST_H_
#define _IVM_PUB_CONST_H_

#define IVM_PERF_PROFILE 1

/* triggers of pools */
#define IVM_USE_FUNCTION_POOL 1
#define IVM_USE_FRAME_POOL 1

/* if defined 1, a slot table will turn from list to hash table when having too many elements */
#define IVM_USE_HASH_TABLE_AS_SLOT_TABLE 1

#define IVM_DEFAULT_FUNCTION_POOL_SIZE 32
#define IVM_DEFAULT_CONTEXT_POOL_SIZE 32
#define IVM_DEFAULT_FRAME_POOL_SIZE 32

/* size of a single heap in GC */
#define IVM_DEFAULT_INIT_HEAP_SIZE ((2 << 18))

/* list buffer sizes */
#define IVM_DEFAULT_PTLIST_BUFFER_SIZE 32
#define IVM_DEFAULT_STACK_BUFFER_SIZE 32

#define IVM_DEFAULT_TYPE_LIST_BUFFER_SIZE 8
#define IVM_DEFAULT_VMSTACK_BUFFER_SIZE 64
#define IVM_DEFAULT_FRAME_STACK_BUFFER_SIZE 32
#define IVM_DEFAULT_CORO_LIST_BUFFER_SIZE 4
#define IVM_DEFAULT_FUNC_LIST_BUFFER_SIZE 64
#define IVM_DEFAULT_EXEC_LIST_BUFFER_SIZE 32
#define IVM_DEFAULT_INSTR_BLOCK_LIST_BUFFER_SIZE 32
#define IVM_DEFAULT_BINARY_OP_PROC_LIST_BUFFER_SIZE 8

#define IVM_DEFAULT_INSTR_BLOCK_BUFFER_SIZE 32

/* bytecode buffer size */
#define IVM_DEFAULT_EXEC_BUFFER_SIZE 32

/* string pool block size */
#define IVM_DEFAULT_STRING_POOL_BLOCK_SIZE 1024

#define IVM_DEFAULT_SLOT_TABLE_SIZE 3
/* when the number of elements is greater than this value, the slot table will be turned into hash table */
#define IVM_DEFAULT_SLOT_TABLE_TO_HASH_THRESHOLD 24

/* only one of the following definition can be 1 */
#define IVM_DISPATCH_METHOD_DIRECT_THREAD 1

#endif
