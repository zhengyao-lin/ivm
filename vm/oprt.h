#ifndef _IVM_VM_OPRT_H_
#define _IVM_VM_OPRT_H_

#include "pub/com.h"
#include "pub/const.h"

#include "std/list.h"

IVM_COM_HEADER

struct ivm_type_t_tag;
struct ivm_object_t_tag;
struct ivm_vmstate_t_tag;

typedef struct ivm_object_t_tag *(*ivm_oprt_binary_t)(struct ivm_vmstate_t_tag *state,
													  struct ivm_object_t_tag *opr1,
													  struct ivm_object_t_tag *opr2);

typedef struct {
	ivm_size_t alloc;
	ivm_size_t size;
	ivm_oprt_binary_t *lst;
} ivm_oprt_binary_table_t;

ivm_oprt_binary_table_t *
ivm_oprt_binary_table_new();

void
ivm_oprt_binary_table_free(ivm_oprt_binary_table_t *table);

#define ivm_oprt_binary_table_get(table, i) \
	((table) && (i) < (table)->size ? (table)->lst[i] : IVM_NULL)

IVM_INLINE
void
_ivm_oprt_binary_table_incTo(ivm_oprt_binary_table_t *table,
							 ivm_size_t size) /* size >= table->size */
{
	if (size > table->alloc) {
		table->lst = MEM_REALLOC(table->lst,
								 sizeof(ivm_oprt_binary_t) * size,
								 ivm_oprt_binary_t *);
		table->alloc = size;
	}

	MEM_INIT(table->lst + table->size,
			 sizeof(ivm_oprt_binary_t) * (size - table->size));

	table->size = size;

	return;
}

IVM_INLINE
void
ivm_oprt_binary_table_set(ivm_oprt_binary_table_t *table,
						  ivm_size_t i,
						  ivm_oprt_binary_t proc)
{
	if (i >= table->size) {
		_ivm_oprt_binary_table_incTo(table, i + 1);
	}

	table->lst[i] = proc;

	return;
}

void
ivm_oprt_initType(struct ivm_type_t_tag *type,
				  struct ivm_vmstate_t_tag *state);

IVM_COM_END

#endif
