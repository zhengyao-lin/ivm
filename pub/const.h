#ifndef _IVM_PUB_CONST_H_
#define _IVM_PUB_CONST_H_

#define IVM_USE_PERF_PROFILE 1

/* triggers of pools */
#define IVM_USE_FUNCTION_POOL 1
#define IVM_USE_CORO_POOL 1

#define IVM_USE_INLINE_CACHE 1

/* if defined 1, a slot table will turn from list to hash table when having too many elements */
#define IVM_USE_HASH_TABLE_AS_SLOT_TABLE 1

#define IVM_DEFAULT_CONTEXT_POOL_SIZE 32

#define IVM_DEFAULT_FUNCTION_POOL_SIZE 32
#define IVM_DEFAULT_FRAME_POOL_SIZE 32
#define IVM_DEFAULT_CORO_POOL_SIZE 32

#define IVM_CONTEXT_POOL_MAX_CACHE_LEN 7

#define IVM_DEFAULT_PARSER_INIT_HEAP_SIZE (2 << 15)

/* list buffer sizes */
#define IVM_DEFAULT_PTLIST_BUFFER_SIZE 8
#define IVM_DEFAULT_LIST_BUFFER_SIZE 8
#define IVM_DEFAULT_LIST_OBJECT_BUFFER_SIZE 8

#define IVM_DEFAULT_TYPE_LIST_BUFFER_SIZE 8
#define IVM_DEFAULT_VMSTACK_BUFFER_SIZE 1
#define IVM_DEFAULT_FRAME_STACK_BUFFER_SIZE 32
#define IVM_DEFAULT_CORO_LIST_BUFFER_SIZE 8
#define IVM_DEFAULT_FUNC_LIST_BUFFER_SIZE 8
#define IVM_DEFAULT_EXEC_LIST_BUFFER_SIZE 8
#define IVM_DEFAULT_EXEC_BUFFER_SIZE 32

/* string pool block size */
#define IVM_DEFAULT_STRING_POOL_BLOCK_SIZE 2048
#define IVM_DEFAULT_STRING_POOL_BUFFER_SIZE 64

#define IVM_DEFAULT_CONST_THRESHOLD IVM_DEFAULT_STRING_POOL_BLOCK_SIZE

#define IVM_DEFAULT_SLOT_TABLE_SIZE 2
/* when the number of elements is greater than this value, the slot table will be turned into hash table */
#define IVM_DEFAULT_SLOT_TABLE_TO_HASH_THRESHOLD 8

/* GC config */
#define IVM_DEFAULT_MAX_MEM_LIMIT (2 << 30)
#define IVM_DEFAULT_INIT_HEAP_SIZE (2 << 23)
#define IVM_DEFAULT_HEAP_MAX_COMPACT_BC 2
#define IVM_DEFAULT_GC_MAX_LIVE_RATIO 15 // only when live ratio is under 15%
#define IVM_DEFAULT_GC_BC_RESTORE_RATIO 10
#define IVM_DEFAULT_GC_BC_WEIGHT 5
#define IVM_DEFAULT_GC_MAX_SKIP 20

/* only one of the following definition can be 1 */
#define IVM_DISPATCH_METHOD_DIRECT_THREAD 1

/* the number of stack element(s) cached */
#define IVM_STACK_CACHE_N_TOS 0

#define IVM_PER_INSTR_DBG(runtime) // (ivm_dbg_printRuntime(runtime))

#define IVM_COPYRIGHT_HELP "this project is released under the MIT license"
#define IVM_TAB "   "

#define IVM_IASM_FILE_SUFFIX ".ias"
#define IVM_IOBJ_FILE_SUFFIX ".iobj"

#endif
