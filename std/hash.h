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
	void **table;
	ivm_size_t size;
} ivm_ptset_t;

void
ivm_ptset_init(ivm_ptset_t *set);

void
ivm_ptset_dump(ivm_ptset_t *set);

IVM_INLINE
void
_ivm_ptset_expand(ivm_ptset_t *set);

IVM_INLINE
void **
ivm_ptset_find(ivm_ptset_t *set,
			   void *val)
{
	ivm_hash_val_t hash;
	register void **end, **tmp, **i;
	hash = ivm_hash_fromPointer(val);

	end = set->table + set->size;
	tmp = set->table + (hash & (set->size - 1));
	
	for (i = tmp; i != end; i++) {
		if (!*i || *i == val) {
			return i;
		}
	}
	
	for (i = set->table; i != tmp; i++) {
		if (!*i || *i == val) {
			return i;
		}
	}

	return IVM_NULL;
}

IVM_INLINE
void
ivm_ptset_insert(ivm_ptset_t *set, void *val)
{
	void **found;

	while (1) {
		found = ivm_ptset_find(set, val);

		if (found) {
			if (!*found) {
				*found = val;
			}

			return;
		} else {
			_ivm_ptset_expand(set);
		}
	}

	return;
}

IVM_INLINE
void
ivm_ptset_rehash(ivm_ptset_t *set, void *val)
{
	void **found;

	found = ivm_ptset_find(set, val);

	IVM_ASSERT(found, "impossible");
	*found = val;

	return;
}

IVM_INLINE
ivm_bool_t /* found the ptr? */
ivm_ptset_remove(ivm_ptset_t *set,
				 void *val)
{
	void **found = ivm_ptset_find(set, val);
	void **table, **end, **i;
	ivm_size_t mask;
	ivm_hash_val_t hash;

	if (found && *found) {
		*found = IVM_NULL;

		table = set->table;
		end = table + set->size;
		mask = set->size - 1;

		while (1) {
			for (i = found + 1; i != end; i++) {
				if (!*i) return IVM_TRUE; // next gap: no influence
				
				hash = ivm_hash_fromPointer(*i) & mask;
				
				if (table + hash <= found ||
					table + hash > i) {
					goto FILL; // found a filling element
				}
			}

			for (i = table; i != found; i++) {
				if (!*i) return IVM_TRUE; // next gap: no influence
				
				hash = ivm_hash_fromPointer(*i) & mask;
				
				if (table + hash > i) {
					goto FILL; // found a filling element
				}
			}

		FILL:
			*found = *i;
			found = i;
			*found = IVM_NULL;
		}

	} else return IVM_FALSE;
}

IVM_INLINE
void
_ivm_ptset_expand(ivm_ptset_t *set)
{
	ivm_size_t orig_size = set->size;
	void **orig = set->table,
		 **i, **end;

	set->size <<= 1;
	set->table = STD_ALLOC_INIT(sizeof(*set->table) * set->size);

	IVM_MEMCHECK(set->table);

	for (i = orig, end = orig + orig_size;
		 i != end; i++) {
		if (*i) {
			ivm_ptset_rehash(set, *i);
		}
	}

	return;
}

typedef void **ivm_ptset_iterator_t;

#define IVM_PTSET_ITER_TYPE(elem_type) elem_type *
#define IVM_PTSET_ITER_GET(iter) (*(iter))
#define IVM_PTSET_EACHPTR(ptset, iter, type) \
	type *__ps_end_##iter##__; \
	for ((iter) = (type *)((ptset)->table), \
		 __ps_end_##iter##__ = (iter) + (ptset)->size; \
		 (iter) != __ps_end_##iter##__; \
		 (iter)++)

IVM_COM_END

#endif
