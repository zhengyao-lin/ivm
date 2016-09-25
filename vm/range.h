#ifndef _IVM_VM_RANGE_H_
#define _IVM_VM_RANGE_H_

#include "pub/com.h"
#include "pub/const.h"
#include "pub/type.h"

#include "obj.h"

IVM_COM_HEADER

struct ivm_vmstate_t_tag;

typedef struct {
	IVM_OBJECT_HEADER
	ivm_long_t from;
	ivm_long_t to;
	ivm_long_t step;
} ivm_range_t;

ivm_object_t *
ivm_range_new(struct ivm_vmstate_t_tag *state,
			  ivm_long_t from, ivm_long_t to,
			  ivm_long_t step);

typedef struct {
	IVM_OBJECT_HEADER
	ivm_long_t cur;
	ivm_long_t end;
	ivm_long_t step;
} ivm_range_iter_t;

ivm_object_t *
ivm_range_iter_new(struct ivm_vmstate_t_tag *state,
				   ivm_long_t cur, ivm_long_t end,
				   ivm_long_t step);

ivm_object_t *
ivm_range_iter_next(ivm_range_iter_t *iter,
					struct ivm_vmstate_t_tag *state);

IVM_COM_END

#endif
