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
	ivm_size_t size;
	ivm_slot_t *tabl;
	union {
		struct {
			ivm_int_t dummy1: sizeof(ivm_ptr_t) / 2 * 8;
			ivm_int_t dummy2: sizeof(ivm_ptr_t) / 2 * 8 - 4;
			ivm_int_t is_hash: 1;
			ivm_int_t is_shared: 1; // shared by multiple objects
			ivm_int_t gen: 2;
		} sub;
		struct ivm_slot_table_t_tag *copy;
	} mark;
	ivm_uid_t uid;
} ivm_slot_table_t;

#define ivm_slot_table_getCopy(table) ((ivm_slot_table_t *)((((ivm_uptr_t)(table)->mark.copy) << 4) >> 4))

IVM_INLINE
void
ivm_slot_table_setCopy(ivm_slot_table_t *table,
					   ivm_slot_table_t *copy)
{
	table->mark.copy = (ivm_slot_table_t *)
					   ((((ivm_uptr_t)table->mark.copy
					   	>> (sizeof(ivm_ptr_t) * 8 - 4))
						<< (sizeof(ivm_ptr_t) * 8 - 4))
						| (ivm_uptr_t)copy);
	return;
}

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
	IVM_BIT_SET_TRUE(table->mark.sub.is_shared);
	return table;
}

#define ivm_slot_table_isShared(table) ((table)->mark.sub.is_shared)

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

/* be sure table is not null */
IVM_INLINE
ivm_bool_t
ivm_slot_table_checkCacheValid(ivm_slot_table_t *table,
							   ivm_instr_t *instr)
{
	return ivm_instr_cacheID(instr) == table->uid;
}

#define ivm_slot_table_getCacheSlot(state, instr) \
	(ivm_slot_getValue(((ivm_slot_t *)ivm_instr_cacheData(instr)), (state)))

#define ivm_slot_table_setCacheSlot(state, instr, value) \
	(ivm_slot_setValue(((ivm_slot_t *)ivm_instr_cacheData(instr)), (state), (value)))

#define IS_EMPTY_SLOT(slot) (!(slot)->k)

ivm_bool_t /* type change? linear table -> hash table */
_ivm_slot_table_expand(ivm_slot_table_t *table,
					   struct ivm_vmstate_t_tag *state);

#define SET_SLOT(e1, e2) \
	{                                                                            \
		ivm_hash_val_t hash;                                                     \
		ivm_size_t osize;                                                        \
                                                                                 \
		register ivm_slot_t *i, *tmp, *end;                                      \
                                                                                 \
		if (table->mark.sub.is_hash) {                                           \
		TO_HASH_TABLE:                                                           \
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
			if (_ivm_slot_table_expand(table, state)) {                          \
				goto TO_HASH_TABLE;                                              \
			}                                                                    \
			tmp = table->tabl + osize;                                           \
			tmp->k = ivm_string_copyIfNotConst_pool(key, state);                 \
			tmp->v = obj;                                                        \
			e2;                                                                  \
		}                                                                        \
                                                                                 \
		return;                                                                  \
	} int dummy()

IVM_INLINE
void
ivm_slot_table_setSlot(ivm_slot_table_t *table,
					   struct ivm_vmstate_t_tag *state,
					   const ivm_string_t *key,
					   struct ivm_object_t_tag *obj)
SET_SLOT(0, 0);

void
ivm_slot_table_setSlot_r(ivm_slot_table_t *table,
						 struct ivm_vmstate_t_tag *state,
						 const ivm_char_t *rkey,
						 struct ivm_object_t_tag *obj);

IVM_INLINE
void
ivm_slot_table_setSlot_cc(ivm_slot_table_t *table,
						  struct ivm_vmstate_t_tag *state,
						  const ivm_string_t *key,
						  struct ivm_object_t_tag *obj,
						  ivm_instr_t *instr)
SET_SLOT(
	ivm_instr_setCache(instr, table->uid, (ivm_ptr_t)i),
	ivm_instr_setCache(instr, table->uid, (ivm_ptr_t)tmp)
);

#define GET_SLOT(e1, e2) \
	{                                                                                   \
		ivm_hash_val_t hash;                                                            \
		register ivm_slot_t *i, *tmp, *end;                                             \
                                                                                        \
		if (table->mark.sub.is_hash) {                                                  \
			hash = ivm_hash_fromString(ivm_string_trimHead(key)) % table->size;         \
                                                                                        \
			tmp = table->tabl + hash;                                                   \
			end = table->tabl + table->size;                                            \
                                                                                        \
			for (i = tmp; i != end; i++) {                                              \
				if (IS_EMPTY_SLOT(i)) {                                                 \
					return IVM_NULL;                                                    \
				} else if (ivm_string_compare(i->k, key)) {                             \
					e1;                                                                 \
					return i;                                                           \
				}                                                                       \
			}                                                                           \
                                                                                        \
			for (i = table->tabl; i != tmp; i++) {                                      \
				if (IS_EMPTY_SLOT(i)) {                                                 \
					return IVM_NULL;                                                    \
				} else if (ivm_string_compare(i->k, key)) {                             \
					e1;                                                                 \
					return i;                                                           \
				}                                                                       \
			}                                                                           \
		} else {                                                                        \
			for (tmp = table->tabl,                                                     \
				 end = table->tabl + table->size;                                       \
				 tmp != end; tmp++) {                                                   \
				if (!tmp->k) {                                                          \
					return IVM_NULL;                                                    \
				} else if (ivm_string_compare(tmp->k, key)) {                           \
					e2;                                                                 \
					return tmp;                                                         \
				}                                                                       \
			}                                                                           \
		}                                                                               \
                                                                                        \
		return IVM_NULL;                                                                \
	} int dummy()

IVM_INLINE
ivm_slot_t *
ivm_slot_table_getSlot(ivm_slot_table_t *table,
						struct ivm_vmstate_t_tag *state,
						const ivm_string_t *key)
GET_SLOT(0, 0);

/* cache version */
IVM_INLINE
ivm_slot_t *
ivm_slot_table_getSlot_cc(ivm_slot_table_t *table,
						   struct ivm_vmstate_t_tag *state,
						   const ivm_string_t *key,
						   ivm_instr_t *instr)
GET_SLOT(
	ivm_instr_setCache(instr, table->uid, (ivm_ptr_t)i),
	ivm_instr_setCache(instr, table->uid, (ivm_ptr_t)tmp)
);

#undef SET_SLOT
#undef GET_SLOT

#undef IS_EMPTY_SLOT

IVM_INLINE
ivm_bool_t
ivm_slot_table_setExistSlot(ivm_slot_table_t *table,
							struct ivm_vmstate_t_tag *state,
							const ivm_string_t *key,
							struct ivm_object_t_tag *obj)
{
	ivm_slot_t *slot = ivm_slot_table_getSlot(table, state, key);

	if (slot) {
		ivm_slot_setValue(slot, state, obj);
		return IVM_TRUE;
	}

	return IVM_FALSE;
}

IVM_INLINE
ivm_bool_t
ivm_slot_table_setExistSlot_cc(ivm_slot_table_t *table,
							   struct ivm_vmstate_t_tag *state,
							   const ivm_string_t *key,
							   struct ivm_object_t_tag *obj,
							   ivm_instr_t *instr)
{
	ivm_slot_t *slot = ivm_slot_table_getSlot_cc(table, state, key, instr);

	if (slot) {
		ivm_slot_setValue(slot, state, obj);
		return IVM_TRUE;
	}

	return IVM_FALSE;
}

IVM_COM_END

#endif
