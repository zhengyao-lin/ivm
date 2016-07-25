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
	return ivm_numeric_new(state, ivm_list_getSize(IVM_AS(arg.base, ivm_list_object_t)));
}
