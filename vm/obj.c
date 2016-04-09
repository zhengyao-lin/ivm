#include "pub/mem.h"
#include "obj.h"
#include "err.h"

ivm_object_t *ivm_new_obj()
{
	ivm_object_t *ret = MEM_ALLOC_INIT(sizeof(*ret));

	IVM_ASSERT(ret, "Failed to allocate new room for new object");

	ret->type = IVM_OBJECT_T;
	ret->slots = IVM_NULL;
	ret->des = IVM_NULL;

	return ret;
}
