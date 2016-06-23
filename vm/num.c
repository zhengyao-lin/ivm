#include "pub/mem.h"
#include "pub/err.h"
#include "pub/vm.h"
#include "pub/inlines.h"

#include "num.h"
#include "obj.h"

ivm_bool_t
ivm_numeric_isTrue(ivm_object_t *obj,
				   ivm_vmstate_t *state)
{
	return IVM_AS(obj, ivm_numeric_t)->val != 0;
}
