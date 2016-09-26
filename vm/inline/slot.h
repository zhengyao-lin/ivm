#ifndef _IVM_VM_INLINE_SLOT_H_
#define _IVM_VM_INLINE_SLOT_H_

#include "pub/com.h"
#include "pub/vm.h"

#include "vm/slot.h"

#define IS_EMPTY_SLOT(slot) (!(slot)->k)

ivm_bool_t /* return: type change? */
_ivm_slot_table_expand(ivm_slot_table_t *table,
					   ivm_vmstate_t *state);

#define SET_SLOT(check, set_cc) \
	{                                                                            \
		ivm_hash_val_t hash;                                                     \
		ivm_size_t osize;                                                        \
                                                                                 \
		register ivm_slot_t *i, *tmp, *end;                                      \
                                                                                 \
		IVM_WBSLOT(state, table, obj);                                           \
		check;                                                                   \
                                                                                 \
        if (table->mark.sub.is_hash) {                                           \
		TO_HASH_TABLE:                                                           \
			hash = ivm_string_hash(key);                                         \
			while (1) {                                                          \
				tmp = table->tabl + hash % table->size;                          \
				end = table->tabl + table->size;                                 \
                                                                                 \
				for (i = tmp;                                                    \
					 i != end;                                                   \
					 i++) {                                                      \
					if (IS_EMPTY_SLOT(i)) {                                      \
						if (obj) {                                               \
							i->k = ivm_vmstate_constantize(state, key);          \
							i->v = obj;                                          \
							set_cc;                                              \
						}                                                        \
						return;                                                  \
					} else if (ivm_string_compare(i->k, key)) {                  \
						i->v = obj;                                              \
						set_cc;                                                  \
						return;                                                  \
					}                                                            \
				}                                                                \
                                                                                 \
				for (i = table->tabl;                                            \
					 i != tmp;                                                   \
					 i++) {                                                      \
					if (IS_EMPTY_SLOT(i)) {                                      \
						if (obj) {                                               \
							i->k = ivm_vmstate_constantize(state, key);          \
							i->v = obj;                                          \
							set_cc;                                              \
						}                                                        \
						return;                                                  \
					} else if (ivm_string_compare(i->k, key)) {                  \
						i->v = obj;                                              \
						set_cc;                                                  \
						return;                                                  \
					}                                                            \
				}                                                                \
				if (!obj) return;                                                \
                                                                                 \
				/* allocate new space */                                         \
				_ivm_slot_table_expand(table, state);                            \
			}                                                                    \
		} else {                                                                 \
			for (i = table->tabl,                                                \
				 end = table->tabl + table->size;                                \
				 i != end; i++) {                                                \
				if (IS_EMPTY_SLOT(i)) {                                          \
					if (obj) {                                                   \
						i->k = ivm_vmstate_constantize(state, key);              \
						i->v = obj;                                              \
						set_cc;                                                  \
					}                                                            \
					return;                                                      \
				} else if (ivm_string_compare(i->k, key)) {                      \
					i->v = obj;                                                  \
					set_cc;                                                      \
					return;                                                      \
				}                                                                \
			}                                                                    \
                                                                                 \
			osize = table->size;                                                 \
			if (_ivm_slot_table_expand(table, state)) {                          \
				goto TO_HASH_TABLE;                                              \
			}                                                                    \
			i = table->tabl + osize;                                             \
			i->k = ivm_vmstate_constantize(state, key);                          \
			i->v = obj;                                                          \
			set_cc;                                                              \
		}                                                                        \
                                                                                 \
		return;                                                                  \
	} int dummy()

/* null obj for deleting slot */
IVM_INLINE
void
ivm_slot_table_setSlot(ivm_slot_table_t *table,
					   ivm_vmstate_t *state,
					   const ivm_string_t *key,
					   ivm_object_t *obj)
SET_SLOT(0, 0);

void
ivm_slot_table_setSlot_r(ivm_slot_table_t *table,
						 ivm_vmstate_t *state,
						 const ivm_char_t *rkey,
						 ivm_object_t *obj);

IVM_INLINE
void
ivm_slot_table_setSlot_cc(ivm_slot_table_t *table,
						  ivm_vmstate_t *state,
						  const ivm_string_t *key,
						  ivm_object_t *obj,
						  ivm_instr_t *instr)
SET_SLOT(
	if (ivm_slot_table_checkCacheValid(table, instr)) {
		//IVM_TRACE("set: %s to cache %p(in %p size: %d?)\n",
		//		  ivm_string_trimHead(ivm_slot_table_getCacheSlot(state, instr)->k),
		//		  ivm_slot_table_getCacheSlot(state, instr),
		//		  table->tabl, table->size);
		ivm_slot_table_setCacheSlot(state, instr, obj);
		return;
	},
	ivm_instr_setCache(instr, table->uid, (ivm_ptr_t)i)
);

#define GET_SLOT(check, set_cc) \
	{                                                                                   \
		ivm_hash_val_t hash;                                                            \
		register ivm_slot_t *i, *tmp, *end;                                             \
                                                                                        \
		check;                                                                          \
                                                                                        \
		if (table->mark.sub.is_hash) {                                                  \
			hash = ivm_string_hash(key) % table->size;                                  \
                                                                                        \
			tmp = table->tabl + hash;                                                   \
			end = table->tabl + table->size;                                            \
                                                                                        \
			for (i = tmp; i != end; i++) {                                              \
				if (IS_EMPTY_SLOT(i)) {                                                 \
					return IVM_NULL;                                                    \
				} else if (ivm_string_compare(i->k, key)) {                             \
					if (!i->v) return IVM_NULL;                                         \
					set_cc;                                                             \
					return i;                                                           \
				}                                                                       \
			}                                                                           \
                                                                                        \
			for (i = table->tabl; i != tmp; i++) {                                      \
				if (IS_EMPTY_SLOT(i)) {                                                 \
					return IVM_NULL;                                                    \
				} else if (ivm_string_compare(i->k, key)) {                             \
					if (!i->v) return IVM_NULL;                                         \
					set_cc;                                                             \
					return i;                                                           \
				}                                                                       \
			}                                                                           \
		} else {                                                                        \
			for (i = table->tabl,                                                       \
				 end = table->tabl + table->size;                                       \
				 i != end; i++) {                                                       \
				if (!i->k) {                                                            \
					return IVM_NULL;                                                    \
				} else if (ivm_string_compare(i->k, key)) {                             \
					if (!i->v) return IVM_NULL;                                         \
					set_cc;                                                             \
					return i;                                                           \
				}                                                                       \
			}                                                                           \
		}                                                                               \
                                                                                        \
		return IVM_NULL;                                                                \
	} int dummy()

IVM_INLINE
ivm_slot_t *
ivm_slot_table_getSlot(ivm_slot_table_t *table,
					   ivm_vmstate_t *state,
					   const ivm_string_t *key)
GET_SLOT(0, 0);

#if 0
IVM_INLINE
void
__print(ivm_vmstate_t *state, ivm_slot_table_t *table, ivm_bool_t yes)
{
	ivm_slot_t *i, *end;
	ivm_int_t j = 0;
	
	IVM_TRACE("heap1 btop: %d, curp: %p\n",
			  IVM_HEAP_GET(state->heaps, BLOCK_COUNT),
			  IVM_HEAP_GET(state->heaps, CUR_PTR));

	IVM_TRACE("heap2 btop: %d, curp: %p\n",
			  IVM_HEAP_GET(state->heaps + 1, BLOCK_COUNT),
			  IVM_HEAP_GET(state->heaps + 1, CUR_PTR));

	IVM_TRACE("table: %p, core: %p\n", table, table->tabl);
	for (i = table->tabl, end = i + table->size;
		 i != end; i++, j++) {
		if (yes && j == 1) {
			IVM_TRACE("  k: %p, v: %p, howo: %f\n", i->k, i->v, ((ivm_numeric_t *)(&(i->v)))->val);
		} else if (yes && j == 2) {
			IVM_TRACE("  k: %p, v: %p, howo: %d\n", i->k, i->v, (int)ivm_numeric_getValue((ivm_numeric_t *)i->k));
		} else
			IVM_TRACE("  k: %p, v: %p\n", i->k, i->v);
		/*if (i->k) {
			IVM_TRACE("k: %s, v: %p\n", ivm_string_trimHead(i->k), i->v);
		}*/
	}
	IVM_TRACE("end\n");
	
	return;
}
#endif

/* cache version */
IVM_INLINE
ivm_slot_t *
ivm_slot_table_getSlot_cc(ivm_slot_table_t *table,
						  ivm_vmstate_t *state,
						  const ivm_string_t *key,
						  ivm_instr_t *instr)
GET_SLOT(
	if (ivm_slot_table_checkCacheValid(table, instr)) {
		//if (ivm_slot_table_getCacheSlot(state, instr)->k == IVM_NULL) {
			// __print(state, table, IVM_TRUE);
		//	IVM_TRACE("something wrong %d %d, %d %d\n",
		//			  ivm_heap_isIn(state->heaps, table), IVM_HEAP_GET(state->heaps, BLOCK_TOP),
		//			  ivm_heap_isIn(state->heaps + 1, table), IVM_HEAP_GET(state->heaps + 1, BLOCK_TOP));
		//}
		//IVM_TRACE("got it! %p %p\n",
		//		  ivm_slot_table_getCacheSlot(state, instr)->k,
		//		  ivm_slot_table_getCacheSlot(state, instr)->v);

		tmp = ivm_slot_table_getCacheSlot(state, instr);
		return tmp->v ? tmp : IVM_NULL;
	},
	ivm_instr_setCache(instr, table->uid, (ivm_ptr_t)i)
);

#define SET_EMPTY_SLOT() \
	{                                                                            \
		ivm_hash_val_t hash;                                                     \
		ivm_size_t osize;                                                        \
                                                                                 \
		register ivm_slot_t *i, *tmp, *end;                                      \
                                                                                 \
		IVM_WBSLOT(state, table, obj);                                           \
                                                                                 \
        if (table->mark.sub.is_hash) {                                           \
		TO_HASH_TABLE:                                                           \
			hash = ivm_string_hash(key);                                         \
			while (1) {                                                          \
				tmp = table->tabl + hash % table->size;                          \
				end = table->tabl + table->size;                                 \
                                                                                 \
				for (i = tmp;                                                    \
					 i != end;                                                   \
					 i++) {                                                      \
					if (IS_EMPTY_SLOT(i)) {                                      \
						i->k = ivm_vmstate_constantize(state, key);              \
						i->v = obj;                                              \
						return IVM_TRUE;                                         \
					} else if (ivm_string_compare(i->k, key)) {                  \
						return IVM_FALSE;                                        \
					}                                                            \
				}                                                                \
                                                                                 \
				for (i = table->tabl;                                            \
					 i != tmp;                                                   \
					 i++) {                                                      \
					if (IS_EMPTY_SLOT(i)) {                                      \
						i->k = ivm_vmstate_constantize(state, key);              \
						i->v = obj;                                              \
						return IVM_TRUE;                                         \
					} else if (ivm_string_compare(i->k, key)) {                  \
						return IVM_FALSE;                                        \
					}                                                            \
				}                                                                \
				/* allocate new space */                                         \
				_ivm_slot_table_expand(table, state);                            \
			}                                                                    \
		} else {                                                                 \
			for (i = table->tabl,                                                \
				 end = table->tabl + table->size;                                \
				 i != end; i++) {                                                \
				if (IS_EMPTY_SLOT(i)) {                                          \
					i->k = ivm_vmstate_constantize(state, key);                  \
					i->v = obj;                                                  \
					return IVM_TRUE;                                             \
				} else if (ivm_string_compare(i->k, key)) {                      \
					return IVM_FALSE;                                            \
				}                                                                \
			}                                                                    \
                                                                                 \
			osize = table->size;                                                 \
			if (_ivm_slot_table_expand(table, state)) {                          \
				goto TO_HASH_TABLE;                                              \
			}                                                                    \
			tmp = table->tabl + osize;                                           \
			tmp->k = ivm_vmstate_constantize(state, key);                        \
			tmp->v = obj;                                                        \
		}                                                                        \
                                                                                 \
		return IVM_TRUE;                                                         \
	} int dummy()

IVM_INLINE
ivm_bool_t
ivm_slot_table_setEmptySlot(ivm_slot_table_t *table,
							ivm_vmstate_t *state,
							const ivm_string_t *key,
							ivm_object_t *obj)
SET_EMPTY_SLOT();

ivm_bool_t
ivm_slot_table_setEmptySlot_r(ivm_slot_table_t *table,
							  ivm_vmstate_t *state,
							  const ivm_char_t *rkey,
							  ivm_object_t *obj);

IVM_INLINE
void
_ivm_slot_table_rehash(ivm_slot_table_t *table,
					   ivm_vmstate_t *state,
					   const ivm_string_t *key,
					   ivm_object_t *obj)
{
	register ivm_slot_t *i, *tmp, *end;

	IVM_ASSERT(table->mark.sub.is_hash, "impossible");
	tmp = table->tabl + ivm_string_hash(key) % table->size;
	end = table->tabl + table->size;

	for (i = tmp; i != end; i++) {
		if (IS_EMPTY_SLOT(i)) {
			i->k = key;
			i->v = obj;
			return;
		}
	}

	for (i = table->tabl; i != tmp; i++) {
		if (IS_EMPTY_SLOT(i)) {
			i->k = key;
			i->v = obj;
			return;
		}
	}

	IVM_FATAL("impossible");
	return;
}

#undef SET_SLOT
#undef GET_SLOT
#undef SET_EMPTY_SLOT

#undef IS_EMPTY_SLOT

IVM_INLINE
ivm_bool_t
ivm_slot_table_setExistSlot(ivm_slot_table_t *table,
							ivm_vmstate_t *state,
							const ivm_string_t *key,
							ivm_object_t *obj)
{
	ivm_slot_t *slot = ivm_slot_table_getSlot(table, state, key);

	if (slot) {
		IVM_WBSLOT(state, table, obj);
		_ivm_slot_setValue(slot, obj);

		return IVM_TRUE;
	}

	return IVM_FALSE;
}

IVM_INLINE
ivm_bool_t
ivm_slot_table_setExistSlot_cc(ivm_slot_table_t *table,
							   ivm_vmstate_t *state,
							   const ivm_string_t *key,
							   ivm_object_t *obj,
							   ivm_instr_t *instr)
{
	ivm_slot_t *slot = ivm_slot_table_getSlot_cc(table, state, key, instr);

	if (slot) {
		IVM_WBSLOT(state, table, obj);
		_ivm_slot_setValue(slot, obj);

		return IVM_TRUE;
	}

	return IVM_FALSE;
}

IVM_INLINE
ivm_bool_t
ivm_slot_table_hasOop(ivm_slot_table_t *table)
{
	ivm_object_t **i, **end;
	ivm_uint_t count = table->mark.sub.oop_count;

	if (count) {
		for (i = table->oops, end = i + count;
			 i != end; i++) {
			if (*i) return IVM_TRUE;
		}
	}

	return IVM_FALSE;
}

IVM_INLINE
void
ivm_slot_table_setOop(ivm_slot_table_t *table,
					  ivm_vmstate_t *state,
					  ivm_int_t op,
					  ivm_object_t *func)
{
	if (op >= table->mark.sub.oop_count) {
		_ivm_slot_table_expandOopTo(table, state, op + 1);
	}

	IVM_WBSLOT(state, table, func);
	table->oops[op] = func;

	return;
}

IVM_INLINE
ivm_object_t *
ivm_slot_table_getOop(ivm_slot_table_t *table,
					  ivm_int_t op)
{
	if (!table || op >= table->mark.sub.oop_count) {
		return IVM_NULL;
	}

	return table->oops[op];
}

#endif
