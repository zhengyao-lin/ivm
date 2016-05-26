#ifndef _IVM_VM_STD_HASH_H_
#define _IVM_VM_STD_HASH_H_

#include "pub/type.h"
#include "list.h"

IVM_COM_HEADER

typedef ivm_uint64_t ivm_hash_val_t;

typedef ivm_int_t (*ivm_hash_table_comparer_t)(void *a, void *b);
typedef ivm_hash_val_t (*ivm_hash_function_t)(void *ptr);

typedef struct {
	void *k;
	void *v;
} ivm_ptpair_t;

typedef struct {
	ivm_uint_t bsize; /* base size */
	ivm_ptpair_t *base; /* pairs with NULL keys are considered empty */

	ivm_size_t osize;
	ivm_ptpair_t *otable;

	ivm_hash_table_comparer_t cmp;
	ivm_hash_function_t hash;
} ivm_hash_table_t;

ivm_hash_table_t *
ivm_hash_table_new(ivm_uint_t bsize,
				   ivm_hash_table_comparer_t cmp,
				   ivm_hash_function_t hash);

void
ivm_hash_table_free(ivm_hash_table_t *table);

void
ivm_hash_table_setMap(ivm_hash_table_t *table,
					  void *key, void *value);

void *
ivm_hash_table_getValue(ivm_hash_table_t *table,
						void *key, ivm_bool_t *suc);

IVM_COM_END

#endif
