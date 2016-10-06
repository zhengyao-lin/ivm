#include "pub/const.h"
#include "pub/type.h"
#include "pub/vm.h"
#include "pub/inlines.h"

#include "std/mem.h"

#include "listobj.h"

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

	if (!nlist) {
		return IVM_NULL;
	}

	ivm_object_t *ret;

	STD_MEMCPY(nlist, list1->lst, sizeof(*nlist) * list1->size);
	STD_MEMCPY(nlist + list1->size, list2->lst, sizeof(*nlist) * list2->size);

	ret = _ivm_list_object_new_nc(state, nlist, size);

	IVM_WBOBJ(state, ret, IVM_AS_OBJ(list1)) ||
	IVM_WBOBJ(state, ret, IVM_AS_OBJ(list2));

	return ret;
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
ivm_list_object_destructor(ivm_object_t *obj,
						   ivm_vmstate_t *state)
{
	STD_FREE(IVM_AS(obj, ivm_list_object_t)->lst);
	return;
}

void
ivm_list_object_cloner(ivm_object_t *obj,
					   ivm_vmstate_t *state)
{
	ivm_list_object_t *list = IVM_AS(obj, ivm_list_object_t);
	ivm_size_t size = sizeof(*list->lst) * list->alloc;
	ivm_object_t **nlst = ivm_vmstate_allocWild(state, size);

	STD_MEMCPY(nlst, list->lst, size);

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

void
ivm_list_object_iter_traverser(ivm_object_t *obj,
							   ivm_traverser_arg_t *arg)
{
	ivm_list_object_iter_t *iter = IVM_AS(obj, ivm_list_object_iter_t);

	iter->list = IVM_AS(ivm_collector_copyObject(IVM_AS_OBJ(iter->list), arg), ivm_list_object_t);

	return;
}
