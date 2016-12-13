#include "pub/com.h"
#include "pub/err.h"

#include "mem.h"
#include "hash.h"

void
ivm_pthash_init(ivm_pthash_t *set)
{
	set->size = IVM_DEFAULT_PTHASH_BUFFER_SIZE;
	set->table = STD_ALLOC_INIT(sizeof(*set->table) * IVM_DEFAULT_PTHASH_BUFFER_SIZE);

	IVM_MEMCHECK(set->table);

	return;
}

void
ivm_pthash_dump(ivm_pthash_t *set)
{
	if (set) {
		STD_FREE(set->table);
	}

	return;
}


