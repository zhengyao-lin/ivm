#include "pub/const.h"
#include "pub/type.h"
#include "pub/vm.h"
#include "pub/mem.h"
#include "pub/inlines.h"

#include "listobj.h"

ivm_object_t *
ivm_list_object_new(ivm_vmstate_t *state,
					ivm_size_t size)
{
	ivm_list_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), state, IVM_LIST_OBJECT_T);

	ret->alloc = IVM_DEFAULT_LIST_OBJECT_BUFFER_SIZE;
	ret->size = size;

	ret->lst = ivm_vmstate_allocWild(
		state,
		sizeof(*ret->lst) * IVM_DEFAULT_LIST_OBJECT_BUFFER_SIZE
	);

	MEM_INIT(ret->lst, sizeof(*ret->lst) * size);

	ivm_vmstate_addDesLog(state, IVM_AS_OBJ(ret));

	return IVM_AS_OBJ(ret);
}

ivm_object_t *
ivm_list_object_new_c(ivm_vmstate_t *state,
					  ivm_object_t **init,
					  ivm_size_t count)
{
	ivm_list_object_t *ret;
	ivm_object_t **i, **end;
	
	if (!count) return ivm_list_object_new(state, 0);

	ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), state, IVM_LIST_OBJECT_T);

	ret->alloc = ret->size = count;
	ret->lst = ivm_vmstate_allocWild(
		state,
		sizeof(*ret->lst) * count
	);

	MEM_COPY(ret->lst, init, sizeof(*ret->lst) * count);

	ivm_vmstate_addDesLog(state, IVM_AS_OBJ(ret));

	for (i = ret->lst, end = i + count;
		 i != end || IVM_OBJECT_GET(ret, WB); i++) {
		if (*i)
			IVM_WBOBJ(state, IVM_AS_OBJ(ret), *i);
	}

	return IVM_AS_OBJ(ret);
}

IVM_PRIVATE
IVM_INLINE
ivm_object_t *
_ivm_list_object_new_nc(ivm_vmstate_t *state,
						ivm_object_t **init,
						ivm_size_t size)
{
	ivm_list_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), state, IVM_LIST_OBJECT_T);

	ret->alloc = ret->size = size;
	ret->lst = init;

	ivm_vmstate_addDesLog(state, IVM_AS_OBJ(ret));

	return IVM_AS_OBJ(ret);
}

/* assert size > osize */
IVM_PRIVATE
IVM_INLINE
void
_ivm_list_object_expandTo(ivm_list_object_t *list,
						  ivm_vmstate_t *state,
						  ivm_size_t size)
{
	ivm_object_t **olst;
	ivm_size_t osize = list->size;

	if (size <= list->alloc) {
		list->size = size;
	} else {
		list->alloc = size << 1;
		list->size = size;
		olst = list->lst;
		list->lst = ivm_vmstate_reallocWild(state, list->lst, sizeof(*olst) * list->alloc);
	}

	MEM_INIT(list->lst + osize, sizeof(*list->lst) * (size - osize));

	return;
}

IVM_PRIVATE
IVM_INLINE
void
_ivm_list_object_expand(ivm_list_object_t *list,
						ivm_vmstate_t *state)
{
	list->alloc <<= 1;
	list->lst = ivm_vmstate_reallocWild(
		state, list->lst,
		sizeof(*list->lst) * list->alloc
	);

	return;
}

ivm_size_t
ivm_list_object_push(ivm_list_object_t *list,
					 ivm_vmstate_t *state,
					 ivm_object_t *obj)
{
	if (list->size + 1 >= list->alloc) {
		_ivm_list_object_expand(list, state);
	}

	IVM_WBOBJ(state, IVM_AS_OBJ(list), obj);

	list->lst[list->size++] = obj;

	return list->size;
}

ivm_object_t *
ivm_list_object_set(ivm_list_object_t *list,
					ivm_vmstate_t *state,
					ivm_long_t i,
					ivm_object_t *obj)
{
	i = ivm_list_object_realIndex(list, i);

	if (i >= list->size) {
		_ivm_list_object_expandTo(list, state, i + 1);
	}

	IVM_WBOBJ(state, IVM_AS_OBJ(list), obj);

	return list->lst[i] = obj;
}

ivm_object_t *
ivm_list_object_link(ivm_list_object_t *list1,
					 ivm_list_object_t *list2,
					 ivm_vmstate_t *state)
{
	ivm_size_t size = list1->size + list2->size;
	ivm_object_t **nlist = ivm_vmstate_allocWild(
		state,
		sizeof(*nlist) * size
	);

	ivm_object_t *ret;

	MEM_COPY(nlist, list1->lst, sizeof(*nlist) * list1->size);
	MEM_COPY(nlist + list1->size, list2->lst, sizeof(*nlist) * list2->size);

	ret = _ivm_list_object_new_nc(state, nlist, size);

	IVM_WBOBJ(state, ret, IVM_AS_OBJ(list1)) ||
	IVM_WBOBJ(state, ret, IVM_AS_OBJ(list2));

	return ret;
}

void
ivm_list_object_reverse(ivm_list_object_t *list)
{
	ivm_object_t *tmp, **lst = list->lst;
	ivm_size_t i, j, mid = list->size / 2;

	for (i = 0, j = list->size - 1;
		 i <= mid; i++, j--) {
		tmp = lst[i];
		lst[i] = lst[j];
		lst[j] = tmp;
	}

	return;
}

void
ivm_list_object_step(ivm_list_object_t *list,
					 ivm_long_t step)
{
	ivm_object_t **lst = list->lst;
	ivm_long_t i, j, size = list->size;

	IVM_ASSERT(step >= 1, IVM_ERROR_MSG_ILLEGAL_STEP);

	if (step == 1) return;

	for (i = 0, j = 0; j < size; i++, j += step) {
		lst[i] = lst[j];
	}

	list->size = i;

	return;
}

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

void
ivm_list_object_multiply(ivm_list_object_t *list,
						 ivm_vmstate_t *state,
						 ivm_size_t times)
{
	ivm_size_t osize = list->size, esize;
	ivm_object_t **lst, **cur;

	_ivm_list_object_expandTo(list, state, osize * times);

	cur = lst = list->lst;
	esize = sizeof(*lst) * osize;

	while (times--) {
		MEM_COPY(cur += osize, lst, esize);
	}

	return;
}

void
ivm_list_object_destructor(ivm_object_t *obj,
						   ivm_vmstate_t *state)
{
	MEM_FREE(IVM_AS(obj, ivm_list_object_t)->lst);
	return;
}

void
ivm_list_object_cloner(ivm_object_t *obj,
					   ivm_vmstate_t *state)
{
	ivm_list_object_t *list = IVM_AS(obj, ivm_list_object_t);
	ivm_size_t size = sizeof(*list->lst) * list->alloc;
	ivm_object_t **nlst = ivm_vmstate_allocWild(state, size);

	MEM_COPY(nlst, list->lst, size);

	list->lst = nlst;

	ivm_vmstate_addDesLog(state, obj);

	return;
}

void
ivm_list_object_traverser(ivm_object_t *obj,
						  ivm_traverser_arg_t *arg)
{
	ivm_list_object_t *list = IVM_AS(obj, ivm_list_object_t);
	ivm_object_t **i, **end;

	for (i = list->lst,
		 end = i + list->size;
		 i != end; i++) {
		ivm_collector_copyObject_p(*i, arg, i);
	}

	return;
}

ivm_object_t *
ivm_list_object_iter_new(ivm_vmstate_t *state,
						 ivm_list_object_t *list)
{
	ivm_list_object_iter_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), state, IVM_LIST_OBJECT_ITER_T);

	ret->list = list;
	ret->cur = 0;

	return IVM_AS_OBJ(ret);
}

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

void
ivm_list_object_iter_traverser(ivm_object_t *obj,
							   ivm_traverser_arg_t *arg)
{
	ivm_list_object_iter_t *iter = IVM_AS(obj, ivm_list_object_iter_t);

	iter->list = IVM_AS(ivm_collector_copyObject(IVM_AS_OBJ(iter->list), arg), ivm_list_object_t);

	return;
}
