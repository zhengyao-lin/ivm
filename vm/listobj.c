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
	ret->lst = MEM_ALLOC(sizeof(*ret->lst) * IVM_DEFAULT_LIST_OBJECT_BUFFER_SIZE,
						 ivm_object_t **);

	if (size) {
		MEM_INIT(ret->lst, sizeof(*ret->lst) * size);
	}

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
	ret->lst = MEM_ALLOC(sizeof(*ret->lst) * count,
						 ivm_object_t **);

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
		list->lst = MEM_REALLOC(list->lst, sizeof(*olst) * list->alloc,
								ivm_object_t **);
	}

	MEM_INIT(list->lst + osize, sizeof(*list->lst) * (size - osize));

	return;
}

IVM_PRIVATE
IVM_INLINE
ivm_long_t
_ivm_list_object_realIndex(ivm_list_object_t *list,
						   ivm_long_t i)
{
	if (i < 0) {
		i = -i % list->size;

		if (i) {
			i = list->size - i;
		}
	}

	return i;
}

ivm_object_t *
ivm_list_object_set(ivm_list_object_t *list,
					ivm_vmstate_t *state,
					ivm_long_t i,
					ivm_object_t *obj)
{
	i = _ivm_list_object_realIndex(list, i);

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
	ivm_object_t **nlist = MEM_ALLOC(sizeof(*nlist) * size,
									 ivm_object_t **);
	ivm_object_t *ret;

	MEM_COPY(nlist, list1->lst, sizeof(*nlist) * list1->size);
	MEM_COPY(nlist + list1->size, list2->lst, sizeof(*nlist) * list2->size);

	ret = _ivm_list_object_new_nc(state, nlist, size);

	IVM_WBOBJ(state, ret, IVM_AS_OBJ(list1)) ||
	IVM_WBOBJ(state, ret, IVM_AS_OBJ(list2));

	return ret;
}

void
ivm_list_object_destructor(ivm_object_t *obj,
						   ivm_vmstate_t *state)
{
	MEM_FREE(IVM_AS(obj, ivm_list_object_t)->lst);
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
