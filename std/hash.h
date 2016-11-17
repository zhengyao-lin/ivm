#ifndef _IVM_STD_HASH_H_
#define _IVM_STD_HASH_H_

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
	ivm_size_t tsize; /* table size */
	ivm_ptpair_t *table;

	ivm_hash_table_comparer_t cmp;
	ivm_hash_function_t hash;
} ivm_c_hash_table_t; /* closed(open addressing) */

ivm_c_hash_table_t *
ivm_c_hash_table_new(ivm_size_t tsize,
					 ivm_hash_table_comparer_t cmp,
					 ivm_hash_function_t hash);

void
ivm_c_hash_table_free(ivm_c_hash_table_t *table);

void
ivm_c_hash_table_insert(ivm_c_hash_table_t *table,
						void *key, void *value);

void *
ivm_c_hash_table_getValue(ivm_c_hash_table_t *table,
						  void *key, ivm_bool_t *suc);

/* hash functions */

IVM_INLINE
ivm_hash_val_t
ivm_hash_fromString_c(const ivm_char_t *key,
					  ivm_size_t len)
{
	register ivm_hash_val_t hash = 5381;
 
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

IVM_INLINE
ivm_hash_val_t
ivm_hash_fromString(const ivm_char_t *key)
{
	return ivm_hash_fromString_c(key, strlen(key));
}

IVM_INLINE
ivm_hash_val_t
ivm_hash_fromString_n(const ivm_char_t *key,
					  ivm_size_t len)
{
	return ivm_hash_fromString_c(key, len);
}

IVM_COM_END

#endif
