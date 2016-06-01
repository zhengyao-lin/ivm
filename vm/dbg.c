#include <stdio.h>
#include <string.h>
#include "pub/com.h"
#include "dbg.h"
#include "op.h"
#include "byte.h"
#include "instr.h"
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
	ivm_instr_t instr;
	ivm_size_t len = ivm_exec_length(exec);

	for (pc = 0; pc < len; pc++) {
		instr = ivm_exec_instrAt(exec, pc);

		fprintf(fp, "%s%4ld: %-20s %d",
				prefix, pc, ivm_op_table_getName(instr.op),
				instr.arg);
		if (ivm_op_table_getArg(instr.op)[0] == 'S') {
			fprintf(fp, "(\"%s\")", ivm_exec_getString(exec, instr.arg));
		}

		fprintf(fp, "\n");
	}

	return;
}

#define B2MB(val) ((double)val / (2 << 20))

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

	fprintf(fp, "%sblock size: %.2fMB\n", prefix, B2MB(bsize));
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
