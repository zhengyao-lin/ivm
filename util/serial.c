#include "pub/com.h"
#include "pub/const.h"
#include "pub/type.h"
#include "pub/vm.h"
#include "pub/inlines.h"

#include "std/mem.h"
#include "std/string.h"
#include "std/io.h"

#include "vm/mod/mod.h"

#include "serial.h"

typedef struct {
	ivm_byte_t opc;
	ivm_opcode_arg_t arg;
} ivm_serial_instr_t;

typedef struct {
	ivm_uint64_t name;
	ivm_byte_t is_varg;
} ivm_serial_param_t;

typedef struct {
	ivm_uint64_t count;
	ivm_serial_param_t *param;
} ivm_serial_param_list_t;

typedef struct {
	ivm_uint64_t pool;
	ivm_uint64_t size;
	ivm_serial_param_list_t *param;
	ivm_serial_instr_t *instrs;
} ivm_serial_exec_t;

typedef struct {
	ivm_string_pool_list_t *pool_list;
	ivm_uint64_t size;
	ivm_serial_exec_t *execs;
} ivm_serial_exec_list_t;

typedef struct {
	ivm_uint64_t root;
	ivm_serial_exec_list_t *list;
} ivm_serial_exec_unit_t;

IVM_PRIVATE
ivm_serial_param_list_t *
_ivm_serial_serializeParam(ivm_param_list_t *param,
						   ivm_vmstate_t *state,
						   ivm_string_pool_t *pool)
{
	ivm_serial_param_list_t *ret = STD_ALLOC(sizeof(*ret));
	ivm_param_t *tmp;
	ivm_size_t i;

	IVM_MEMCHECK(ret);

	ret->count = ivm_param_list_count(param);
	ret->param = STD_ALLOC(sizeof(*ret->param) * ret->count);

	IVM_MEMCHECK(ret->param);

	for (i = 0; i < ret->count; i++) {
		tmp = ivm_param_list_getParam(param, i);
		ret->param[i].name = ivm_string_pool_register_i(pool, ivm_param_name(tmp));
		ret->param[i].is_varg = ivm_param_isVarg(tmp);
	}

	return ret;
}

IVM_PRIVATE
ivm_serial_exec_t
_ivm_serial_serializeExec(ivm_exec_t *exec,
						  ivm_vmstate_t *state,
						  ivm_size_t pool_id)
{
	ivm_serial_exec_t ret = {
		pool_id,
		ivm_exec_length(exec)
	};
	ivm_instr_t *i, *end;
	ivm_serial_instr_t *tmp;

	IVM_IMPORTANT(!ivm_exec_cached(exec), IVM_ERROR_MSG_SERIALIZE_CACHED_EXEC);

	tmp = ret.instrs = STD_ALLOC(sizeof(*ret.instrs) * ret.size);

	IVM_MEMCHECK(ret.instrs);

	ret.param = _ivm_serial_serializeParam(ivm_exec_getParam(exec), state, ivm_exec_pool(exec));

	for (i = ivm_exec_instrPtrStart(exec),
		 end = i + ret.size;
		 i != end; i++, tmp++) {
		*tmp = (ivm_serial_instr_t) {
			.opc = ivm_instr_opcode(i),
			.arg = ivm_instr_arg(i)
		};
	}

	return ret;
}

IVM_PRIVATE
ivm_exec_t *
_ivm_serial_unserializeExec(ivm_serial_exec_t *exec,
							ivm_string_pool_list_t *pool_list)
{
	ivm_size_t count = exec->param->count, cur;
	ivm_string_pool_t *pool = ivm_string_pool_list_at(pool_list, exec->pool);
	ivm_exec_t *ret = ivm_exec_new(pool, count);
	ivm_serial_instr_t *i, *end;
	ivm_serial_param_t *tmp;

	for (cur = 0; cur != count; cur++) {
		tmp = exec->param->param + cur;
		
		if (!ivm_string_pool_hasIndex(pool, tmp->name)) {
			/* illegal string id */
			ivm_ref_inc(ret);
			ivm_exec_free(ret);
			return IVM_NULL;
		}

		ivm_exec_setParam(ret, cur, ivm_string_pool_get(pool, tmp->name), tmp->is_varg);
	}

	for (i = exec->instrs,
		 end = i + exec->size;
		 i != end; i++) {
		ivm_exec_addInstr_c(ret, ivm_instr_build(i->opc, i->arg));
	}

	return ret;
}

IVM_PRIVATE
ivm_serial_exec_list_t *
_ivm_serial_serializeExecList(ivm_exec_list_t *list,
							  ivm_vmstate_t *state)
{
	ivm_serial_exec_list_t *
	ret = STD_ALLOC(sizeof(*ret));
	ivm_exec_list_iterator_t eiter;
	ivm_exec_t *tmp_exec;
	ivm_string_pool_list_t *pools;
	ivm_size_t tmp_id;
	ivm_serial_exec_t *i;

	IVM_MEMCHECK(ret);

	pools = ret->pool_list = ivm_string_pool_list_new();
	i = ret->execs = STD_ALLOC(
		sizeof(*ret->execs)
		* (ret->size = ivm_exec_list_size(list))
	);

	IVM_MEMCHECK(i);

	IVM_EXEC_LIST_EACHPTR(list, eiter) {
		tmp_exec = IVM_EXEC_LIST_ITER_GET(eiter);
		tmp_id = ivm_string_pool_list_find(pools, ivm_exec_pool(tmp_exec));

		if (tmp_id == (ivm_size_t)-1) {
			tmp_id = ivm_string_pool_list_register(
				pools, ivm_exec_pool(tmp_exec)
			);
		}

		*i++ = _ivm_serial_serializeExec(tmp_exec, state, tmp_id);
	}

	return ret;
}

IVM_PRIVATE
ivm_exec_list_t *
_ivm_serial_unserializeExecList(ivm_serial_exec_list_t *list)
{
	ivm_exec_list_t *ret = ivm_exec_list_new();
	ivm_serial_exec_t *i, *end;
	ivm_exec_t *tmp;

	for (i = list->execs,
		 end = i + list->size;
		 i != end; i++) {
		tmp = _ivm_serial_unserializeExec(i, list->pool_list);
		if (!tmp) {
			ivm_exec_list_free(ret);
			return IVM_NULL;
		}
		ivm_exec_list_push(ret, tmp);
	}

	return ret;
}

IVM_PRIVATE
ivm_serial_exec_unit_t *
_ivm_serial_serializeExecUnit(ivm_exec_unit_t *unit,
							  ivm_vmstate_t *state)
{
	ivm_serial_exec_unit_t *ret = STD_ALLOC(sizeof(*ret));

	IVM_MEMCHECK(ret);

	ret->root = ivm_exec_unit_root(unit);
	ret->list = _ivm_serial_serializeExecList(ivm_exec_unit_execList(unit), state);

	return ret;
}

IVM_PRIVATE
ivm_exec_unit_t *
_ivm_serial_unserializeExecUnit(ivm_serial_exec_unit_t *unit)
{
	ivm_exec_list_t *list;
	ivm_exec_unit_t *ret;

	list = _ivm_serial_unserializeExecList(unit->list);

	if (!list) {
		return IVM_NULL;
	}

	ret = ivm_exec_unit_new(unit->root, list);

	return ret;
}

IVM_PRIVATE
void
_ivm_serial_param_list_free(ivm_serial_param_list_t *param)
{
	if (param) {
		STD_FREE(param->param);
		STD_FREE(param);
	}

	return;
}

IVM_PRIVATE
void
_ivm_serial_exec_dump(ivm_serial_exec_t *exec)
{
	if (exec) {
		_ivm_serial_param_list_free(exec->param);
		STD_FREE(exec->instrs);
	}

	return;
}

IVM_PRIVATE
void
_ivm_serial_exec_list_free(ivm_serial_exec_list_t *list)
{
	ivm_serial_exec_t *i, *end;

	if (list) {
		ivm_string_pool_list_free(list->pool_list);
		if (list->execs) {
			for (i = list->execs,
				 end = i + list->size;
				 i != end; i++) {
				_ivm_serial_exec_dump(i);
			}
		}
		STD_FREE(list->execs);
		STD_FREE(list);
	}

	return;
}

IVM_PRIVATE
void
_ivm_serial_exec_unit_free(ivm_serial_exec_unit_t *unit)
{
	if (unit) {
		_ivm_serial_exec_list_free(unit->list);
		STD_FREE(unit);
	}

	return;
}

/*
 * string pool format(in bits):
 * ---------------------
 * |     size: 64      |
 * ---------------------
 * |    is_const: 1    |
 * |      len: 32      |
 * |    content: v     | string 0
 * ---------------------
 * |    is_const: 1    |
 * |      len: 32      |
 * |    content: v     | string 1
 * ---------------------
 *           .
 *           .
 *           .
 */

IVM_PRIVATE
ivm_size_t
_ivm_serial_stringPoolToFile(ivm_string_pool_t *pool,
							 ivm_stream_t *stream)
{
	ivm_uint64_t size = ivm_string_pool_size(pool);
	const ivm_string_t **i, **end;
	ivm_size_t ret = 0;

	ret += ivm_stream_write(stream, &size, sizeof(size), 1);

	for (i = ivm_string_pool_core(pool),
		 end = i + size; i != end; i++) {
		ret += ivm_stream_write(stream, (void *)*i, ivm_string_size(*i), 1);
	}

	return ret;
}

IVM_PRIVATE
ivm_string_pool_t *
_ivm_serial_stringPoolFromFile(ivm_stream_t *stream)
{
	ivm_heap_t *heap = IVM_NULL;
	ivm_uint64_t size;
	ivm_size_t tmp_len;
	const ivm_string_t **table, **i, **end;
	ivm_string_t tmp_head;
	ivm_string_t *tmp_str;
	ivm_string_pool_t *ret;

	if (!ivm_stream_read(stream, &size, sizeof(size), 1))
		return IVM_NULL;
	
	// IVM_TRACE("pool size: %ld\n", size);

	heap = ivm_heap_new(IVM_DEFAULT_STRING_POOL_BLOCK_SIZE);
	// ret = ivm_string_pool_new(IVM_TRUE);

	table = STD_ALLOC(sizeof(*table) * size);

	if (!table) {
		goto CLEAN;
	}

	// IVM_MEMCHECK(table);

	for (i = table,
		 end = i + size;
		 i != end; i++) {
		/* read header */
		if (!ivm_stream_read(stream, &tmp_head, sizeof(tmp_head), 1)) goto CLEAN;

		tmp_str = ivm_heap_alloc(heap, ivm_string_size(&tmp_head));
		if (!tmp_str) goto CLEAN; // illegal size

		*tmp_str = tmp_head; /* copy header */
		tmp_len = ivm_string_length(tmp_str) + 1;

		if (ivm_stream_read(stream, ivm_string_trimHead(tmp_str),
						  sizeof(ivm_char_t), tmp_len)
			!= tmp_len) {
			goto CLEAN;
		}

		*i = tmp_str;
	}

goto CLEAN_END;
CLEAN:

	ivm_heap_free(heap);
	STD_FREE(table);

	return IVM_NULL;

CLEAN_END:

	ret = ivm_string_pool_new_t(table, size, heap);

	STD_FREE(table);

	return ret;
}

/*
 * string pool list format(in bits):
 * ---------------------
 * |     size: 64      |
 * ---------------------
 * |     pool0: v      |
 * ---------------------
 * |     pool1: v      |
 * ---------------------
 * |     pool2: v      |
 * ---------------------
 *           .
 *           .
 *           .
 */

IVM_PRIVATE
ivm_size_t
_ivm_serial_stringPoolListToFile(ivm_string_pool_list_t *list,
								 ivm_stream_t *stream)
{
	ivm_string_pool_list_iterator_t siter;
	ivm_uint64_t size = ivm_string_pool_list_size(list);
	ivm_size_t ret = 0;

	ret += ivm_stream_write(stream, &size, sizeof(size), 1);

	IVM_STRING_POOL_LIST_EACHPTR(list, siter) {
		ret += _ivm_serial_stringPoolToFile(
			IVM_STRING_POOL_LIST_ITER_GET(siter),
			stream
		);
	}

	return ret;
}

IVM_PRIVATE
ivm_string_pool_list_t *
_ivm_serial_stringPoolListFromFile(ivm_stream_t *stream)
{
	ivm_string_pool_list_t *ret;
	ivm_string_pool_t *tmp_pool;
	ivm_uint64_t size;
	ivm_size_t i;

	if (!ivm_stream_read(stream, &size, sizeof(size), 1))
		return IVM_NULL;
	
	// IVM_TRACE("pool count: %ld\n", size);

	ret = ivm_string_pool_list_new();

	for (i = 0; i < size; i++) {
		tmp_pool = _ivm_serial_stringPoolFromFile(stream);
		if (!tmp_pool) goto CLEAN;
		ivm_string_pool_list_register(ret, tmp_pool);
	}

goto CLEAN_END;
CLEAN:;
	
	ivm_string_pool_list_free(ret);

	return IVM_NULL;

CLEAN_END:

	return ret;
}

/*
 * exec format(in bits):
 * ---------------------
 * |    pool_id: 64    |
 * |     size: 64      |
 * ---------------------
 * |  param_count: 64  |
 * ---------------------
 * |   param1 id: 64   |
 * | param1 is_varg: 8 |
 * ---------------------
 * |   param2 id: 64   |
 * | param2 is_varg: 8 |
 * ---------------------
 * |         .         |
 * |         .         |
 * |         .         |
 * ---------------------
 * |     instr0: v     |
 * |     instr1: v     |
 * |         .         |
 * |         .         |
 * |         .         |
 * ---------------------
 */

IVM_PRIVATE
ivm_size_t
_ivm_serial_execToFile(ivm_serial_exec_t *exec,
					   ivm_stream_t *stream)
{
	ivm_size_t ret = 0;
	ivm_size_t i;
	ivm_serial_param_t *tmp;
	ivm_serial_instr_t *tmp_ip;

	ret += ivm_stream_write(stream, &exec->pool, sizeof(exec->pool), 1);
	ret += ivm_stream_write(stream, &exec->size, sizeof(exec->size), 1);

	ret += ivm_stream_write(stream, &exec->param->count, sizeof(exec->param->count), 1);
	for (i = 0; i < exec->param->count; i++) {
		tmp = exec->param->param + i;
		ret += ivm_stream_write(stream, &tmp->name, sizeof(tmp->name), 1);
		ret += ivm_stream_write(stream, &tmp->is_varg, sizeof(tmp->is_varg), 1);
	}

	for (i = 0; i < exec->size; i++) {
		tmp_ip = exec->instrs + i;
		ret += ivm_stream_write(stream, &tmp_ip->opc, sizeof(tmp_ip->opc), 1);
		ret += ivm_stream_write(stream, &tmp_ip->arg, sizeof(tmp_ip->arg), 1);
	}

	return ret;
}

IVM_PRIVATE
ivm_serial_param_list_t *
_ivm_serial_paramFromFile(ivm_stream_t *stream)
{
	ivm_serial_param_list_t *ret = STD_ALLOC(sizeof(*ret));
	ivm_size_t i;
	ivm_serial_param_t *tmp;

	IVM_MEMCHECK(ret);

	if (!ivm_stream_read(stream, &ret->count, sizeof(ret->count), 1)) {
		STD_FREE(ret);
		return IVM_NULL;
	}

	ret->param = STD_ALLOC(sizeof(*ret->param) * ret->count);

	if (!ret->param) {
		STD_FREE(ret);
		return IVM_NULL;
	}

	for (i = 0; i < ret->count; i++) {
		tmp = ret->param + i;
		if (!ivm_stream_read(stream, &tmp->name, sizeof(tmp->name), 1) ||
			!ivm_stream_read(stream, &tmp->is_varg, sizeof(tmp->is_varg), 1)) {
			_ivm_serial_param_list_free(ret);
			return IVM_NULL;
		}
	}

	return ret;
}

IVM_PRIVATE
ivm_bool_t
_ivm_serial_execFromFile(ivm_stream_t *stream,
						 ivm_serial_exec_t *ret)
{
	ivm_size_t i;
	ivm_serial_instr_t *tmp_ip;

	if (!ivm_stream_read(stream, &ret->pool, sizeof(ret->pool), 1)) return IVM_FALSE;
	if (!ivm_stream_read(stream, &ret->size, sizeof(ret->size), 1)) return IVM_FALSE;

	ret->param = _ivm_serial_paramFromFile(stream);

	if (!ret->param) return IVM_FALSE;

	ret->instrs = STD_ALLOC(sizeof(*ret->instrs) * ret->size);

	if (!ret->instrs) {
		_ivm_serial_param_list_free(ret->param);
		return IVM_FALSE;
	}

	for (i = 0; i < ret->size; i++) {
		tmp_ip = ret->instrs + i;
		if (!ivm_stream_read(stream, &tmp_ip->opc, sizeof(tmp_ip->opc), 1) ||
			!ivm_stream_read(stream, &tmp_ip->arg, sizeof(tmp_ip->arg), 1)) {
			STD_FREE(ret->instrs);
			ret->instrs = IVM_NULL;
			return IVM_FALSE;
		}
	}

	return IVM_TRUE;
}

/*
 * exec list format(in bits):
 * ---------------------
 * |     pools: v      |
 * ---------------------
 * |     size: 64      |
 * ---------------------
 * |     exec0: v      |
 * ---------------------
 * |     exec1: v      |
 * ---------------------
 * |     exec2: v      |
 * ---------------------
 *           .
 *           .
 *           .
 */

IVM_PRIVATE
ivm_size_t
_ivm_serial_execListToFile(ivm_serial_exec_list_t *list,
						   ivm_stream_t *stream)
{
	ivm_size_t ret = 0;
	ivm_serial_exec_t *i, *end;

	ret += _ivm_serial_stringPoolListToFile(
		list->pool_list,
		stream
	);

	ret += ivm_stream_write(stream, &list->size, sizeof(list->size), 1);

	for (i = list->execs,
		 end = i + list->size;
		 i != end; i++) {
		ret += _ivm_serial_execToFile(i, stream);
	}

	return ret;
}

IVM_PRIVATE
ivm_serial_exec_list_t *
_ivm_serial_execListFromFile(ivm_stream_t *stream)
{
	ivm_serial_exec_t *i, *end;
	ivm_serial_exec_list_t *
	ret = STD_ALLOC_INIT(sizeof(*ret));

	IVM_MEMCHECK(ret);

	if (!(ret->pool_list = _ivm_serial_stringPoolListFromFile(stream)))
		goto CLEAN;

	if (!ivm_stream_read(stream, &ret->size, sizeof(ret->size), 1))
		goto CLEAN;

	/* avoid error in cleaning */
	i = ret->execs = STD_ALLOC_INIT(sizeof(*ret->execs) * ret->size);

	IVM_MEMCHECK(i);

	for (end = i + ret->size;
		 i != end; i++) {
		if (!_ivm_serial_execFromFile(stream, i))
			goto CLEAN;
		if (i->pool >= ivm_string_pool_list_size(ret->pool_list))
			goto CLEAN;
	}

goto CLEAN_END;
CLEAN:;
	_ivm_serial_exec_list_free(ret);
	return IVM_NULL;
CLEAN_END:

	return ret;
}

/*
 * exec unit format(in bits):
 * ---------------------
 * |     root: 64      |
 * ---------------------
 * |     execs: v      |
 * ---------------------
 */

IVM_PRIVATE
ivm_size_t
_ivm_serial_execUnitToFile(ivm_serial_exec_unit_t *unit,
						   ivm_stream_t *stream)
{
	ivm_size_t ret = 0;

	ret += ivm_stream_write(stream, &unit->root, sizeof(unit->root), 1);
	ret += _ivm_serial_execListToFile(unit->list, stream);

	return ret;
}

IVM_PRIVATE
ivm_serial_exec_unit_t *
_ivm_serial_execUnitFromFile(ivm_stream_t *stream)
{
	ivm_serial_exec_unit_t *ret = STD_ALLOC_INIT(sizeof(*ret));

	IVM_MEMCHECK(ret);

	if (!ivm_stream_read(stream, &ret->root, sizeof(ret->root), 1)) {
		goto CLEAN;
	}

	if (!(ret->list = _ivm_serial_execListFromFile(stream))) {
		goto CLEAN;
	}

goto CLEAN_END;
CLEAN:;
	_ivm_serial_exec_unit_free(ret);
	return IVM_NULL;
CLEAN_END:

	return ret;
}

ivm_exec_unit_t *
ivm_serial_decodeCache(ivm_stream_t *stream)
{
	ivm_exec_unit_t *ret = IVM_NULL;
	ivm_serial_exec_unit_t *s_unit;
	ivm_char_t hbuf[2];

	if (!ivm_stream_read(stream, hbuf, sizeof(*hbuf), 2) ||
		hbuf[0] != '\x31' ||
		hbuf[1] != '\x31') {
		return IVM_NULL;
	}

	s_unit = _ivm_serial_execUnitFromFile(stream);
	if (s_unit) {
		ret = _ivm_serial_unserializeExecUnit(s_unit);
		_ivm_serial_exec_unit_free(s_unit);
	}

	return ret;
}

ivm_size_t
ivm_serial_encodeCache(ivm_exec_unit_t *unit,
					   ivm_stream_t *stream)
{
	ivm_serial_exec_unit_t *s_unit;
	ivm_size_t size = 0;
	IVM_PRIVATE const ivm_char_t htok[] = "\x31\x31"; // header token

	s_unit = _ivm_serial_serializeExecUnit(unit, IVM_NULL);
	
	size += ivm_stream_write(stream, htok, sizeof(*htok), 2);
	size += _ivm_serial_execUnitToFile(s_unit, stream);

	_ivm_serial_exec_unit_free(s_unit);

	return size;
}

ivm_object_t *
ivm_serial_mod_loadCache(const ivm_char_t *path,
						 ivm_char_t **err,
						 ivm_bool_t *is_const,
						 ivm_vmstate_t *state,
						 ivm_coro_t *coro,
						 ivm_context_t *context)
{
	ivm_char_t *tmp_err = IVM_NULL;
	ivm_object_t *ret = IVM_NULL;
	ivm_file_t *cache_fp = ivm_file_new(path, IVM_FMODE_READ_BINARY);
	ivm_exec_unit_t *unit = IVM_NULL;
	ivm_function_t *root;
	ivm_context_t *dest;
	ivm_stream_t *stream;

	*is_const = IVM_TRUE;

	if (!cache_fp) {
		tmp_err = IVM_ERROR_MSG_FAILED_OPEN_FILE;
		goto END;
	}

	stream = ivm_file_stream_new(cache_fp);

	unit = ivm_serial_decodeCache(stream);

	ivm_stream_free(stream);
	ivm_file_free(cache_fp);
	
	if (!unit) {
		tmp_err = IVM_ERROR_MSG_FAILED_PARSE_CACHE;
		goto END;
	}

	root = ivm_exec_unit_mergeToVM(unit, state);

	ivm_exec_unit_free(unit);

	if (!root) {
		tmp_err = IVM_ERROR_MSG_CACHE_NO_ROOT;
		goto END;
	}

	ivm_function_invoke_r(root, state, coro, IVM_NULL);
	dest = ivm_context_addRef(ivm_coro_getRuntimeGlobal(coro));
	ivm_coro_resume(coro, state, IVM_NULL);

	ret = ivm_context_getObject(dest, state);

	ivm_context_free(dest, state);

END:

	*err = tmp_err;

	return ret;
}
