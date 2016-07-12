#include <stdio.h>
#include <string.h>

#include "pub/com.h"
#include "pub/const.h"
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

IVM_PRIVATE
void
_ivm_dbg_printInstr(ivm_exec_t *exec,
					ivm_instr_t *ip,
					const char *format,
					FILE *fp)
{
	char t;
	char buffer[64];
	ivm_ptrdiff_t pc = (ivm_ptr_t)ip - 0;

	if (exec)
		pc = (ivm_ptr_t)ip - (ivm_ptr_t)ivm_exec_instrPtrStart(exec);

	IVM_SNPRINTF(buffer, sizeof(buffer), format, pc / sizeof(*ip));
	// "%4ld: "
	// 
	switch (t = ivm_opcode_table_getParam(ivm_instr_opcode(ip))[0]) {
		case 'N':
			fprintf(fp, "%s%s",
					buffer, ivm_opcode_table_getName(ivm_instr_opcode(ip)));
			break;
		case 'I':
		case 'X':
			fprintf(fp, "%s%-20s %ld",
					buffer, ivm_opcode_table_getName(ivm_instr_opcode(ip)),
					ivm_opcode_arg_toInt(ivm_instr_arg(ip)));
			break;
		case 'F':
			fprintf(fp, "%s%-20s %f",
					buffer, ivm_opcode_table_getName(ivm_instr_opcode(ip)),
					ivm_opcode_arg_toFloat(ivm_instr_arg(ip)));
			break;
		case 'S':
			if (!exec || ivm_exec_cached(exec)) {
				fprintf(fp, "%s%-20s #?(\"%s\")",
						buffer, ivm_opcode_table_getName(ivm_instr_opcode(ip)),
						ivm_string_trimHead(
							(const ivm_string_t *)ivm_opcode_arg_toPointer(ivm_instr_arg(ip))
						));
			} else {
				fprintf(fp, "%s%-20s #%ld",
						buffer, ivm_opcode_table_getName(ivm_instr_opcode(ip)),
						ivm_opcode_arg_toInt(ivm_instr_arg(ip)));
				fprintf(fp, "(\"%s\")",
						ivm_string_trimHead(ivm_exec_getString(exec, ivm_opcode_arg_toInt(ivm_instr_arg(ip)))));
			}
			break;
		default: IVM_FATAL(IVM_ERROR_MSG_UNEXPECTED_ARG_TYPE(t));
	}

	return;
}

void
ivm_dbg_printExec(ivm_exec_t *exec,
				   const char *prefix,
				   FILE *fp)
{
	ivm_instr_t *ip = ivm_exec_instrPtrStart(exec),
				*end = ivm_exec_instrPtrEnd(exec);

	for (; ip != end; ip++) {
		fprintf(fp, "%s", prefix);
		_ivm_dbg_printInstr(exec, ip, "%4ld: ", fp);
		fprintf(fp, "\n");
	}

	return;
}

void
ivm_dbg_printExecUnit(ivm_exec_unit_t *unit, FILE *fp)
{
	ivm_size_t i = 0;
	ivm_exec_list_t *list = ivm_exec_unit_execList(unit);
	ivm_exec_list_iterator_t eiter;

	IVM_EXEC_LIST_EACHPTR(list, eiter) {
		if (i) {
			fputc('\n', fp);
		}

		fprintf(fp, "exec %lu:\n", i);
		i++;
		ivm_dbg_printExec(
			IVM_EXEC_LIST_ITER_GET(eiter),
			"  ", fp
		);
	}

	return;
}

#define B2MB(val) ((double)val / (2 << 20))

IVM_PRIVATE
void
_ivm_dbg_printHeap(ivm_heap_t *heap,
				   const char *prefix,
				   FILE *fp)
{
	ivm_size_t bcount = IVM_HEAP_GET(heap, BLOCK_COUNT),
			   bsize = IVM_HEAP_GET(heap, BLOCK_SIZE),
			   used = IVM_HEAP_GET(heap, BLOCK_USED),
			   total = IVM_HEAP_GET(heap, BLOCK_TOTAL);

	fprintf(fp, "%sblock size: %.2fMB\n", prefix, B2MB(bsize));
	fprintf(fp, "%sblock count: %zd\n", prefix, bcount);
	fprintf(fp, "%susage: %zd in %zd\n", prefix, used, total);

	return;
}

void
ivm_dbg_heapState(ivm_vmstate_t *state, FILE *fp)
{
	ivm_heap_t *cur = IVM_VMSTATE_GET(state, CUR_HEAP);
	ivm_heap_t *empty = IVM_VMSTATE_GET(state, EMPTY_HEAP);

	fprintf(fp, "current heap:\n");
	_ivm_dbg_printHeap(cur, IVM_DBG_TAB, fp);

	fprintf(fp, "\nempty heap\n");
	_ivm_dbg_printHeap(empty, IVM_DBG_TAB, fp);

	return;
}

void
ivm_dbg_stackState(ivm_coro_t *coro, FILE *fp)
{
	ivm_frame_stack_t *frames = IVM_CORO_GET(coro, FRAME_STACK);
	ivm_vmstack_t *stack = IVM_CORO_GET(coro, STACK);
	ivm_runtime_t *runtime = IVM_CORO_GET(coro, RUNTIME);
	ivm_object_t *tmp;
	ivm_size_t fi = 0,
			   fsize = ivm_frame_stack_size(frames);

	ivm_object_t **i = runtime ? ivm_vmstack_edge(stack) : IVM_NULL,
				 **sp = runtime ? IVM_RUNTIME_GET(runtime, SP) : IVM_NULL;

	ivm_frame_t *tmp_fr;

	if (stack) {
		/* fprintf(fp, "stack %p in coro %p:\n", (void *)stack, (void *)coro); */
		while (i != sp) {
			if (frames && fi < fsize
				&& IVM_FRAME_GET(tmp_fr = ivm_frame_stack_at(frames, fi), BP) == i) {
				fprintf(fp, "ip at %p\n", (void *)IVM_FRAME_GET(tmp_fr, IP));
				fi++;
			}

			tmp = *i;
			fprintf(fp, IVM_DBG_TAB "%4ld: %p of <%s>\n",
					ivm_vmstack_offset(stack, i), (void *)tmp,
					tmp ? IVM_OBJECT_GET(tmp, TYPE_NAME) : "null pointer");
			i++;
		}
	}

	if (stack && i) {
		fprintf(fp, "(total of %ld object(s) in the stack)\n",
				ivm_vmstack_offset(stack, i));
	} else {
		fprintf(fp, "(empty stack)\n");
	}

	return;
}

IVM_PRIVATE
const char
stack_border[] = "--------------";

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX_CELL_COUNT 10
#define CELL_WIDTH_STR "-14"
#define CONTENT_WIDTH_STR "-12"

#define TYPE_NAME_OF(obj) \
	((obj) ? IVM_OBJECT_GET((obj), TYPE_NAME) : "(nil)")

void
ivm_dbg_printRuntime(ivm_dbg_runtime_t runtime)
{
	ivm_int_t i, tmp_cst = runtime.cst;
	ivm_vmstack_t *stack = runtime.stack;
	ivm_int_t border_count = MIN(MAX_CELL_COUNT, runtime.sp + tmp_cst);;

	IVM_TRACE("\nstack state(sp: %zd, bp: %zd, cst: %d, cmp_reg: %d):\n",
			  runtime.sp, runtime.bp, runtime.cst, runtime.cmp_reg);

#if 0
#if IVM_STACK_CACHE_N_TOS == 1
	if (tmp_cst)
		IVM_TRACE("%" CELL_WIDTH_STR "s", " stc0");
	IVM_TRACE("\n");
#elif IVM_STACK_CACHE_N_TOS == 2
	IVM_TRACE("%" CELL_WIDTH_STR "s", " stc0");
	IVM_TRACE("%" CELL_WIDTH_STR "s", " stc1");
	IVM_TRACE("\n");
#endif
#endif

DRAW_BORDER:
	for (i = 0; i < border_count; i++) {
		IVM_TRACE(stack_border);
	}
	IVM_TRACE("-\n");

	if (!stack) goto DRAW_END;

#if IVM_STACK_CACHE_N_TOS == 1
	if (tmp_cst) {
		IVM_TRACE("|>");
		IVM_TRACE("%" CONTENT_WIDTH_STR "s", TYPE_NAME_OF(runtime.stc0));
	}
#elif IVM_STACK_CACHE_N_TOS == 2
	if (tmp_cst == 2) {
		IVM_TRACE("|>");
		IVM_TRACE("%" CONTENT_WIDTH_STR "s", TYPE_NAME_OF(runtime.stc1));
		IVM_TRACE("|>");
		IVM_TRACE("%" CONTENT_WIDTH_STR "s", TYPE_NAME_OF(runtime.stc0));
	} else if (tmp_cst == 1) {
		IVM_TRACE("|>");
		IVM_TRACE("%" CONTENT_WIDTH_STR "s", TYPE_NAME_OF(runtime.stc0));
	}
#endif

	for (i = runtime.sp - 1;
		 i >= 0 && runtime.sp - i - tmp_cst < MAX_CELL_COUNT;
		 i--) {
		IVM_TRACE("| ");
		IVM_TRACE("%" CONTENT_WIDTH_STR "s", TYPE_NAME_OF(ivm_vmstack_at(stack, i)));
	}

	if (i < 0) {
		IVM_TRACE("| <end>");
	} else {
		IVM_TRACE("| <...>");
	}

	IVM_TRACE("\n");

	stack = IVM_NULL;
	goto DRAW_BORDER;

DRAW_END:

	IVM_TRACE("\n");

	// ivm_dbg_heapState(runtime.state, stderr);
	// IVM_TRACE("\n");

	switch (runtime.action) {
		case IVM_CORO_ACTION_NONE:
			_ivm_dbg_printInstr(IVM_NULL, runtime.ip, "> ", stderr);
			break;
		case IVM_CORO_ACTION_INVOKE:
			if (!runtime.retval)
				IVM_TRACE("****** ACTION: INVOKE ******");
			else
				IVM_TRACE("****** ACTION: INVOKE NATIVE ******");
			break;
		case IVM_CORO_ACTION_YIELD:
			IVM_TRACE("****** ACTION: YIELD(%s) ******", TYPE_NAME_OF(runtime.retval));
			break;
		case IVM_CORO_ACTION_RETURN:
			IVM_TRACE("****** ACTION: RETURN(%s) ******", TYPE_NAME_OF(runtime.retval));
			break;
	}

	getc(stdin);
	
	return;
}
