#include "pub/com.h"
#include "pub/mem.h"
#include "hash.h"
#include "err.h"

ivm_hash_table_t *
ivm_hash_table_new(ivm_uint_t bsize,
				   ivm_hash_table_comparer_t cmp,
				   ivm_hash_function_t hash)
{
	ivm_hash_table_t *ret = MEM_ALLOC(sizeof(*ret),
									  ivm_hash_table_t *);

	IVM_ASSERT(ret, IVM_ERROR_MSG_FAILED_ALLOC_NEW("hash table"));

	ret->bsize = bsize;
	ret->base = MEM_ALLOC_INIT(sizeof(*ret->base) * bsize,
							   ivm_ptpair_t *);

	IVM_ASSERT(ret->base, IVM_ERROR_MSG_FAILED_ALLOC_NEW("hash table base"));

	ret->osize = 0;
	ret->otable = IVM_NULL;

	ret->cmp = cmp;
	ret->hash = hash;

	IVM_ASSERT(ret->cmp && ret->hash,
			   IVM_ERROT_MSG_NULL_PTR("hash comparer or hash function"));

	return ret;
}

void
ivm_hash_table_free(ivm_hash_table_t *table)
{
	if (table) {
		MEM_FREE(table->base);
		MEM_FREE(table->otable);
		MEM_FREE(table);
	}

	return;
}

#define IS_EMPTY_SLOT(pair) (!(pair)->k)

IVM_PRIVATE
ivm_ptpair_t * /* NULL value means new space were allocated */
ivm_hash_table_getOverflow(ivm_hash_table_t *table,
						   ivm_size_t i)
{
	ivm_size_t orig;

	if (i >= table->osize) {
		orig = table->osize;
		table->osize = i + 1;
		table->otable = MEM_REALLOC(table->otable,
									sizeof(*table->otable) * table->osize,
									ivm_ptpair_t *);
		MEM_INIT(&table->otable[orig],
				 sizeof(*table->otable)
				 * (table->osize - orig));
	}

	return &table->otable[i];
}

void
ivm_hash_table_setMap(ivm_hash_table_t *table,
					  void *key, void *value)
{
	ivm_hash_val_t hash = table->hash(key);
	ivm_uint_t i = hash % table->bsize;
	ivm_ptpair_t *tmp = &table->base[i];
	ivm_size_t j;

	IVM_ASSERT(key, IVM_ERROR_MSG_FAILED_ALLOC_NEW("key"));

	if (IS_EMPTY_SLOT(tmp)) {
		tmp->k = key;
		tmp->v = value;
	} else if (table->cmp(key, tmp->k) == 0) {
		tmp->v = value;
	} else {
		/* overflow */
		for (j = i; ; j += (table->bsize >> 2) + 1) {
			tmp = ivm_hash_table_getOverflow(table, j);
			if (IS_EMPTY_SLOT(tmp)) {
				tmp->k = key;
				tmp->v = value;
				break;
			} else if (table->cmp(key, tmp->k) == 0) {
				tmp->v = value;
				break;
			}
		}
	}

	return;
}

#define SET_SUCCESS(flag) ((flag) ? *(flag) = IVM_TRUE : IVM_TRUE)
#define SET_FAILED(flag) ((flag) ? *(flag) = IVM_FALSE : IVM_FALSE)

void *
ivm_hash_table_getValue(ivm_hash_table_t *table,
						void *key, ivm_bool_t *suc)
{
	ivm_hash_val_t hash = table->hash(key);
	ivm_uint_t i = hash % table->bsize;
	ivm_ptpair_t *tmp = &table->base[i];
	void *ret = IVM_NULL;
	ivm_size_t j;

	IVM_ASSERT(key, IVM_ERROR_MSG_FAILED_ALLOC_NEW("key"));

	if (IS_EMPTY_SLOT(tmp)) {
		SET_FAILED(suc);
	} else if (table->cmp(key, tmp->k) == 0) {
		ret = tmp->v;
		SET_SUCCESS(suc);
	} else {
		/* check overflow */
		for (j = i; ; j += (table->bsize >> 2) + 1) {
			tmp = ivm_hash_table_getOverflow(table, j);
			if (IS_EMPTY_SLOT(tmp)) {
				SET_FAILED(suc);
				break;
			} else if (table->cmp(key, tmp->k) == 0) {
				ret = tmp->v;
				SET_SUCCESS(suc);
				break;
			}
		}
	}

	return ret;
}
