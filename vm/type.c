#include "pub/type.h"
#include "pub/com.h"
#include "pub/err.h"

#include "type.h"
#include "obj.h"

ivm_type_t *
ivm_type_new(ivm_type_t type)
{
	ivm_type_t *ret = STD_ALLOC(sizeof(*ret));

	IVM_MEMCHECK(ret);

	ivm_type_init(ret, &type);

	return ret;
}

void
ivm_type_free(ivm_type_t *type)
{
/*
	ivm_int_t i;

	if (type) {
		for (i = 0; i < IVM_BINOP_COUNT; i++) {
			ivm_binop_table_dump(type->binops + i);
		}

		STD_FREE(type);
	}
*/

	return;
}

void
ivm_type_init(ivm_type_t *type, ivm_type_t *src)
{
	// ivm_binop_table_t *i, *end;

	STD_MEMCPY(type, src, sizeof(*type));

	type->header.type = type;
	type->header.proto = IVM_NULL;
	type->header.slots = IVM_NULL;
	type->header.mark.copy = IVM_NULL;

	STD_INIT(type->def_oops, sizeof(type->def_oops));

	ivm_binop_table_init(type->uniops);
	ivm_uniop_table_init(type->uniops);

	return;
}

void
ivm_type_dump(ivm_type_t *type)
{
/*
	ivm_int_t i;
	if (type) {
		for (i = 0; i < IVM_BINOP_COUNT; i++) {
			ivm_binop_table_dump(type->binops + i);
		}
	}
*/
	return;
}
