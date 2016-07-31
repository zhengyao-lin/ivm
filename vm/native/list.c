#include "pub/type.h"
#include "pub/const.h"
#include "pub/inlines.h"
#include "pub/vm.h"

#include "priv.h"
#include "list.h"

#include "vm/listobj.h"
#include "vm/num.h"

IVM_NATIVE_FUNC(_list_size)
{
	CHECK_BASE(IVM_LIST_OBJECT_T);
	return ivm_numeric_new(NAT_STATE(), ivm_list_object_getSize(IVM_AS(NAT_BASE(), ivm_list_object_t)));
}

IVM_NATIVE_FUNC(_list_slice)
{
	ivm_long_t start, end, size;
	ivm_object_t **lst;
	ivm_list_object_t *list;

	CHECK_BASE(IVM_LIST_OBJECT_T);
	CHECK_ARG_2(IVM_NUMERIC_T, IVM_NUMERIC_T);
	
	list = IVM_AS(NAT_BASE(), ivm_list_object_t);
	start = ivm_list_object_realIndex(list, ivm_numeric_getValue(NAT_ARG_AT(2)));
	end = ivm_list_object_realIndex(list, ivm_numeric_getValue(NAT_ARG_AT(1)));
	size = ivm_list_object_getSize(list);
	lst = ivm_list_object_core(list);

	if (start > size) start = size;
	if (end > size) end = size;

	if (start < end) {
		return ivm_list_object_new_c(NAT_STATE(), lst + start, end - start);
	}

	return ivm_list_object_new(NAT_STATE(), 0);
}
