#ifndef _IVM_VM_GC_CELL_H_
#define _IVM_VM_GC_CELL_H_

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

/* cell */
ivm_cell_t *
ivm_cell_new();

/* free cell: just dispose the container */
void
ivm_cell_free(ivm_cell_t *cell);
/* dispose cell: dispose both the container and the object it contains */
void
ivm_cell_dispose(ivm_cell_t *cell, struct ivm_vmstate_t_tag *state);
/* dump cell: dump the data the object contains, but not free it */
void
ivm_cell_dump(ivm_cell_t *cell, struct ivm_vmstate_t_tag *state);

/* notice: use ivm_cell_move_to_set() to move cell BETWEEN SETS
 * because there will be a problem in deleting origin log if the set
 * only has one cell
 */
void
ivm_cell_moveBetween(ivm_cell_t *cell, ivm_cell_t *prev, ivm_cell_t *next);
ivm_cell_t *
ivm_cell_moveToSet(ivm_cell_t *cell, ivm_cell_set_t *from, ivm_cell_set_t *to);

/* cell set */
ivm_cell_set_t *
ivm_cell_set_new();

/* similar to the interface of free/dispose cell */
void
ivm_cell_set_free(ivm_cell_set_t *set);
void
ivm_cell_set_dispose(ivm_cell_set_t *set, struct ivm_vmstate_t_tag *state);
void
ivm_cell_set_dump(ivm_cell_set_t *set, struct ivm_vmstate_t_tag *state);

void
ivm_cell_set_addCell(ivm_cell_set_t *set, ivm_cell_t *cell);
ivm_cell_t *
ivm_cell_set_addObject(ivm_cell_set_t *set, ivm_object_t *obj);
/* just remove, no free */
ivm_cell_t *
ivm_cell_set_removeTail(ivm_cell_set_t *set);

#endif
