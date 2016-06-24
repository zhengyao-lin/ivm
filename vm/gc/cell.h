#ifndef _IVM_VM_GC_CELL_H_
#define _IVM_VM_GC_CELL_H_

#include "pub/com.h"

IVM_COM_HEADER

struct ivm_vmstate_t_tag;
struct ivm_cell_t_tag;
struct ivm_cell_set_t_tag;
struct ivm_object_t_tag;

typedef struct ivm_cell_t_tag {
	struct ivm_object_t_tag *obj;

	struct ivm_cell_t_tag *prev;
	struct ivm_cell_t_tag *next;
} ivm_cell_t;

typedef struct ivm_cell_set_t_tag {
	ivm_cell_t *head;
	ivm_cell_t *tail;
} ivm_cell_set_t;

#define IVM_CELL_GET_OBJ(cell) ((cell)->obj)
#define IVM_CELL_SET_OBJ(cell, o) ((cell)->obj = (o))

#define IVM_CELL_GET(obj, member) IVM_GET((obj), IVM_CELL, member)
#define IVM_CELL_SET(obj, member, val) IVM_SET((obj), IVM_CELL, member, (val))

/* cell */
ivm_cell_t *
ivm_cell_new();

/* free cell: just dispose the container */
void
ivm_cell_free(ivm_cell_t *cell);
/* destruct cell: call the destructor of the object and free the cell */
void
ivm_cell_destruct(ivm_cell_t *cell, struct ivm_vmstate_t_tag *state);
#define ivm_cell_init(cell, o) ((cell)->next = (cell)->prev = IVM_NULL, (cell)->obj = (o))

/* notice: use ivm_cell_move_to_set() to move cell BETWEEN SETS
 * because there will be a problem in deleting origin log if the set
 * only has one cell
 */

void
ivm_cell_moveBetween(ivm_cell_t *cell, ivm_cell_t *prev, ivm_cell_t *next);
#define ivm_cell_isolate(cell) (ivm_cell_moveBetween((cell), IVM_NULL, IVM_NULL))
ivm_cell_t *
ivm_cell_moveToSet(ivm_cell_t *cell, ivm_cell_set_t *from, ivm_cell_set_t *to);
#define ivm_cell_removeFrom(cell, set) (ivm_cell_moveToSet((cell), (set), IVM_NULL), ivm_cell_isolate(cell))

/* cell set */
ivm_cell_set_t *
ivm_cell_set_new();

/* similar to the interface of free/destruct cell */
void
ivm_cell_set_free(ivm_cell_set_t *set);
void
ivm_cell_set_destruct(ivm_cell_set_t *set, struct ivm_vmstate_t_tag *state);

void
ivm_cell_set_addCell(ivm_cell_set_t *set, ivm_cell_t *cell);
ivm_cell_t *
ivm_cell_set_addObject(ivm_cell_set_t *set, struct ivm_object_t_tag *obj);
/* just remove, no free */
ivm_cell_t *
ivm_cell_set_removeTail(ivm_cell_set_t *set);

IVM_COM_END

#endif
