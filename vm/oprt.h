#ifndef _IVM_VM_OPRT_H_
#define _IVM_VM_OPRT_H_

#include "pub/com.h"
#include "pub/const.h"
#include "pub/type.h"

#include "std/mem.h"
#include "std/list.h"

IVM_COM_HEADER

struct ivm_type_t_tag;
struct ivm_object_t_tag;
struct ivm_vmstate_t_tag;
struct ivm_coro_t_tag;

typedef struct ivm_object_t_tag *(*ivm_uniop_proc_t)(struct ivm_vmstate_t_tag *state,
													 struct ivm_object_t_tag *op1);

typedef struct ivm_object_t_tag *(*ivm_binop_proc_t)(struct ivm_vmstate_t_tag *state,
													 struct ivm_object_t_tag *op1,
													 struct ivm_object_t_tag *op2);

typedef struct ivm_object_t_tag *(*ivm_triop_proc_t)(struct ivm_vmstate_t_tag *state,
													 struct ivm_object_t_tag *op1,
													 struct ivm_object_t_tag *op2,
													 struct ivm_object_t_tag *op3);

#define IVM_UNIOP_ID(op) IVM_UNIOP_##op
#define IVM_BINOP_ID(op) IVM_BINOP_##op

// overload op
#define IVM_OOP_ID(op) IVM_OOP_##op
#define IVM_OOP_CMP_ID(op) IVM_OOP_CMP_##op

enum {
	IVM_UNIOP_ID(NOT) = 0,
	IVM_UNIOP_ID(NEG),
	IVM_UNIOP_ID(POS),
	IVM_UNIOP_ID(DEL),

	IVM_UNIOP_COUNT
};

enum {
	IVM_BINOP_ID(ADD) = 0,
	IVM_BINOP_ID(SUB),
	IVM_BINOP_ID(MUL),
	IVM_BINOP_ID(DIV),
	IVM_BINOP_ID(MOD),
	
	IVM_BINOP_ID(NE),
	IVM_BINOP_ID(EQ),
	IVM_BINOP_ID(GT),
	IVM_BINOP_ID(GE),
	IVM_BINOP_ID(LT),
	IVM_BINOP_ID(LE),

	IVM_BINOP_ID(AND),
	IVM_BINOP_ID(IOR), // inclusive or
	IVM_BINOP_ID(EOR), // exclusive or
	IVM_BINOP_ID(IDX),
	IVM_BINOP_ID(IDXA),

	IVM_BINOP_ID(SHL),
	IVM_BINOP_ID(SHAR), // arithmetic
	IVM_BINOP_ID(SHLR), // logic

	IVM_BINOP_COUNT
};

enum {
	IVM_OOP_ID(NOT) = 0,
	IVM_OOP_ID(NEG),
	IVM_OOP_ID(POS),

	IVM_OOP_UNIOP_COUNT,

	IVM_OOP_ID(ADD) = IVM_OOP_UNIOP_COUNT,
	IVM_OOP_ID(SUB),
	IVM_OOP_ID(MUL),
	IVM_OOP_ID(DIV),
	IVM_OOP_ID(MOD),

	IVM_OOP_ID(NE),
	IVM_OOP_ID(EQ),
	IVM_OOP_ID(GT),
	IVM_OOP_ID(GE),
	IVM_OOP_ID(LT),
	IVM_OOP_ID(LE),

	IVM_OOP_ID(AND),
	IVM_OOP_ID(IOR),
	IVM_OOP_ID(EOR),
	IVM_OOP_ID(IDX),

	IVM_OOP_ID(SHL),
	IVM_OOP_ID(SHLR),
	IVM_OOP_ID(SHAR),

	IVM_OOP_ID(CALL),

	IVM_OOP_COUNT
};

/*
	support overload op(max 14):
		unary: not

		binary:
			add, sub, mul, div,
			mod, and, ior, eor,
			idx, shl, shar, shlr
 */

typedef ivm_uniop_proc_t ivm_uniop_table_t[IVM_UNIOP_COUNT];

#define ivm_uniop_table_init(table) (STD_INIT((table), sizeof(ivm_uniop_table_t)))
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
	table->lst = STD_REALLOC(table->lst, sizeof(ivm_binop_proc_t) * size);
	
	STD_INIT(table->lst + table->size,
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
