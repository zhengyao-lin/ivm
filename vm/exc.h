#ifndef _IVM_VM_EXC_H_
#define _IVM_VM_EXC_H_

#include "pub/com.h"
#include "pub/type.h"

#include "obj.h"

IVM_COM_HEADER

struct ivm_vmstate_t_tag;

typedef struct {
	IVM_OBJECT_HEADER
	const ivm_string_t *msg;
	const ivm_string_t *file;
	ivm_long_t line;
} ivm_exception_t;

ivm_object_t *
ivm_exception_new(struct ivm_vmstate_t_tag *state,
				  const ivm_char_t *msg,
				  const ivm_char_t *file,
				  ivm_long_t line);

ivm_object_t *
ivm_exception_new_c(struct ivm_vmstate_t_tag *state,
					const ivm_string_t *msg,
					const ivm_string_t *file,
					ivm_long_t line);

#define ivm_exception_getMsg(exc) ((exc)->msg)
#define ivm_exception_getFile(exc) ((exc)->file)
#define ivm_exception_getLine(exc) ((exc)->line)

#define ivm_exception_getMsg_r(exc) ivm_string_trimHead((exc)->msg)
#define ivm_exception_getFile_r(exc) ivm_string_trimHead((exc)->file)
#define ivm_exception_getLine(exc) ((exc)->line)

void
ivm_exception_setMsg(ivm_exception_t *exc,
					 struct ivm_vmstate_t_tag *state,
					 const ivm_char_t *msg);

void
ivm_exception_setPos(ivm_exception_t *exc,
					 struct ivm_vmstate_t_tag *state,
					 const ivm_char_t *file,
					 ivm_long_t line);

IVM_COM_END

#endif
