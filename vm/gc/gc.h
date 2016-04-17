#ifndef _IVM_VM_GC_GC_H_
#define _IVM_VM_GC_GC_H_

#include "cell.h"

typedef struct {
	ivm_cell_set_t *all_set;
	ivm_cell_set_t *grey_set;
} ivm_collector_t;

#define IVM_MARK_WHITE 0
#define IVM_MARK_GREY 1
#define IVM_MARK_BLACK 2

#endif
