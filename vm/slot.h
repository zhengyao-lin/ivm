#ifndef _IVM_VM_SLOT_H_
#define _IVM_VM_SLOT_H_

#include "pub/com.h"
#include "pub/type.h"

#include "std/hash.h"
#include "std/string.h"
#include "std/uid.h"
#include "std/bit.h"

#include "instr.h"

IVM_COM_HEADER

struct ivm_object_t_tag;
struct ivm_vmstate_t_tag;
struct ivm_slot_t_tag;
struct ivm_heap_t_tag;

typedef struct ivm_slot_t_tag {
	const ivm_string_t *k;
	struct ivm_object_t_tag *v;
} ivm_slot_t;

IVM_INLINE
void
ivm_slot_setValue(ivm_slot_t *slot,
				  struct ivm_vmstate_t_tag *state,
				  struct ivm_object_t_tag *obj)
{
	if (slot) {
		slot->v = obj;
	}

	return;
}

IVM_INLINE
struct ivm_object_t_tag *
ivm_slot_getValue(ivm_slot_t *slot,
				  struct ivm_vmstate_t_tag *state)
{
	return slot ? slot->v : IVM_NULL;
}

typedef struct ivm_slot_table_t_tag {
	ivm_bool_t is_hash: 1;
	ivm_bool_t is_shared: 1; // shared by multiple objects
	ivm_size_t size;
	ivm_uid_t uid;
	ivm_slot_t *tabl;
} ivm_slot_table_t;

ivm_slot_table_t *
ivm_slot_table_new(struct ivm_vmstate_t_tag *state);

ivm_slot_table_t *
ivm_slot_table_new_c(struct ivm_vmstate_t_tag *state,
					 ivm_size_t prealloc);

ivm_slot_table_t *
ivm_slot_table_copy(ivm_slot_table_t *table,
					struct ivm_vmstate_t_tag *state,
					struct ivm_heap_t_tag *heap);

typedef ivm_slot_t *ivm_slot_table_iterator_t;

IVM_INLINE
ivm_slot_table_t *
ivm_slot_table_copyShared(ivm_slot_table_t *table)
{
	IVM_BIT_SET_TRUE(table->is_shared);
	return table;
}

#define ivm_slot_table_isShared(table) ((table)->is_shared)

ivm_slot_table_t *
_ivm_slot_table_copy_state(ivm_slot_table_t *table,
						   struct ivm_vmstate_t_tag *state);

IVM_INLINE
ivm_slot_table_t *
ivm_slot_table_copyOnWrite(ivm_slot_table_t *table,
						   struct ivm_vmstate_t_tag *state)
{
	// IVM_TRACE("COW!!\n");
	return _ivm_slot_table_copy_state(table, state);
}

#define IVM_SLOT_TABLE_ITER_SET_KEY(iter, key) ((iter)->k = (key))
#define IVM_SLOT_TABLE_ITER_SET_VAL(iter, val) ((iter)->v = (val))
#define IVM_SLOT_TABLE_ITER_SET(iter, key, val) ((iter)->k = (key), (iter)->v = (val))
#define IVM_SLOT_TABLE_ITER_GET_KEY(iter) ((iter)->k)
#define IVM_SLOT_TABLE_ITER_GET_VAL(iter) ((iter)->v)
#define IVM_SLOT_TABLE_EACHPTR(table, iter) \
	ivm_slot_t *__sl_end_##iter##__; \
	for ((iter) = (table)->tabl, \
		 __sl_end_##iter##__ = (iter) + (table)->size; \
		 (iter) < __sl_end_##iter##__; \
		 (iter)++)

#define IS_EMPTY_SLOT(slot) (!(slot)->k)

IVM_INLINE
ivm_bool_t
ivm_slot_table_checkCacheValid(ivm_slot_table_t *table,
							   ivm_instr_cache_t *cache)
{
	return table && ivm_instr_cache_id(cache) == table->uid;
}

#define IS_EMPTY_SLOT(slot) (!(slot)->k)

void
_ivm_slot_table_expand(ivm_slot_table_t *table,
					   struct ivm_vmstate_t_tag *state);

#define ADD_SLOT(e1, e2) \
	{                                                                            \
		ivm_hash_val_t hash;                                                     \
		ivm_size_t osize;                                                        \
                                                                                 \
		register ivm_slot_t *i, *tmp, *end;                                      \
                                                                                 \
		if (table->is_hash) {                                                    \
			hash = ivm_hash_fromString(ivm_string_trimHead(key));                \
			while (1) {                                                          \
				tmp = table->tabl + hash % table->size;                          \
				end = table->tabl + table->size;                                 \
                                                                                 \
				for (i = tmp;                                                    \
					 i != end;                                                   \
					 i++) {                                                      \
					if (IS_EMPTY_SLOT(i)) {                                      \
						i->k = ivm_string_copyIfNotConst_pool(key, state);       \
						i->v = obj;                                              \
						e1;                                                      \
						return;                                                  \
					} else if (ivm_string_compare(i->k, key)) {                  \
						i->v = obj;                                              \
						e1;                                                      \
						return;                                                  \
					}                                                            \
				}                                                                \
                                                                                 \
				for (i = table->tabl;                                            \
					 i != tmp;                                                   \
					 i++) {                                                      \
					if (IS_EMPTY_SLOT(i)) {                                      \
						i->k = ivm_string_copyIfNotConst_pool(key, state);       \
						i->v = obj;                                              \
						e1;                                                      \
						return;                                                  \
					} else if (ivm_string_compare(i->k, key)) {                  \
						i->v = obj;                                              \
						e1;                                                      \
						return;                                                  \
					}                                                            \
				}                                                                \
                                                                                 \
				/* allocate new space */                                         \
				_ivm_slot_table_expand(table, state);                            \
			}                                                                    \
		} else {                                                                 \
			for (i = table->tabl,                                                \
				 end = table->tabl + table->size;                                \
				 i != end; i++) {                                                \
				if (i->k == IVM_NULL) {                                          \
					i->k = ivm_string_copyIfNotConst_pool(key, state);           \
					i->v = obj;                                                  \
					e1;                                                          \
					return;                                                      \
				} else if (ivm_string_compare(i->k, key)) {                      \
					i->v = obj;                                                  \
					e1;                                                          \
					return;                                                      \
				}                                                                \
			}                                                                    \
                                                                                 \
			osize = table->size;                                                 \
			_ivm_slot_table_expand(table, state);                                \
			tmp = table->tabl + osize;                                           \
			tmp->k = ivm_string_copyIfNotConst_pool(key, state);                 \
			tmp->v = obj;                                                        \
			e2;                                                                  \
		}                                                                        \
                                                                                 \
		return;                                                                  \
	}

IVM_INLINE
void
ivm_slot_table_addSlot_cc(ivm_slot_table_t *table,
						  struct ivm_vmstate_t_tag *state,
						  const ivm_string_t *key,
						  struct ivm_object_t_tag *obj,
						  ivm_instr_cache_t *cache)
ADD_SLOT(
	*cache = ivm_instr_cache_build(table->uid, (ivm_ptr_t)i),
	*cache = ivm_instr_cache_build(table->uid, (ivm_ptr_t)tmp)
)

IVM_INLINE
void
ivm_slot_table_addSlot(ivm_slot_table_t *table,
					   struct ivm_vmstate_t_tag *state,
					   const ivm_string_t *key,
					   struct ivm_object_t_tag *obj)
ADD_SLOT(0, 0)

#define FIND_SLOT(e1, e2) \
	{                                                                                   \
		ivm_hash_val_t hash;                                                            \
		register ivm_slot_t *i, *tmp, *end;                                             \
                                                                                        \
		if (table) {                                                                    \
			if (table->is_hash) {                                                       \
				hash = ivm_hash_fromString(ivm_string_trimHead(key)) % table->size;     \
                                                                                        \
				tmp = table->tabl + hash;                                               \
				end = table->tabl + table->size;                                        \
                                                                                        \
				for (i = tmp; i != end; i++) {                                          \
					if (IS_EMPTY_SLOT(i)) {                                             \
						return IVM_NULL;                                                \
					} else if (ivm_string_compare(i->k, key)) {                         \
						e1;                                                             \
						return i;                                                       \
					}                                                                   \
				}                                                                       \
                                                                                        \
				for (i = table->tabl; i != tmp; i++) {                                  \
					if (IS_EMPTY_SLOT(i)) {                                             \
						return IVM_NULL;                                                \
					} else if (ivm_string_compare(i->k, key)) {                         \
						e1;                                                             \
						return i;                                                       \
					}                                                                   \
				}                                                                       \
			} else {                                                                    \
				for (tmp = table->tabl,                                                 \
					 end = table->tabl + table->size;                                   \
					 tmp != end; tmp++) {                                               \
					if (!tmp->k) {                                                      \
						return IVM_NULL;                                                \
					} else if (ivm_string_compare(tmp->k, key)) {                       \
						e2;                                                             \
						return tmp;                                                     \
					}                                                                   \
				}                                                                       \
			}                                                                           \
		}                                                                               \
                                                                                        \
		return IVM_NULL;                                                                \
	}

/* cache version */
IVM_INLINE
ivm_slot_t *
ivm_slot_table_findSlot_cc(ivm_slot_table_t *table,
						   struct ivm_vmstate_t_tag *state,
						   const ivm_string_t *key,
						   ivm_instr_cache_t *cache)
FIND_SLOT(
	*cache = ivm_instr_cache_build(table->uid, (ivm_ptr_t)i),
	*cache = ivm_instr_cache_build(table->uid, (ivm_ptr_t)tmp)
)

IVM_INLINE
ivm_slot_t *
ivm_slot_table_findSlot(ivm_slot_table_t *table,
						struct ivm_vmstate_t_tag *state,
						const ivm_string_t *key)
FIND_SLOT(0, 0)


#undef FIND_SLOT

#undef IS_EMPTY_SLOT

IVM_COM_END

#endif
