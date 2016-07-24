#ifndef _IVM_VM_INLINE_SLOT_H_
#define _IVM_VM_INLINE_SLOT_H_

#include "pub/com.h"
#include "pub/vm.h"

#include "../slot.h"

#define IS_EMPTY_SLOT(slot) (!(slot)->k)

ivm_bool_t /* return: type change? */
_ivm_slot_table_expand(ivm_slot_table_t *table,
					   ivm_vmstate_t *state);

#define SET_SLOT(e0, e1, e2) \
	{                                                                            \
		ivm_hash_val_t hash;                                                     \
		ivm_size_t osize;                                                        \
                                                                                 \
		register ivm_slot_t *i, *tmp, *end;                                      \
                                                                                 \
		IVM_WBSLOT(state, table, obj);                                           \
		e0;                                                                      \
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
SET_SLOT(0, 0, 0);

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
	if (ivm_slot_table_checkCacheValid(table, instr)) {
		//IVM_TRACE("set: %s to cache %p(in %p size: %d?)\n",
		//		  ivm_string_trimHead(ivm_slot_table_getCacheSlot(state, instr)->k),
		//		  ivm_slot_table_getCacheSlot(state, instr),
		//		  table->tabl, table->size);
		ivm_slot_table_setCacheSlot(state, instr, obj);
		return;
	},
	ivm_instr_setCache(instr, table->uid, (ivm_ptr_t)i),
	ivm_instr_setCache(instr, table->uid, (ivm_ptr_t)tmp)
);

#define GET_SLOT(e0, e1, e2) \
	{                                                                                   \
		ivm_hash_val_t hash;                                                            \
		register ivm_slot_t *i, *tmp, *end;                                             \
                                                                                        \
		e0;                                                                             \
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
GET_SLOT(0, 0, 0);

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

	IVM_TRACE("heyo table: %p, core: %p\n", table, table->tabl);
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
						  struct ivm_vmstate_t_tag *state,
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
		return ivm_slot_table_getCacheSlot(state, instr);
	},
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
		IVM_WBSLOT(state, table, obj);
		_ivm_slot_setValue(slot, state, obj);
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
		IVM_WBSLOT(state, table, obj);
		_ivm_slot_setValue(slot, state, obj);
		return IVM_TRUE;
	}

	return IVM_FALSE;
}

#endif
