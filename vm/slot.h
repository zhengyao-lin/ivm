#ifndef _IVM_VM_SLOT_H_
#define _IVM_VM_SLOT_H_

#include "pub/com.h"
#include "pub/type.h"

#include "std/hash.h"
#include "std/string.h"
#include "std/uid.h"
#include "std/bit.h"

#include "instr.h"
#include "oprt.h"

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
_ivm_slot_setValue(ivm_slot_t *slot,
				   struct ivm_object_t_tag *obj)
{
	slot->v = obj;
	return;
}

IVM_INLINE
struct ivm_object_t_tag *
ivm_slot_getValue(ivm_slot_t *slot,
				  struct ivm_vmstate_t_tag *state)
{
	return slot ? slot->v : IVM_NULL;
}

#define _IVM_SLOT_TABLE_MARK_HEADER_BITS 10

typedef struct ivm_slot_table_t_tag {
	ivm_size_t size;
	ivm_slot_t *tabl;
	struct ivm_object_t_tag **oops;
	union {
		struct {
			ivm_int_t dummy1: sizeof(ivm_sint64_t) / 2 * 8;
			ivm_int_t dummy2: sizeof(ivm_sint64_t) / 2 * 8 - _IVM_SLOT_TABLE_MARK_HEADER_BITS;
			ivm_uint_t oop_count: 6; // max 64
			ivm_uint_t is_hash: 1;
			ivm_uint_t is_linked: 1; // assert(is_linked & is_shared != 1)
			ivm_uint_t is_shared: 1; // shared by multiple objects, need to copy on write
			ivm_uint_t wb: 1;
			ivm_uint_t gen: 1;
		} sub;
		struct ivm_slot_table_t_tag *copy;
	} mark;
	ivm_uid_t uid;
} ivm_slot_table_t;

#define ivm_slot_table_getCopy(table) \
	((ivm_slot_table_t *)((((ivm_uptr_t)(table)->mark.copy)        \
							<< _IVM_SLOT_TABLE_MARK_HEADER_BITS)   \
							>> _IVM_SLOT_TABLE_MARK_HEADER_BITS))

IVM_INLINE
void
ivm_slot_table_setCopy(ivm_slot_table_t *table,
					   ivm_slot_table_t *copy)
{
	table->mark.copy = (ivm_slot_table_t *)
					   ((((ivm_uptr_t)table->mark.copy
					   	>> (sizeof(ivm_ptr_t) * 8 - _IVM_SLOT_TABLE_MARK_HEADER_BITS))
						<< (sizeof(ivm_ptr_t) * 8 - _IVM_SLOT_TABLE_MARK_HEADER_BITS))
						| (ivm_uptr_t)copy);
	return;
}

// #undef _IVM_SLOT_TABLE_MARK_HEADER_BITS

#define ivm_slot_table_getWB(table) ((table)->mark.sub.wb)
#define ivm_slot_table_setWB(table, val) ((table)->mark.sub.wb = (val))
#define ivm_slot_table_getGen(table) ((table)->mark.sub.gen)
#define ivm_slot_table_setGen(table, val) ((table)->mark.sub.gen = (val))
#define ivm_slot_table_incGen(table) (++(table)->mark.sub.gen)

#define ivm_slot_table_updateUID(table, state) \
	((table)->uid = ivm_vmstate_genUID(state))

ivm_slot_table_t *
ivm_slot_table_new(struct ivm_vmstate_t_tag *state);

ivm_slot_table_t *
ivm_slot_table_newAt(struct ivm_vmstate_t_tag *state, ivm_int_t gen);

ivm_slot_table_t *
ivm_slot_table_new_c(struct ivm_vmstate_t_tag *state,
					 ivm_size_t prealloc);

ivm_slot_table_t *
ivm_slot_table_newAt_c(struct ivm_vmstate_t_tag *state,
					   ivm_size_t prealloc,
					   ivm_int_t gen);

ivm_slot_table_t *
ivm_slot_table_copy(ivm_slot_table_t *table,
					struct ivm_vmstate_t_tag *state,
					struct ivm_heap_t_tag *heap);

/* merge tb to tas */
void
ivm_slot_table_merge(ivm_slot_table_t *ta,
					 struct ivm_vmstate_t_tag *state,
					 ivm_slot_table_t *tb,
					 ivm_bool_t overw);

ivm_slot_table_t *
ivm_slot_table_copy_state(ivm_slot_table_t *table,
						  struct ivm_vmstate_t_tag *state);

#define ivm_slot_table_isLinked(table) ((table)->mark.sub.is_linked)
#define ivm_slot_table_setLinked(table) ((table)->mark.sub.is_linked = IVM_TRUE)
#define ivm_slot_table_isShared(table) ((table)->mark.sub.is_shared)

IVM_INLINE
ivm_slot_table_t *
ivm_slot_table_copyOnWrite(ivm_slot_table_t *table,
						   struct ivm_vmstate_t_tag *state)
{
	// IVM_TRACE("COW!!\n");
	if (table && ivm_slot_table_isShared(table)) {
		return ivm_slot_table_copy_state(table, state);
	}

	return table;
}

IVM_INLINE
ivm_slot_table_t *
ivm_slot_table_copyShared(ivm_slot_table_t *table,
						  struct ivm_vmstate_t_tag *state)
{
	if (table) {
		if (ivm_slot_table_isLinked(table)) {
			return ivm_slot_table_copy_state(table, state);
		}

		table->mark.sub.is_shared = IVM_TRUE;
		return table;
	}
	
	return IVM_NULL;
}

void
ivm_slot_table_expandTo(ivm_slot_table_t *table,
						struct ivm_vmstate_t_tag *state,
						ivm_size_t to);

void
_ivm_slot_table_expandOopTo(ivm_slot_table_t *table,
							struct ivm_vmstate_t_tag *state,
							ivm_size_t size);

typedef ivm_slot_t *ivm_slot_table_iterator_t;

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
		 (iter)++) if ((iter)->v)

/* be sure table is not null */
IVM_INLINE
ivm_bool_t
ivm_slot_table_checkCacheValid(ivm_slot_table_t *table,
							   ivm_instr_t *instr)
{
	return ivm_instr_cacheID(instr) == table->uid;
}

#define ivm_slot_table_getCacheSlot(state, instr) \
	((ivm_slot_t *)ivm_instr_cacheData(instr))

#define ivm_slot_table_setCacheSlot(state, instr, value) \
	(_ivm_slot_setValue((ivm_slot_t *)ivm_instr_cacheData(instr), (value)))

#define ivm_slot_table_getOops(table) ((table)->oops)
#define ivm_slot_table_getOopCount(table) ((table)->mark.sub.oop_count)

IVM_COM_END

#endif
