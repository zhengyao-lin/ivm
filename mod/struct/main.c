#include <stdio.h>

#include "pub/type.h"
#include "pub/vm.h"
#include "pub/obj.h"
#include "pub/err.h"
#include "pub/inlines.h"

#include "std/string.h"
#include "std/mem.h"

#include "vm/native/native.h"
#include "vm/native/priv.h"

#define STRUCT_TYPE_NAME "struct.struct"
#define STRUCT_TYPE_CONS IVM_GET_NATIVE_FUNC(_struct_struct)

#define STRUCT_ERROR_MSG_NO_DEFINED_FIELD							"no defined field is found"
#define STRUCT_ERROR_MSG_UNKNOWN_TYPE_FOR_FIELD(name)				"unknown type for field '%s'", (name)
#define STRUCT_ERROR_MSG_NO_DEFAULT_SIZE							"no default size"
#define STRUCT_ERROR_MSG_UNABLE_ALLOC_PACK_BUF						"unable to allocate buffer when packing"
#define STRUCT_ERROR_MSG_UNEXPECTED_ARG_TYPE(type, expect)			"unexpected argument type(expecting %s, %s given)", (expect), (type)
#define STRUCT_ERROR_MSG_BUFFER_TO_SMALL(size, expect)				"the buffer given is too small(expecting size %ld, buffer of size %ld given)", (expect), (size)

enum {
	IVM_STRUCT_TYPE_NONE = 0,
	IVM_STRUCT_TYPE_INT,
	IVM_STRUCT_TYPE_LONG,
	IVM_STRUCT_TYPE_FLOAT,
	IVM_STRUCT_TYPE_DOUBLE,
	IVM_STRUCT_TYPE_BUF,
	IVM_STRUCT_TYPE_COUNT
};

IVM_PRIVATE
IVM_INLINE
ivm_bool_t
_is_legal_type(ivm_uint_t t)
{
	return t > IVM_STRUCT_TYPE_NONE && t < IVM_STRUCT_TYPE_COUNT;
}

typedef struct {
	const ivm_string_t *name;
	ivm_size_t size;
	ivm_uint_t type;
} ivm_struct_field_t;

#define ivm_struct_field_build(name, size, type) \
	((ivm_struct_field_t) { (name), (size), (type) })

typedef struct {
	IVM_OBJECT_HEADER
	ivm_size_t rsize; // real size
	ivm_size_t fcount;
	ivm_struct_field_t *fields;
} ivm_struct_object_t;

ivm_object_t *
ivm_struct_object_new(ivm_vmstate_t *state,
					  ivm_size_t rsize,
					  ivm_size_t fcount,
					  ivm_struct_field_t *fields)
{
	ivm_struct_object_t *ret = ivm_vmstate_alloc(state, sizeof(*ret));

	ivm_object_init(IVM_AS_OBJ(ret), IVM_TPTYPE(state, STRUCT_TYPE_NAME));

	ret->rsize = rsize;
	ret->fcount = fcount;
	ret->fields = fields;

	ivm_vmstate_addDesLog(state, IVM_AS_OBJ(ret));

	return IVM_AS_OBJ(ret);
}

void
ivm_struct_object_destructor(ivm_object_t *obj,
							 ivm_vmstate_t *state)
{
	STD_FREE(IVM_AS(obj, ivm_struct_object_t)->fields);
	return;
}

void
ivm_struct_object_cloner(ivm_object_t *obj,
						 ivm_vmstate_t *state)
{
	ivm_struct_object_t *strc = IVM_AS(obj, ivm_struct_object_t);
	ivm_struct_field_t *orig = strc->fields;
	ivm_size_t size = sizeof(*strc->fields) * strc->fcount;

	strc->fields = ivm_vmstate_allocWild(state, size);
	if (!strc->fields) {
		strc->rsize = strc->fcount = 0;
	} else {
		STD_MEMCPY(strc->fields, orig, size);
	}

	return;
}

IVM_NATIVE_FUNC(_struct_struct)
{
	ivm_object_t *def, *type;
	ivm_slot_table_t *slots;
	ivm_slot_table_iterator_t siter;
	ivm_size_t fcount, size, fsize;
	ivm_struct_field_t *fields, *cur;
	const ivm_string_t *name;
	ivm_uint_t tid;

	MATCH_ARG(".", &def);

	slots = IVM_OBJECT_GET(def, SLOTS);

	RTM_ASSERT(slots, STRUCT_ERROR_MSG_NO_DEFINED_FIELD);

	fcount = 0;
	{ IVM_SLOT_TABLE_EACHPTR(slots, siter) fcount++; }

	RTM_ASSERT(fcount, STRUCT_ERROR_MSG_NO_DEFINED_FIELD);

	fields = ivm_vmstate_allocWild(NAT_STATE(), sizeof(*fields) * fcount);

	RTM_ASSERT(fields, IVM_ERROR_MSG_FAILED_ALLOC_NEW("struct"));

	cur = fields;
	size = 0;

	{
		IVM_SLOT_TABLE_EACHPTR(slots, siter) {
			name = IVM_SLOT_TABLE_ITER_GET_KEY(siter);
			type = IVM_SLOT_TABLE_ITER_GET_VAL(siter);

			if (IVM_IS_BTTYPE(type, NAT_STATE(), IVM_NUMERIC_T)) {
				tid = ivm_numeric_getValue(type);
				
				if (!_is_legal_type(tid)) {
					goto ILLEGAL_TYPE;
				}

				fsize = 0;
				switch (tid) {
					case IVM_STRUCT_TYPE_INT:
					case IVM_STRUCT_TYPE_FLOAT:
						fsize = 4;
						break;

					case IVM_STRUCT_TYPE_LONG:
					case IVM_STRUCT_TYPE_DOUBLE:
						fsize = 8;
						break;

					default:
						STD_FREE(fields);
						RTM_FATAL(STRUCT_ERROR_MSG_NO_DEFAULT_SIZE);
						/* unreachable */
				}

				size += fsize;
				*cur++ = ivm_struct_field_build(name, fsize, tid);
			} else {
			ILLEGAL_TYPE:
				STD_FREE(fields);
				RTM_FATAL(STRUCT_ERROR_MSG_UNKNOWN_TYPE_FOR_FIELD(ivm_string_trimHead(name)));
				/* unreachable */
			}
		}
	}

	return ivm_struct_object_new(NAT_STATE(), size, fcount, fields);
}

IVM_NATIVE_FUNC(_struct_struct_size)
{
	CHECK_BASE_TP(STRUCT_TYPE_CONS);
	return ivm_numeric_new(NAT_STATE(), IVM_AS(NAT_BASE(), ivm_struct_object_t)->rsize);
}

#define _write_native_endian(addr, type, val) \
	(*((type *)addr) = (type)(val))

#define _read_native_endian(addr, type) \
	(*((type *)addr))

IVM_NATIVE_FUNC(_struct_struct_pack)
{
	ivm_struct_object_t *strc;
	ivm_struct_field_t *i, *end;
	ivm_byte_t *buf, *cur;
	ivm_argc_t arg;

	CHECK_BASE_TP(STRUCT_TYPE_CONS);

	strc = IVM_AS(NAT_BASE(), ivm_struct_object_t);

	CHECK_ARG_COUNT(strc->fcount);

	cur = buf = ivm_vmstate_allocWild(NAT_STATE(), strc->rsize);

	RTM_ASSERT(buf, STRUCT_ERROR_MSG_UNABLE_ALLOC_PACK_BUF);

#define _CHECK_ARG(t) \
	if (!IVM_IS_BTTYPE(NAT_ARG_AT(arg), NAT_STATE(), t)) {    \
		STD_FREE(buf);                                        \
		RTM_FATAL(STRUCT_ERROR_MSG_UNEXPECTED_ARG_TYPE(       \
			IVM_OBJECT_GET(NAT_ARG_AT(arg), TYPE_NAME),       \
			ivm_vmstate_getTypeName(NAT_STATE(), t)           \
		));                                                   \
	}

	for (arg = 1,
		 i = strc->fields,
		 end = i + strc->fcount;
		 i != end; i++, arg++) {
		switch (i->type) {
			case IVM_STRUCT_TYPE_INT:
				_CHECK_ARG(IVM_NUMERIC_T);
				_write_native_endian(cur, ivm_sint32_t, ivm_numeric_getValue(NAT_ARG_AT(arg)));
				break;

			case IVM_STRUCT_TYPE_LONG:
				_CHECK_ARG(IVM_NUMERIC_T);
				_write_native_endian(cur, ivm_sint64_t, ivm_numeric_getValue(NAT_ARG_AT(arg)));
				break;

			case IVM_STRUCT_TYPE_FLOAT:
				_CHECK_ARG(IVM_NUMERIC_T);
				_write_native_endian(cur, ivm_single_t, ivm_numeric_getValue(NAT_ARG_AT(arg)));
				break;

			case IVM_STRUCT_TYPE_DOUBLE:
				_CHECK_ARG(IVM_NUMERIC_T);
				_write_native_endian(cur, ivm_double_t, ivm_numeric_getValue(NAT_ARG_AT(arg)));
				break;

			default:
				RTM_FATAL(STRUCT_ERROR_MSG_UNKNOWN_TYPE_FOR_FIELD(ivm_string_trimHead(i->name)));
		}

		cur += i->size;
	}

#undef _CHECK_ARG

	return ivm_buffer_object_new_c(NAT_STATE(), strc->rsize, buf);
}

IVM_NATIVE_FUNC(_struct_struct_unpack)
{
	ivm_buffer_object_t *buf;
	ivm_struct_object_t *strc;
	ivm_struct_field_t *i, *end;
	ivm_byte_t *raw, *cur;
	ivm_list_object_t *ret;

	CHECK_BASE_TP(STRUCT_TYPE_CONS);
	MATCH_ARG("b", &buf);

	strc = IVM_AS(NAT_BASE(), ivm_struct_object_t);

	RTM_ASSERT(
		ivm_buffer_object_getSize(buf) >= strc->rsize,
		STRUCT_ERROR_MSG_BUFFER_TO_SMALL(ivm_buffer_object_getSize(buf), strc->rsize)
	);

	ret = IVM_AS(ivm_list_object_new_b(NAT_STATE(), strc->fcount), ivm_list_object_t);

	cur = raw = ivm_buffer_object_getRaw(buf);

	for (i = strc->fields,
		 end = i + strc->fcount;
		 i != end; i++) {
		switch (i->type) {
			case IVM_STRUCT_TYPE_INT:
				ivm_list_object_push(
					ret, NAT_STATE(),
					ivm_numeric_new(NAT_STATE(), _read_native_endian(cur, ivm_sint32_t))
				);
				break;

			case IVM_STRUCT_TYPE_LONG:
				ivm_list_object_push(
					ret, NAT_STATE(),
					ivm_numeric_new(NAT_STATE(), _read_native_endian(cur, ivm_sint64_t))
				);
				break;

			case IVM_STRUCT_TYPE_FLOAT:
				ivm_list_object_push(
					ret, NAT_STATE(),
					ivm_numeric_new(NAT_STATE(), _read_native_endian(cur, ivm_single_t))
				);
				break;

			case IVM_STRUCT_TYPE_DOUBLE:
				ivm_list_object_push(
					ret, NAT_STATE(),
					ivm_numeric_new(NAT_STATE(), _read_native_endian(cur, ivm_double_t))
				);
				break;

			default:
				RTM_FATAL(STRUCT_ERROR_MSG_UNKNOWN_TYPE_FOR_FIELD(ivm_string_trimHead(i->name)));
		}

		cur += i->size;
	}

	return IVM_AS_OBJ(ret);
}

IVM_PRIVATE
ivm_type_t
_struct_type = IVM_TPTYPE_BUILD(
	STRUCT_TYPE_NAME, sizeof(ivm_struct_object_t),
	STRUCT_TYPE_CONS,

	.des = ivm_struct_object_destructor,
	.clone = ivm_struct_object_cloner,
	.const_bool = IVM_TRUE
);

ivm_object_t *
ivm_mod_main(ivm_vmstate_t *state,
			 ivm_coro_t *coro,
			 ivm_context_t *context)
{
	ivm_object_t *mod = ivm_object_new(state);
	ivm_object_t *struct_proto;

	/* struct.struct */
	IVM_VMSTATE_REGISTER_TPTYPE(state, coro, STRUCT_TYPE_NAME, &_struct_type, {
		struct_proto = ivm_struct_object_new(state, 0, 0, IVM_NULL);
		ivm_type_setProto(_TYPE, struct_proto);
		ivm_object_setProto(struct_proto, state, ivm_vmstate_getTypeProto(state, IVM_OBJECT_T));

		ivm_object_setSlot_r(struct_proto, state, "size", IVM_NATIVE_WRAP(state, _struct_struct_size));
		ivm_object_setSlot_r(struct_proto, state, "pack", IVM_NATIVE_WRAP(state, _struct_struct_pack));
		ivm_object_setSlot_r(struct_proto, state, "unpack", IVM_NATIVE_WRAP(state, _struct_struct_unpack));
	});

	ivm_object_setSlot_r(mod, state, "struct", IVM_NATIVE_WRAP_CONS(state, struct_proto, _struct_struct));

	ivm_object_setSlot_r(mod, state, "int", ivm_numeric_new(state, IVM_STRUCT_TYPE_INT));
	ivm_object_setSlot_r(mod, state, "long", ivm_numeric_new(state, IVM_STRUCT_TYPE_LONG));
	ivm_object_setSlot_r(mod, state, "float", ivm_numeric_new(state, IVM_STRUCT_TYPE_FLOAT));
	ivm_object_setSlot_r(mod, state, "double", ivm_numeric_new(state, IVM_STRUCT_TYPE_DOUBLE));

	return mod;
}
