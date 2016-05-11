#include <stdio.h>
#include <string.h>
#include "dbg.h"
#include "op.h"
#include "byte.h"
#include "exec.h"

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
