#ifndef _IVM_STD_HASH_H_
#define _IVM_STD_HASH_H_

#include "pub/type.h"

#include "list.h"

IVM_COM_HEADER

typedef ivm_uint64_t ivm_hash_val_t;

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
ivm_hash_fromPointer(void *ptr)
{
	register ivm_uptr_t key = (ivm_uptr_t)ptr >> 3;

	key = (~key) + (key << 21); // key = (key << 21) - key - 1;
	key = key ^ (key >> 24);
	key = (key + (key << 3)) + (key << 8); // key * 265
	key = key ^ (key >> 14);
	key = (key + (key << 2)) + (key << 4); // key * 21
	key = key ^ (key >> 28);
	key = key + (key << 31);

	return key;
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

typedef struct {
	void *k;
	void *v;
} ivm_pthash_pair_t;

typedef struct {
	ivm_pthash_pair_t *table;
	ivm_size_t size;
	ivm_size_t ecount;
} ivm_pthash_t;

void
ivm_pthash_init(ivm_pthash_t *table);

void
ivm_pthash_dump(ivm_pthash_t *table);

IVM_INLINE
void
_ivm_pthash_expand(ivm_pthash_t *table);

#define ivm_pthash_count(table) ((table)->ecount)

IVM_INLINE
ivm_pthash_pair_t *
_ivm_pthash_find_c(ivm_pthash_t *table,
				   void *key)
{
	ivm_hash_val_t hash;
	register ivm_pthash_pair_t *end, *tmp, *i;
	hash = ivm_hash_fromPointer(key);

	if (!key) return IVM_NULL;

	end = table->table + table->size;
	tmp = table->table + (hash & (table->size - 1));
	
	for (i = tmp; i != end; i++) {
		if (!i->k || i->k == key) {
			return i;
		}
	}
	
	for (i = table->table; i != tmp; i++) {
		if (!i->k || i->k == key) {
			return i;
		}
	}

	return IVM_NULL;
}

IVM_INLINE
void *
ivm_pthash_find(ivm_pthash_t *table,
				void *key)
{
	ivm_pthash_pair_t *res = _ivm_pthash_find_c(table, key);

	if (res && res->k) return res->v;

	return IVM_NULL;
}

IVM_INLINE
void
ivm_pthash_insert(ivm_pthash_t *table,
				  void *key, void *val)
{
	ivm_pthash_pair_t *found;

	if (!key) return;

	while (1) {
		found = _ivm_pthash_find_c(table, key);

		if (found) {
			if (!found->k) {
				found->k = key;
			}

			found->v = val;
			table->ecount++;

			return;
		} else {
			_ivm_pthash_expand(table);
		}
	}

	return;
}

IVM_INLINE
ivm_bool_t /* suc or not */
ivm_pthash_insertEmpty(ivm_pthash_t *table,
					   void *key, void *val)
{
	ivm_pthash_pair_t *found;

	if (!key) return IVM_FALSE;

	while (1) {
		found = _ivm_pthash_find_c(table, key);

		if (found) {
			if (!found->k) {
				found->k = key;
				found->v = val;
				table->ecount++;
				return IVM_TRUE;
			} else {
				// slot exists
				return IVM_FALSE;
			}
		} else {
			_ivm_pthash_expand(table);
		}
	}

	return IVM_FALSE;
}

IVM_INLINE
void
_ivm_pthash_rehash(ivm_pthash_t *table,
				   void *key,
				   void *val)
{
	ivm_pthash_pair_t *found;

	found = _ivm_pthash_find_c(table, key);

	if (!found) {
		IVM_FATAL("impossible");
	}

	found->k = key;
	found->v = val;

	return;
}

IVM_INLINE
ivm_bool_t /* found the ptr? */
ivm_pthash_remove(ivm_pthash_t *table,
				  void *key)
{
	ivm_pthash_pair_t *gap = _ivm_pthash_find_c(table, key);
	ivm_pthash_pair_t *tab, *end, *i;
	ivm_size_t mask;
	ivm_hash_val_t hash;

	if (gap && gap->k) {
		gap->k = IVM_NULL;

		tab = table->table;
		end = tab + table->size;
		mask = table->size - 1;

		while (1) {
			// IVM_TRACE("always\n");
			for (i = gap + 1; i != end; i++) {
				if (!i->k) goto SUC; // next gap: no influence
				
				hash = ivm_hash_fromPointer(i->k) & mask;
				
				if (tab + hash <= gap ||
					tab + hash > i) {
					goto FILL; // found a filling element
				}
			}

			for (i = tab; i != gap; i++) {
				if (!i->k) goto SUC; // next gap: no influence
				
				hash = ivm_hash_fromPointer(i->k) & mask;
				
				if (tab + hash > i &&
					tab + hash <= gap) {
					goto FILL; // found a filling element
				}
			}

			break;

		FILL:
			*gap = *i;
			gap = i;
			gap->k = IVM_NULL;
		}

		SUC:
			table->ecount--;
			return IVM_TRUE;

	} else return IVM_FALSE;
}

IVM_INLINE
void
_ivm_pthash_expand(ivm_pthash_t *table)
{
	ivm_size_t orig_size = table->size;
	ivm_pthash_pair_t *orig = table->table, *i, *end;

	table->size <<= 1;
	table->table = STD_ALLOC_INIT(sizeof(*table->table) * table->size);

	IVM_MEMCHECK(table->table);

	for (i = orig, end = orig + orig_size;
		 i != end; i++) {
		if (i->k) {
			_ivm_pthash_rehash(table, i->k, i->v);
		}
	}

	STD_FREE(orig);

	return;
}

typedef ivm_pthash_pair_t *ivm_pthash_iterator_t;

#define IVM_PTHASH_ITER_TYPE(elem_type) elem_type *
#define IVM_PTHASH_ITER_GET_KEY(iter) ((iter)->k)
#define IVM_PTHASH_ITER_GET_VAL(iter) ((iter)->v)
#define IVM_PTHASH_ITER_SET_VAL(iter, val) ((iter)->v = (val))
#define IVM_PTHASH_EACHPTR_C(pthash, iter) \
	ivm_pthash_pair_t *__ps_end_##iter##__; \
	for ((iter) = ((pthash)->table), \
		 __ps_end_##iter##__ = (iter) + (pthash)->size; \
		 (iter) != __ps_end_##iter##__; \
		 (iter)++)

#define IVM_PTHASH_EACHPTR(pthash, iter) \
	IVM_PTHASH_EACHPTR_C((pthash), iter) if ((iter)->k)

IVM_COM_END

#endif
