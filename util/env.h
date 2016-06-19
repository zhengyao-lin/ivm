#ifndef _IVM_UTIL_ENV_H_
#define _IVM_UTIL_ENV_H_

#include "pub/com.h"
#include "pub/type.h"
#include "pub/vm.h"

#include "std/list.h"

IVM_COM_HEADER

typedef struct {
	const ivm_char_t *label;
	ivm_exec_t *exec;
} ivm_gen_block_t;

typedef ivm_list_t ivm_gen_block_list_t;
typedef IVM_LIST_ITER_TYPE(ivm_gen_block_t) ivm_gen_block_list_iterator_t;

#define ivm_gen_block_list_new() (ivm_list_new(sizeof(ivm_gen_block_t)))
#define ivm_gen_block_list_free ivm_list_free
#define ivm_gen_block_list_push(list, block) (ivm_list_push((list), (block)))

#define IVM_GEN_BLOCK_LIST_ITER_SET IVM_LIST_ITER_SET
#define IVM_GEN_BLOCK_LIST_ITER_GET IVM_LIST_ITER_GET
#define IVM_GEN_BLOCK_LIST_EACHPTR(list, iter) IVM_LIST_EACHPTR((list), (iter), ivm_gen_block_t)

typedef struct {
	ivm_string_pool_t *str_pool;
	ivm_gen_block_list_t *block_list;
} ivm_gen_env_t;

ivm_gen_env_t *
ivm_gen_env_new();

void
ivm_gen_env_free(ivm_gen_env_t *env);

ivm_exec_t *
ivm_gen_env_newExec(ivm_gen_env_t *env,
					const ivm_char_t *label);

int
ivm_env_init();

IVM_COM_END

#endif
