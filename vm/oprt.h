#ifndef _IVM_VM_OPRT_H_
#define _IVM_VM_OPRT_H_

#include "pub/com.h"
#include "pub/const.h"
#include "pub/mem.h"
#include "pub/type.h"

#include "std/list.h"

IVM_COM_HEADER

struct ivm_type_t_tag;
struct ivm_object_t_tag;
struct ivm_vmstate_t_tag;

typedef struct ivm_object_t_tag *(*ivm_uniop_proc_t)(struct ivm_vmstate_t_tag *state,
													 struct ivm_object_t_tag *op1);

typedef struct ivm_object_t_tag *(*ivm_binop_proc_t)(struct ivm_vmstate_t_tag *state,
													 struct ivm_object_t_tag *op1,
													 struct ivm_object_t_tag *op2);

#define IVM_UNIOP_ID(op) IVM_UNIOP_##op
#define IVM_BINOP_ID(op) IVM_BINOP_##op

enum {
	IVM_UNIOP_ID(NOT) = 0,
	IVM_UNIOP_ID(CLONE),
	IVM_UNIOP_COUNT
};

enum {
	IVM_BINOP_ID(ADD) = 0,
	IVM_BINOP_ID(SUB),
	IVM_BINOP_ID(MUL),
	IVM_BINOP_ID(DIV),
	IVM_BINOP_ID(MOD),
	IVM_BINOP_ID(CMP),
	IVM_BINOP_COUNT
};

typedef ivm_uniop_proc_t ivm_uniop_table_t[IVM_UNIOP_COUNT];

#define ivm_uniop_table_init(table) (MEM_INIT((table), sizeof(ivm_uniop_table_t)))
#define ivm_uniop_table_get(table, i) ((table)[i])
#define ivm_uniop_table_set(table, i, proc) ((table)[i] = (proc))

typedef struct {
	ivm_size_t size;
	ivm_binop_proc_t *lst;
} ivm_binop_table_t;

ivm_binop_table_t *
ivm_binop_table_new();

void
ivm_binop_table_free(ivm_binop_table_t *table);

void
ivm_binop_table_init(ivm_binop_table_t *table);

void
ivm_binop_table_dump(ivm_binop_table_t *table);

IVM_INLINE
ivm_binop_proc_t
ivm_binop_table_get(ivm_binop_table_t *table,
					ivm_type_tag_t tag)
{
	if (tag < table->size)
		return table->lst[tag];
	return IVM_NULL;
}

IVM_INLINE
void
_ivm_binop_table_incTo(ivm_binop_table_t *table,
					   ivm_size_t size) /* size >= table->size */
{
	table->lst = MEM_REALLOC(table->lst,
							 sizeof(ivm_binop_proc_t) * size,
							 ivm_binop_proc_t *);
	
	MEM_INIT(table->lst + table->size,
			 sizeof(ivm_binop_proc_t) * (size - table->size));

	table->size = size;

	return;
}

IVM_INLINE
void
ivm_binop_table_set(ivm_binop_table_t *table,
					ivm_size_t i,
					ivm_binop_proc_t proc)
{
	if (i >= table->size) {
		_ivm_binop_table_incTo(table, i + 1);
	}

	table->lst[i] = proc;

	return;
}

void
ivm_oprt_initType(struct ivm_vmstate_t_tag *state);

IVM_COM_END

#endif
