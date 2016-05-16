#include <stdio.h>
#include <string.h>
#include "pub/com.h"
#include "dbg.h"
#include "op.h"
#include "byte.h"
#include "exec.h"
#include "vm.h"
#include "obj.h"
#include "gc/heap.h"

#define STRING_BUF_SIZE 128

void
ivm_dbg_disAsmExec(ivm_exec_t *exec,
				   const char *prefix,
				   FILE *fp)
{
	ivm_pc_t pc;
	ivm_opcode_t op;
	ivm_size_t end, i,
			   len = ivm_exec_length(exec);
	ivm_sint32_t tmp;
	ivm_bool_t first;
	const char *format;

	for (pc = 0; pc < len; pc = end) {

		op = ivm_exec_opAt(exec, pc);
		format = ivm_op_table_getArg(op);
		end = pc + ivm_op_table_getOffset(op);
		first = IVM_TRUE;

		fprintf(fp, "%s%4ld: %-20s",
				prefix, pc,
				ivm_op_table_getName(op));
		pc++;

		for (i = 0; format[i] != '\0';
			 i++, first = IVM_FALSE) {
			if (!first) {
				fprintf(fp, ",");
			}

			switch (format[i]) {
				case IVM_EXEC_FORMAT_ESC_CHAR:
					if (format[i + 1] == 'i') {
						if (format[i + 2] == '8') {
							fprintf(fp, " %db", ivm_byte_readSInt8(ivm_exec_offset(exec, pc)));
							pc++;
							i += 2;
							break;
						} else if (format[i + 2] == '1' &&
								   format[i + 3] == '6') {
							fprintf(fp, " %ds", ivm_byte_readSInt16(ivm_exec_offset(exec, pc)));
							pc += sizeof(ivm_sint16_t);
							i += 3;
							break;
						} else if (format[i + 2] == '3' &&
								   format[i + 3] == '2') {
							fprintf(fp, " %d", ivm_byte_readSInt32(ivm_exec_offset(exec, pc)));
							pc += sizeof(ivm_sint32_t);
							i += 3;
							break;
						} else if (format[i + 2] == '6' &&
								   format[i + 3] == '4') {
							fprintf(fp, " %ldl", ivm_byte_readSInt64(ivm_exec_offset(exec, pc)));
							pc += sizeof(ivm_sint64_t);
							i += 3;
							break;
						}
					} else if (format[i + 1] == 's') {
						tmp = ivm_byte_readSInt32(ivm_exec_offset(exec, pc));
						fprintf(fp, " #%d(\"%s\")",
								tmp, ivm_exec_getString(exec, tmp));
						pc += sizeof(ivm_sint32_t);
						i += 1;
						break;
					}
					/* fallthrough */
				default:
					fprintf(fp, " 8'%d", ivm_byte_readSInt8(ivm_exec_offset(exec, pc)));
					pc++;
			}
		}
		fprintf(fp, "\n");
	}

	return;
}

#define B2MB(val) (val / (2 << 20))

IVM_PRIVATE
void
ivm_dbg_printHeap(ivm_heap_t *heap,
				  const char *prefix,
				  FILE *fp)
{
	ivm_size_t bcount = IVM_HEAP_GET(heap, BLOCK_COUNT),
			   bsize = IVM_HEAP_GET(heap, BLOCK_SIZE),
			   *curs = IVM_HEAP_GET(heap, CUR_SIZE),
			   size = 0, i;

	fprintf(fp, "%sblock size: %ldMB\n", prefix, B2MB(bsize));
	fprintf(fp, "%sblock count: %ld\n", prefix, bcount);
	fprintf(fp, "%susage:\n", prefix);

	for (i = 0; i < bcount; i++) {
		fprintf(fp, "%s" IVM_DBG_TAB "block %ld: %ld in %ld(%.2f%%)\n",
				prefix, i + 1, curs[i], bsize,
				((double)curs[i]) / bsize * 100);
		size += curs[i];
	}
	fprintf(fp, "%s" IVM_DBG_TAB "(total: %ld in %ld(%.2f%%))\n",
			prefix, size, bsize,
			((double)size) / bsize * 100);

	return;
}

void
ivm_dbg_heapState(ivm_vmstate_t *state, FILE *fp)
{
	ivm_heap_t *cur = IVM_VMSTATE_GET(state, CUR_HEAP);
	ivm_heap_t *empty = IVM_VMSTATE_GET(state, EMPTY_HEAP);

	fprintf(fp, "current heap:\n");
	ivm_dbg_printHeap(cur, IVM_DBG_TAB, fp);

	fprintf(fp, "\nempty heap\n");
	ivm_dbg_printHeap(empty, IVM_DBG_TAB, fp);

	return;
}

void
ivm_dbg_printObject(ivm_object_t *obj, FILE *fp)
{
	
}
