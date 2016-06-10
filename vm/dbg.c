#include <stdio.h>
#include <string.h>
#include "pub/com.h"
#include "pub/vm.h"

#include "std/heap.h"
#include "dbg.h"
#include "opcode.h"
#include "byte.h"
#include "instr.h"
#include "exec.h"
#include "obj.h"
#include "coro.h"
#include "vmstack.h"

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
				prefix, pc, ivm_opcode_table_getName(instr.opc),
				instr.arg);
		if (ivm_opcode_table_getArg(instr.opc)[0] == 'S') {
			fprintf(fp, "(\"%s\")", ivm_string_trimHead(ivm_exec_getString(exec, instr.arg)));
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
			prefix, size, bsize * bcount,
			((double)size) / (bsize * bcount) * 100);

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
ivm_dbg_stackState(ivm_coro_t *coro, FILE *fp)
{
	ivm_frame_stack_t *frames = IVM_CORO_GET(coro, FRAME_STACK);
	ivm_vmstack_t *stack = IVM_CORO_GET(coro, STACK);
	ivm_runtime_t *runtime = IVM_CORO_GET(coro, RUNTIME);
	ivm_object_t *tmp;
	ivm_size_t i = 0, fi = 0,
			   end = runtime ? IVM_RUNTIME_GET(runtime, SP) : 0,
			   size = ivm_frame_stack_size(frames);
	ivm_frame_t *tmp_fr;

	if (stack) {
		fprintf(fp, "stack %p in coro %p:\n", (void *)stack, (void *)coro);
		while (i < end) {
			if (frames && fi < size
				&& IVM_FRAME_GET(tmp_fr = ivm_frame_stack_at(frames, fi), BP) == i) {
				fprintf(fp, "exec at %p\n", (void *)IVM_FRAME_GET(tmp_fr, EXEC));
			}

			tmp = ivm_vmstack_at(stack, i);
			fprintf(fp, IVM_DBG_TAB "%4ld: %p of <%s>\n", i, (void *)tmp,
					tmp ? IVM_OBJECT_GET(tmp, TYPE_NAME) : "null pointer");
			i++;
		}
	}

	fprintf(fp, "(total of %ld object(s) in the stack)\n", i);

	return;
}
