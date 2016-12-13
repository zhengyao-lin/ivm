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
} ivm_pthash_t;

void
ivm_pthash_init(ivm_pthash_t *set);

void
ivm_pthash_dump(ivm_pthash_t *set);

IVM_INLINE
void
_ivm_pthash_expand(ivm_pthash_t *set);

IVM_INLINE
ivm_pthash_pair_t *
ivm_pthash_find(ivm_pthash_t *set,
				void *key)
{
	ivm_hash_val_t hash;
	register ivm_pthash_pair_t *end, *tmp, *i;
	hash = ivm_hash_fromPointer(key);

	if (!key) return IVM_NULL;

	end = set->table + set->size;
	tmp = set->table + (hash & (set->size - 1));
	
	for (i = tmp; i != end; i++) {
		if (!i->k || i->k == key) {
			return i;
		}
	}
	
	for (i = set->table; i != tmp; i++) {
		if (!i->k || i->k == key) {
			return i;
		}
	}

	return IVM_NULL;
}

IVM_INLINE
void
ivm_pthash_insert(ivm_pthash_t *set,
				  void *key, void *val)
{
	ivm_pthash_pair_t *found;

	if (!key) return;

	while (1) {
		found = ivm_pthash_find(set, key);

		if (found) {
			if (!found->k) {
				found->k = key;
			}

			found->v = val;

			return;
		} else {
			_ivm_pthash_expand(set);
		}
	}

	return;
}

IVM_INLINE
void
_ivm_pthash_rehash(ivm_pthash_t *set,
				   void *key,
				   void *val)
{
	ivm_pthash_pair_t *found;

	found = ivm_pthash_find(set, val);

	if (!found) {
		IVM_FATAL("impossible");
	}

	found->k = key;
	found->v = val;

	return;
}

IVM_INLINE
ivm_bool_t /* found the ptr? */
ivm_pthash_remove(ivm_pthash_t *set,
				  void *key)
{
	ivm_pthash_pair_t *found = ivm_pthash_find(set, key);
	ivm_pthash_pair_t *table, *end, *i;
	ivm_size_t mask;
	ivm_hash_val_t hash;

	if (found && found->k) {
		found->k = IVM_NULL;

		table = set->table;
		end = table + set->size;
		mask = set->size - 1;

		while (1) {
			for (i = found + 1; i != end; i++) {
				if (!i->k) return IVM_TRUE; // next gap: no influence
				
				hash = ivm_hash_fromPointer(i->k) & mask;
				
				if (table + hash <= found ||
					table + hash > i) {
					goto FILL; // found a filling element
				}
			}

			for (i = table; i != found; i++) {
				if (!i->k) return IVM_TRUE; // next gap: no influence
				
				hash = ivm_hash_fromPointer(i->k) & mask;
				
				if (table + hash > i) {
					goto FILL; // found a filling element
				}
			}

		FILL:
			*found = *i;
			found = i;
			found->k = IVM_NULL;
		}

	} else return IVM_FALSE;
}

IVM_INLINE
void
_ivm_pthash_expand(ivm_pthash_t *set)
{
	ivm_size_t orig_size = set->size;
	ivm_pthash_pair_t *orig = set->table, *i, *end;

	set->size <<= 1;
	set->table = STD_ALLOC_INIT(sizeof(*set->table) * set->size);

	IVM_MEMCHECK(set->table);

	for (i = orig, end = orig + orig_size;
		 i != end; i++) {
		if (i->k) {
			_ivm_pthash_rehash(set, i->k, i->v);
		}
	}

	return;
}

typedef ivm_pthash_pair_t *ivm_pthash_iterator_t;

#define IVM_PTHASH_ITER_TYPE(elem_type) elem_type *
#define IVM_PTHASH_ITER_GET_KEY(iter) ((iter)->k)
#define IVM_PTHASH_ITER_GET_VAL(iter) ((iter)->v)
#define IVM_PTHASH_EACHPTR(pthash, iter, type) \
	type *__ps_end_##iter##__; \
	for ((iter) = (type *)((pthash)->table), \
		 __ps_end_##iter##__ = (iter) + (pthash)->size; \
		 (iter) != __ps_end_##iter##__; \
		 (iter)++)

IVM_COM_END

#endif
