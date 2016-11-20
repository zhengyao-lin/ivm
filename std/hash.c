#include "pub/com.h"
#include "pub/err.h"

#include "mem.h"
#include "hash.h"

void
ivm_ptset_init(ivm_ptset_t *set)
{
	set->size = IVM_DEFAULT_PTSET_BUFFER_SIZE;
	set->table = STD_ALLOC_INIT(sizeof(*set->table) * IVM_DEFAULT_PTSET_BUFFER_SIZE);

	IVM_MEMCHECK(set->table);

	return;
}

void
ivm_ptset_dump(ivm_ptset_t *set)
{
	if (set) {
		STD_FREE(set->table);
	}

	return;
}


