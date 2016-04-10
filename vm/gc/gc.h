#ifndef _IVM_VM_GC_GC_H_
#define _IVM_VM_GC_GC_H_

#include "../obj.h"

struct ivm_vmstate_t_tag;

typedef struct ivm_cell_t_tag {
	ivm_object_t *obj;

	struct ivm_cell_t_tag *prev;
	struct ivm_cell_t_tag *next;
} ivm_cell_t;

typedef struct {
	ivm_cell_t *head;
	ivm_cell_t *tail;
} ivm_cell_set_t;

typedef struct {
	ivm_cell_set_t *white;
	ivm_cell_set_t *grey;
	ivm_cell_set_t *black;
} ivm_heap_t;

/* cell */
ivm_cell_t *
ivm_new_cell();

/* free cell: just dispose the container */
void
ivm_free_cell(ivm_cell_t *cell);
/* dispose cell: dispose both the container and the object it contains */
void
ivm_dispose_cell(struct ivm_vmstate_t_tag *state, ivm_cell_t *cell);

/* notice: use ivm_cell_move_to_set() to move cell BETWEEN SETS
 * because there will be a problem in deleting origin log if the set
 * only has one cell
 */
void
ivm_cell_move_between(ivm_cell_t *cell, ivm_cell_t *prev, ivm_cell_t *next);
void
ivm_cell_move_to_set(ivm_cell_t *cell, ivm_cell_set_t *from, ivm_cell_set_t *to);

/* cell set */
ivm_cell_set_t *
ivm_new_cell_set();

/* similar to the interface of free/dispose cell */
void
ivm_free_cell_set(ivm_cell_set_t *set);
void
ivm_dispose_cell_set(struct ivm_vmstate_t_tag *state, ivm_cell_set_t *set);

void
ivm_cell_set_add_cell(ivm_cell_set_t *set, ivm_cell_t *cell);

#endif
