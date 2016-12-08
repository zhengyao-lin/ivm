#ifndef _IVM_VM_INLINE_LISTOBJ_H_
#define _IVM_VM_INLINE_LISTOBJ_H_

#include "pub/com.h"
#include "pub/vm.h"

#include "vm/listobj.h"
#include "vm/obj.h"

IVM_COM_HEADER

IVM_INLINE
ivm_object_t *
ivm_list_object_new(ivm_vmstate_t *state,
					ivm_size_t size)
{
	ivm_list_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), IVM_BTTYPE(state, IVM_LIST_OBJECT_T));

	ret->alloc = IVM_DEFAULT_LIST_OBJECT_BUFFER_SIZE;
	ret->size = size;

	ret->lst = ivm_vmstate_allocWild(
		state,
		sizeof(*ret->lst) * IVM_DEFAULT_LIST_OBJECT_BUFFER_SIZE
	);

	STD_INIT(ret->lst, sizeof(*ret->lst) * size);

	ivm_vmstate_addDesLog(state, IVM_AS_OBJ(ret));

	return IVM_AS_OBJ(ret);
}

IVM_INLINE
ivm_object_t *
ivm_list_object_new_b(ivm_vmstate_t *state,
					  ivm_size_t buf)
{
	ivm_list_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), IVM_BTTYPE(state, IVM_LIST_OBJECT_T));

	ret->alloc = buf < IVM_DEFAULT_LIST_OBJECT_BUFFER_SIZE ? IVM_DEFAULT_LIST_OBJECT_BUFFER_SIZE : buf;
	ret->size = 0;

	ret->lst = ivm_vmstate_allocWild(
		state,
		sizeof(*ret->lst) * ret->alloc
	);

	// STD_INIT(ret->lst, sizeof(*ret->lst) * ret->alloc);

	ivm_vmstate_addDesLog(state, IVM_AS_OBJ(ret));

	return IVM_AS_OBJ(ret);
}

IVM_INLINE
ivm_object_t *
ivm_list_object_new_c(ivm_vmstate_t *state,
					  ivm_object_t **init,
					  ivm_size_t count)
{
	ivm_list_object_t *ret;
	ivm_object_t **i, **end;
	
	if (!count) return ivm_list_object_new(state, 0);

	ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), IVM_BTTYPE(state, IVM_LIST_OBJECT_T));

	ret->alloc = ret->size = count;
	ret->lst = ivm_vmstate_allocWild(
		state,
		sizeof(*ret->lst) * count
	);

	STD_MEMCPY(ret->lst, init, sizeof(*ret->lst) * count);

	ivm_vmstate_addDesLog(state, IVM_AS_OBJ(ret));

	for (i = ret->lst, end = i + count;
		 i != end || IVM_OBJECT_GET(ret, WB); i++) {
		if (*i)
			IVM_WBOBJ(state, IVM_AS_OBJ(ret), *i);
	}

	return IVM_AS_OBJ(ret);
}

IVM_INLINE
ivm_object_t *
_ivm_list_object_new_nc(ivm_vmstate_t *state,
						ivm_object_t **init,
						ivm_size_t size)
{
	ivm_list_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), IVM_BTTYPE(state, IVM_LIST_OBJECT_T));

	ret->alloc = ret->size = size;
	ret->lst = init;

	ivm_vmstate_addDesLog(state, IVM_AS_OBJ(ret));

	return IVM_AS_OBJ(ret);
}

IVM_INLINE
ivm_bool_t
_ivm_list_object_expand(ivm_list_object_t *list,
						ivm_vmstate_t *state)
{
	ivm_object_t **olst = list->lst;

	list->alloc <<= 1;
	list->lst = ivm_vmstate_reallocWild(
		state, list->lst,
		sizeof(*list->lst) * list->alloc
	);

	if (!list->lst) {
		list->alloc >>= 1;
		list->lst = olst;
		return IVM_FALSE;
	}

	return IVM_TRUE;
}

/* return 0 if error */
IVM_INLINE
ivm_size_t
ivm_list_object_push(ivm_list_object_t *list,
					 ivm_vmstate_t *state,
					 ivm_object_t *obj)
{
	if (list->size + 1 >= list->alloc) {
		if (!_ivm_list_object_expand(list, state)) {
			return 0;
		}
	}

	IVM_WBOBJ(state, IVM_AS_OBJ(list), obj);

	list->lst[list->size++] = obj;

	return list->size;
}

IVM_INLINE
ivm_object_t *
ivm_list_object_pop(ivm_list_object_t *list)
{
	if (!list->size) {
		return IVM_NULL;
	}

	return list->lst[--list->size];
}

/* assert size > osize */
IVM_INLINE
ivm_bool_t
_ivm_list_object_expandTo(ivm_list_object_t *list,
						  ivm_vmstate_t *state,
						  ivm_size_t size)
{
	ivm_object_t **olst;
	ivm_size_t osize = list->size, oalloc;

	// IVM_TRACE("size: %ld\n", size);

	if (size <= list->size) {
		list->size = size;
		return IVM_TRUE;
	} else if (size <= list->alloc) {
		list->size = size;
	} else {
		oalloc = list->alloc;
		list->alloc = size << 1;

		list->size = size;

		olst = list->lst;
		list->lst = ivm_vmstate_reallocWild(state, list->lst, sizeof(*olst) * list->alloc);

		if (!list->lst) {
			list->alloc = oalloc;
			list->size = osize;
			list->lst = olst;

			return IVM_FALSE;
		}
	}

	// IVM_TRACE("%ld %ld %ld\n", osize, size, sizeof(*list->lst) * (size - osize));

	STD_INIT(list->lst + osize, sizeof(*list->lst) * (size - osize));

	return IVM_TRUE;
}

/* return NULL if error */
/* use ivm_list_realIndex to obtain the real index first */
IVM_INLINE
ivm_object_t *
ivm_list_object_set(ivm_list_object_t *list,
					ivm_vmstate_t *state,
					ivm_size_t i,
					ivm_object_t *obj)
{
	if (i >= list->size) {
		if (!_ivm_list_object_expandTo(list, state, i + 1)) {
			return IVM_NULL;
		}
	}

	if (obj) {
		IVM_WBOBJ(state, IVM_AS_OBJ(list), obj);
	}

	return (list->lst[i] = obj) ? obj : IVM_NONE(state);
}

/* return false if error */
IVM_INLINE
ivm_bool_t
ivm_list_object_multiply(ivm_list_object_t *list,
						 ivm_vmstate_t *state,
						 ivm_size_t times)
{
	ivm_size_t osize = list->size, esize;
	ivm_object_t **lst, **cur;

	if (!times) {
		list->size = 0;
		return IVM_TRUE;
	}

	if (!_ivm_list_object_expandTo(list, state, osize * times)) {
		return IVM_FALSE;
	}

	cur = lst = list->lst;
	esize = sizeof(*lst) * osize;

	while (--times) {
		STD_MEMCPY(cur += osize, lst, esize);
	}

	return IVM_TRUE;
}

IVM_INLINE
void
ivm_list_object_reverse(ivm_list_object_t *list)
{
	ivm_vmstack_reverse(list->lst, list->size);
	return;
}

IVM_INLINE
void
_ivm_list_object_unpackTo(ivm_list_object_t *list,
						  ivm_vmstate_t *state,
						  ivm_object_t **sp,
						  ivm_size_t req)
{
	ivm_object_t **lst = list->lst;
	ivm_size_t i, j, size = list->size;

	j = req - 1;

	for (i = 0; i != req && i != size; i++) {
		sp[j--] = lst[i] ? lst[i] : IVM_NONE(state);
	}

	for (; i != req; i++) {
		sp[j--] = IVM_NONE(state);
	}

	return;
}

IVM_INLINE
void
_ivm_list_object_unpackAll(ivm_list_object_t *list,
						   struct ivm_vmstate_t_tag *state,
						   ivm_object_t **sp)
{
	ivm_object_t **end, **cur;

	for (end = list->lst - 1, cur = end + list->size;
		 cur != end; cur--) {
		*sp++ = *cur ? *cur : IVM_NONE(state);
	}

	return;
}

IVM_INLINE
void
_ivm_list_object_unpackAll_r(ivm_list_object_t *list,
							 struct ivm_vmstate_t_tag *state,
							 ivm_object_t **sp)
{
	ivm_object_t **end, **cur;

	for (cur = list->lst, end = cur + list->size;
		 cur != end; cur++) {
		*sp++ = *cur ? *cur : IVM_NONE(state);
	}

	return;
}

IVM_INLINE
ivm_object_t *
ivm_list_object_iter_new(ivm_vmstate_t *state,
						 ivm_list_object_t *list)
{
	ivm_list_object_iter_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), IVM_BTTYPE(state, IVM_LIST_OBJECT_ITER_T));

	ret->list = list;
	ret->cur = 0;

	return IVM_AS_OBJ(ret);
}

IVM_INLINE
ivm_object_t *
ivm_list_object_iter_next(ivm_list_object_iter_t *iter,
						  ivm_vmstate_t *state)
{
	ivm_object_t *ret;

	if (iter->cur >= ivm_list_object_getSize(iter->list)) {
		return IVM_NULL;
	}

	ret = _ivm_list_object_get_c(iter->list, iter->cur++);

	return ret ? ret : IVM_NONE(state);
}

IVM_COM_END

#endif
