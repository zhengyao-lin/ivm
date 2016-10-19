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

#define STRUCT_ERROR_MSG_NO_DEFINED_FIELD							"no field is found"
#define STRUCT_ERROR_MSG_UNKNOWN_FIELD_TYPE(n)						"unknown field type for the %ldth field", (n)
#define STRUCT_ERROR_MSG_ILLEGAL_ARRAY_SIZE(size)					"illegal array size %ld", (size)
#define STRUCT_ERROR_MSG_NOT_ENOUGH_FOR_ARRAY(size, req)			"list too small for array packing(expecting %ld, %ld given)", (req), (size)
#define STRUCT_ERROR_MSG_NO_DEFAULT_SIZE							"no default size"
#define STRUCT_ERROR_MSG_UNABLE_ALLOC_PACK_BUF						"unable to allocate buffer when packing"
#define STRUCT_ERROR_MSG_UNEXPECTED_ARG_TYPE(type, expect)			"unexpected argument type(expecting %s, %s given)", (expect), (type)
#define STRUCT_ERROR_MSG_BUFFER_TO_SMALL(size, expect)				"the buffer given is too small(expecting size %ld, buffer of size %ld given)", (expect), (size)

enum {
	IVM_STRUCT_TYPE_NONE = 0,
	IVM_STRUCT_TYPE_BYTE,
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
	ivm_size_t size;
	ivm_size_t count;
	ivm_uint_t type;
} ivm_struct_field_t;

#define ivm_struct_field_build(size, type) \
	((ivm_struct_field_t) { (size), 0, (type) })

#define ivm_struct_field_buildArray(size, type, count) \
	((ivm_struct_field_t) { (size), (count), (type) })

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

	ivm_vmstate_addDesLog(state, obj);

	return;
}

IVM_NATIVE_FUNC(_struct_struct)
{
	ivm_list_object_t *def;
	ivm_object_t *type, *tmp_obj;
	ivm_list_object_iterator_t liter;

	ivm_size_t fcount, size, fsize;
	ivm_long_t arr_size;
	ivm_struct_field_t *fields, *cur;
	ivm_uint_t tid;

	MATCH_ARG("l", &def);

	fcount = ivm_list_object_getElementCount(def);
	RTM_ASSERT(fcount, STRUCT_ERROR_MSG_NO_DEFINED_FIELD);

	fields = ivm_vmstate_allocWild(NAT_STATE(), sizeof(*fields) * fcount);

	RTM_ASSERT(fields, IVM_ERROR_MSG_FAILED_ALLOC_NEW("struct"));

	cur = fields;
	size = 0;

	IVM_LIST_OBJECT_EACHPTR(def, liter) {
		type = IVM_LIST_OBJECT_ITER_GET(liter);

		if (IVM_IS_BTTYPE(type, NAT_STATE(), IVM_NUMERIC_T)) {
			tid = ivm_numeric_getValue(type);
			
			if (!_is_legal_type(tid)) {
				goto ILLEGAL_TYPE;
			}

			fsize = 0;
			switch (tid) {
				case IVM_STRUCT_TYPE_BYTE:
					fsize = 1;
					break;

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
			*cur++ = ivm_struct_field_build(fsize, tid);
		} else if (IVM_IS_BTTYPE(type, NAT_STATE(), IVM_OBJECT_T)) {
			tmp_obj = ivm_object_getSlot_r(type, NAT_STATE(), "type");
			if (tmp_obj && IVM_IS_BTTYPE(tmp_obj, NAT_STATE(), IVM_NUMERIC_T)) {
				tid = ivm_numeric_getValue(tmp_obj);
			} else goto ILLEGAL_TYPE;

			tmp_obj = ivm_object_getSlot_r(type, NAT_STATE(), "count");
			if (tmp_obj && IVM_IS_BTTYPE(tmp_obj, NAT_STATE(), IVM_NUMERIC_T)) {
				arr_size = ivm_numeric_getValue(tmp_obj);
			} else goto ILLEGAL_TYPE;

			RTM_ASSERT(arr_size > 0, STRUCT_ERROR_MSG_ILLEGAL_ARRAY_SIZE(arr_size));

			switch (tid) {
				case IVM_STRUCT_TYPE_BYTE:
					fsize = 1;
					break;

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

			size += fsize * arr_size;
			*cur++ = ivm_struct_field_buildArray(fsize, tid, arr_size);
		} else {
		ILLEGAL_TYPE:
			STD_FREE(fields);
			RTM_FATAL(STRUCT_ERROR_MSG_UNKNOWN_FIELD_TYPE(IVM_LIST_OBJECT_ITER_INDEX(def, liter)) + 1);
			/* unreachable */
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

#define _CHECK_ARG(t) \
	if (!IVM_IS_BTTYPE(NAT_ARG_AT(arg), NAT_STATE(), t)) {    \
		RTM_FATAL(STRUCT_ERROR_MSG_UNEXPECTED_ARG_TYPE(       \
			IVM_OBJECT_GET(NAT_ARG_AT(arg), TYPE_NAME),       \
			ivm_vmstate_getTypeName(NAT_STATE(), t)           \
		));                                                   \
	}

#define SUB_NUM_FIELD(id, type) \
	case id:                                                                      \
		_CHECK_ARG(IVM_NUMERIC_T);                                                \
		_write_native_endian(cur, type, ivm_numeric_getValue(NAT_ARG_AT(arg)));   \
		break;

#define SUB_ARR_FIELD(id, type) \
	case id: {                                                                                     \
		ivm_size_t count = i->count;                                                               \
		ivm_object_t *tmp_obj;                                                                     \
		_CHECK_ARG(IVM_LIST_OBJECT_T);                                                             \
                                                                                                   \
		arr = IVM_AS(NAT_ARG_AT(arg), ivm_list_object_t);                                          \
		RTM_ASSERT(ivm_list_object_getSize(arr) >= count,                                          \
				   STRUCT_ERROR_MSG_NOT_ENOUGH_FOR_ARRAY(ivm_list_object_getSize(arr), count));    \
                                                                                                   \
		IVM_LIST_OBJECT_ALLPTR(arr, liter) {                                                       \
			if (!count) break;                                                                     \
			tmp_obj = IVM_LIST_OBJECT_ITER_GET(liter);                                             \
			RTM_ASSERT(                                                                            \
				tmp_obj && IVM_IS_BTTYPE(tmp_obj, NAT_STATE(), IVM_NUMERIC_T),                     \
				STRUCT_ERROR_MSG_UNEXPECTED_ARG_TYPE(                                              \
					tmp_obj ? IVM_OBJECT_GET(tmp_obj, TYPE_NAME) : "<nil>", "numeric"              \
				)                                                                                  \
			);                                                                                     \
			_write_native_endian(cur, type, ivm_numeric_getValue(tmp_obj));                        \
			cur += i->size;                                                                        \
			count--;                                                                               \
		}                                                                                          \
		break;                                                                                     \
	}

#define WRITE_FIELD() \
	if (!i->count) {                                                                                \
		switch (i->type) {                                                                          \
			SUB_NUM_FIELD(IVM_STRUCT_TYPE_BYTE, ivm_byte_t)                                         \
			SUB_NUM_FIELD(IVM_STRUCT_TYPE_INT, ivm_sint32_t)                                        \
			SUB_NUM_FIELD(IVM_STRUCT_TYPE_LONG, ivm_sint64_t)                                       \
			SUB_NUM_FIELD(IVM_STRUCT_TYPE_FLOAT, ivm_single_t)                                      \
			SUB_NUM_FIELD(IVM_STRUCT_TYPE_DOUBLE, ivm_double_t)                                     \
			default:                                                                                \
				RTM_FATAL(                                                                          \
					STRUCT_ERROR_MSG_UNKNOWN_FIELD_TYPE(                                            \
						IVM_PTR_DIFF(i, strc->fields, ivm_struct_field_t)                           \
					) + 1                                                                           \
				);                                                                                  \
		}                                                                                           \
		cur += i->size;                                                                             \
	} else {                                                                                        \
		switch (i->type) {                                                                          \
			SUB_ARR_FIELD(IVM_STRUCT_TYPE_BYTE, ivm_byte_t)                                         \
			SUB_ARR_FIELD(IVM_STRUCT_TYPE_INT, ivm_sint32_t)                                        \
			SUB_ARR_FIELD(IVM_STRUCT_TYPE_LONG, ivm_sint64_t)                                       \
			SUB_ARR_FIELD(IVM_STRUCT_TYPE_FLOAT, ivm_single_t)                                      \
			SUB_ARR_FIELD(IVM_STRUCT_TYPE_DOUBLE, ivm_double_t)                                     \
			default:                                                                                \
				RTM_FATAL(                                                                          \
					STRUCT_ERROR_MSG_UNKNOWN_FIELD_TYPE(                                            \
						IVM_PTR_DIFF(i, strc->fields, ivm_struct_field_t)                           \
					) + 1                                                                           \
				);                                                                                  \
		}                                                                                           \
	}

IVM_NATIVE_FUNC(_struct_struct_pack)
{
	ivm_struct_object_t *strc;
	ivm_struct_field_t *i, *end;
	ivm_byte_t *buf, *cur;
	ivm_argc_t arg;
	ivm_object_t *ret;

	ivm_list_object_t *arr;
	ivm_list_object_iterator_t liter;

	CHECK_BASE_TP(STRUCT_TYPE_CONS);

	strc = IVM_AS(NAT_BASE(), ivm_struct_object_t);

	CHECK_ARG_COUNT(strc->fcount);

	cur = buf = ivm_vmstate_allocWild(NAT_STATE(), strc->rsize);
	ret = ivm_buffer_object_new_c(NAT_STATE(), strc->rsize, buf);

	RTM_ASSERT(buf, STRUCT_ERROR_MSG_UNABLE_ALLOC_PACK_BUF);

	for (arg = 1,
		 i = strc->fields,
		 end = i + strc->fcount;
		 i != end; i++, arg++) {
		WRITE_FIELD();
	}

	return ret;
}

IVM_NATIVE_FUNC(_struct_struct_packto)
{
	ivm_struct_object_t *strc;
	ivm_struct_field_t *i, *end;
	ivm_byte_t *buf, *cur;
	ivm_argc_t arg;
	ivm_buffer_object_t *buf_obj;

	ivm_list_object_t *arr;
	ivm_list_object_iterator_t liter;

	CHECK_BASE_TP(STRUCT_TYPE_CONS);

	strc = IVM_AS(NAT_BASE(), ivm_struct_object_t);

	CHECK_ARG_COUNT(strc->fcount + 1);
	MATCH_ARG("b", &buf_obj);

	RTM_ASSERT(
		ivm_buffer_object_getSize(buf_obj) >= strc->rsize,
		STRUCT_ERROR_MSG_BUFFER_TO_SMALL(ivm_buffer_object_getSize(buf_obj), strc->rsize)
	);

	cur = buf = ivm_buffer_object_getRaw(buf_obj);

	for (arg = 2,
		 i = strc->fields,
		 end = i + strc->fcount;
		 i != end; i++, arg++) {
		WRITE_FIELD();
	}

	return IVM_AS_OBJ(buf_obj);
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

#define SUB1(id, type) \
	case id:                                                               \
		if (!ivm_list_object_push(                                         \
			ret, NAT_STATE(),                                              \
			ivm_numeric_new(NAT_STATE(), _read_native_endian(cur, type))   \
		)) return IVM_NULL;                                                \
		break;

#define SUB2(id, type) \
	case id: {                                                                                           \
		ivm_size_t count = i->count, fsize = i->size;                                                    \
		ivm_list_object_t *arr = IVM_AS(ivm_list_object_new_b(NAT_STATE(), count), ivm_list_object_t);   \
                                                                                                         \
		for (; count; count--, cur += fsize) {                                                           \
			if (!ivm_list_object_push(                                                                   \
				arr, NAT_STATE(),                                                                        \
				ivm_numeric_new(NAT_STATE(), _read_native_endian(cur, type))                             \
			)) return IVM_NULL;                                                                          \
		}                                                                                                \
                                                                                                         \
		if (!ivm_list_object_push(ret, NAT_STATE(), IVM_AS_OBJ(arr)))                                    \
			return IVM_NULL;                                                                             \
                                                                                                         \
		break;                                                                                           \
	}

	for (i = strc->fields,
		 end = i + strc->fcount;
		 i != end; i++) {
		if (!i->count) {
			switch (i->type) {
				SUB1(IVM_STRUCT_TYPE_BYTE, ivm_byte_t)
				SUB1(IVM_STRUCT_TYPE_INT, ivm_sint32_t)
				SUB1(IVM_STRUCT_TYPE_LONG, ivm_sint64_t)
				SUB1(IVM_STRUCT_TYPE_FLOAT, ivm_single_t)
				SUB1(IVM_STRUCT_TYPE_DOUBLE, ivm_double_t)
				default:
					RTM_FATAL(STRUCT_ERROR_MSG_UNKNOWN_FIELD_TYPE(IVM_PTR_DIFF(i, strc->fields, ivm_struct_field_t)) + 1);
			}

			cur += i->size;
		} else {
			switch (i->type) {
				SUB2(IVM_STRUCT_TYPE_BYTE, ivm_byte_t)
				SUB2(IVM_STRUCT_TYPE_INT, ivm_sint32_t)
				SUB2(IVM_STRUCT_TYPE_LONG, ivm_sint64_t)
				SUB2(IVM_STRUCT_TYPE_FLOAT, ivm_single_t)
				SUB2(IVM_STRUCT_TYPE_DOUBLE, ivm_double_t)
				default:
					RTM_FATAL(STRUCT_ERROR_MSG_UNKNOWN_FIELD_TYPE(IVM_PTR_DIFF(i, strc->fields, ivm_struct_field_t)) + 1);
			}
		}
	}

	return IVM_AS_OBJ(ret);
}

IVM_NATIVE_FUNC(_struct_array)
{
	ivm_object_t *obj;

	CHECK_ARG_COUNT(2);

	obj = ivm_object_new(NAT_STATE());

	ivm_object_setSlot_r(obj, NAT_STATE(), "type", NAT_ARG_AT(1));
	ivm_object_setSlot_r(obj, NAT_STATE(), "count", NAT_ARG_AT(2));

	return obj;
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
		ivm_object_setSlot_r(struct_proto, state, "packto", IVM_NATIVE_WRAP(state, _struct_struct_packto));
		ivm_object_setSlot_r(struct_proto, state, "unpack", IVM_NATIVE_WRAP(state, _struct_struct_unpack));
	});

	ivm_object_setSlot_r(mod, state, "struct", IVM_NATIVE_WRAP_CONS(state, struct_proto, _struct_struct));

	ivm_object_setSlot_r(mod, state, "byte", ivm_numeric_new(state, IVM_STRUCT_TYPE_BYTE));
	ivm_object_setSlot_r(mod, state, "int", ivm_numeric_new(state, IVM_STRUCT_TYPE_INT));
	ivm_object_setSlot_r(mod, state, "long", ivm_numeric_new(state, IVM_STRUCT_TYPE_LONG));
	ivm_object_setSlot_r(mod, state, "float", ivm_numeric_new(state, IVM_STRUCT_TYPE_FLOAT));
	ivm_object_setSlot_r(mod, state, "double", ivm_numeric_new(state, IVM_STRUCT_TYPE_DOUBLE));

	ivm_object_setSlot_r(mod, state, "array", IVM_NATIVE_WRAP(state, _struct_array));

	return mod;
}
