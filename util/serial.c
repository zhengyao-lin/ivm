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

	IVM_ASSERT(!ivm_exec_cached(exec), IVM_ERROR_MSG_SERIALIZE_CACHED_EXEC);

	tmp = ret.instrs = STD_ALLOC(sizeof(*ret.instrs) * ret.size);

	IVM_MEMCHECK(ret.instrs);

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

ivm_exec_t *
_ivm_serial_unserializeExec(ivm_serial_exec_t *exec,
							ivm_string_pool_list_t *pool_list)
{
	ivm_exec_t *ret = ivm_exec_new(
		ivm_string_pool_list_at(pool_list, exec->pool)
	);
	ivm_serial_instr_t *i, *end;

	for (i = exec->instrs,
		 end = i + exec->size;
		 i != end; i++) {
		ivm_exec_addInstr_c(ret, ivm_instr_build(i->opc, i->arg));
	}

	return ret;
}

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

ivm_exec_list_t *
_ivm_serial_unserializeExecList(ivm_serial_exec_list_t *list)
{
	ivm_exec_list_t *ret = ivm_exec_list_new();
	ivm_serial_exec_t *i, *end;

	for (i = list->execs,
		 end = i + list->size;
		 i != end; i++) {
		ivm_exec_list_push(
			ret,
			_ivm_serial_unserializeExec(i, list->pool_list)
		);
	}

	return ret;
}

ivm_serial_exec_unit_t *
ivm_serial_serializeExecUnit(ivm_exec_unit_t *unit,
							 ivm_vmstate_t *state)
{
	ivm_serial_exec_unit_t *ret = STD_ALLOC(sizeof(*ret));

	IVM_MEMCHECK(ret);

	ret->root = ivm_exec_unit_root(unit);
	ret->list = _ivm_serial_serializeExecList(
		ivm_exec_unit_execList(unit), state
	);

	return ret;
}

ivm_exec_unit_t *
ivm_serial_unserializeExecUnit(ivm_serial_exec_unit_t *unit)
{
	ivm_exec_unit_t *ret = ivm_exec_unit_new(
		unit->root,
		_ivm_serial_unserializeExecList(unit->list)
	);

	return ret;
}

IVM_PRIVATE
void
_ivm_serial_exec_dump(ivm_serial_exec_t *exec)
{
	if (exec) {
		STD_FREE(exec->instrs);
	}

	return;
}

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

void
ivm_serial_exec_unit_free(ivm_serial_exec_unit_t *unit)
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

ivm_size_t
_ivm_serial_stringPoolToFile(ivm_string_pool_t *pool,
							 ivm_file_t *file)
{
	ivm_uint64_t size = ivm_string_pool_size(pool);
	const ivm_string_t **i, **end;
	ivm_size_t ret = 0;
	ivm_size_t size_pos = ivm_file_curPos(file);

	ret += ivm_file_write(file, &size, sizeof(size), 1);

	for (i = ivm_string_pool_core(pool),
		 end = i + size, size = 0;
		 i != end && *i; i++, size++) {
		ret += ivm_file_write(file, (void *)*i, ivm_string_size(*i), 1);
	}

	ivm_file_writeAt(file, size_pos, &size, sizeof(size), 1);

	return ret;
}

ivm_string_pool_t *
_ivm_serial_stringPoolFromFile(ivm_file_t *file)
{
	ivm_heap_t *heap = IVM_NULL;
	ivm_uint64_t size;
	ivm_size_t tmp_len, fsize;
	const ivm_string_t **table, **i, **end;
	ivm_string_t tmp_head;
	ivm_string_t *tmp_str;

	if (!ivm_file_read(file, &size, sizeof(size), 1))
		return IVM_NULL;

	fsize = ivm_file_length(file);
	
	// IVM_ASSERT(tmp, IVM_ERROR_MSG_FILE_FORMAT_ERR);
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
		if (!ivm_file_read(file, &tmp_head, sizeof(tmp_head), 1)) goto CLEAN;

		if (ivm_string_size(&tmp_head) < fsize)
			tmp_str = ivm_heap_alloc(heap, ivm_string_size(&tmp_head));
		else goto CLEAN; // illegal size

		*tmp_str = tmp_head; /* copy header */
		tmp_len = ivm_string_length(tmp_str) + 1;

		if (ivm_file_read(file, ivm_string_trimHead(tmp_str),
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

	return ivm_string_pool_new_t(table, size, heap);
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

ivm_size_t
_ivm_serial_stringPoolListToFile(ivm_string_pool_list_t *list,
								 ivm_file_t *file)
{
	ivm_string_pool_list_iterator_t siter;
	ivm_uint64_t size = ivm_string_pool_list_size(list);
	ivm_size_t ret = 0;

	ret += ivm_file_write(file, &size, sizeof(size), 1);

	IVM_STRING_POOL_LIST_EACHPTR(list, siter) {
		ret += _ivm_serial_stringPoolToFile(
			IVM_STRING_POOL_LIST_ITER_GET(siter),
			file
		);
	}

	return ret;
}

ivm_string_pool_list_t *
_ivm_serial_stringPoolListFromFile(ivm_file_t *file)
{
	ivm_string_pool_list_t *ret;
	ivm_string_pool_t *tmp_pool;
	ivm_uint64_t size;
	ivm_size_t i;

	if (!ivm_file_read(file, &size, sizeof(size), 1))
		return IVM_NULL;
	// IVM_ASSERT(tmp, IVM_ERROR_MSG_FILE_FORMAT_ERR);
	
	// IVM_TRACE("pool count: %ld\n", size);

	ret = ivm_string_pool_list_new();

	for (i = 0; i < size; i++) {
		tmp_pool = _ivm_serial_stringPoolFromFile(file);
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
 * |     instr0: v     |
 * |     instr1: v     |
 * |         .         |
 * |         .         |
 * |         .         |
 * ---------------------
 */

ivm_size_t
_ivm_serial_execToFile(ivm_serial_exec_t *exec,
					   ivm_file_t *file)
{
	ivm_size_t ret = 0;

	ret += ivm_file_write(file, &exec->pool, sizeof(exec->pool), 1);
	ret += ivm_file_write(file, &exec->size, sizeof(exec->size), 1);
	ret += ivm_file_write(file, exec->instrs, sizeof(*exec->instrs), exec->size);

	return ret;
}

ivm_bool_t
_ivm_serial_execFromFile(ivm_file_t *file,
						 ivm_serial_exec_t *ret)
{
	if (!ivm_file_read(file, &ret->pool, sizeof(ret->pool), 1)) return IVM_FALSE;
	if (!ivm_file_read(file, &ret->size, sizeof(ret->size), 1)) return IVM_FALSE;

	ret->instrs = STD_ALLOC(sizeof(*ret->instrs) * ret->size);

	IVM_MEMCHECK(ret->instrs);

	if (ivm_file_read(file, ret->instrs, sizeof(*ret->instrs), ret->size)
		!= ret->size) {
		STD_FREE(ret->instrs);
		ret->instrs = IVM_NULL;
		return IVM_FALSE;
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

ivm_size_t
_ivm_serial_execListToFile(ivm_serial_exec_list_t *list,
						   ivm_file_t *file)
{
	ivm_size_t ret = 0;
	ivm_serial_exec_t *i, *end;

	ret += _ivm_serial_stringPoolListToFile(
		list->pool_list,
		file
	);

	ret += ivm_file_write(file, &list->size, sizeof(list->size), 1);

	for (i = list->execs,
		 end = i + list->size;
		 i != end; i++) {
		ret += _ivm_serial_execToFile(i, file);
	}

	return ret;
}

ivm_serial_exec_list_t *
_ivm_serial_execListFromFile(ivm_file_t *file)
{
	ivm_serial_exec_t *i, *end;
	ivm_serial_exec_list_t *
	ret = STD_ALLOC_INIT(sizeof(*ret));

	IVM_MEMCHECK(ret);

	if (!(ret->pool_list = _ivm_serial_stringPoolListFromFile(file)))
		goto CLEAN;

	if (!ivm_file_read(file, &ret->size, sizeof(ret->size), 1))
		goto CLEAN;

	/* avoid error in cleaning */
	i = ret->execs = STD_ALLOC_INIT(sizeof(*ret->execs) * ret->size);

	IVM_MEMCHECK(i);

	for (end = i + ret->size;
		 i != end; i++) {
		if (!_ivm_serial_execFromFile(file, i))
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

ivm_size_t
ivm_serial_execUnitToFile(ivm_serial_exec_unit_t *unit,
						  ivm_file_t *file)
{
	ivm_size_t ret = 0;

	ret += ivm_file_write(file, &unit->root, sizeof(unit->root), 1);
	ret += _ivm_serial_execListToFile(unit->list, file);

	return ret;
}

ivm_serial_exec_unit_t *
ivm_serial_execUnitFromFile(ivm_file_t *file)
{
	ivm_serial_exec_unit_t *ret = STD_ALLOC_INIT(sizeof(*ret));

	IVM_MEMCHECK(ret);

	if (!ivm_file_read(file, &ret->root, sizeof(ret->root), 1)) {
		goto CLEAN;
	}

	if (!(ret->list = _ivm_serial_execListFromFile(file))) {
		goto CLEAN;
	}

goto CLEAN_END;
CLEAN:;
	ivm_serial_exec_unit_free(ret);
	return IVM_NULL;
CLEAN_END:

	return ret;
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
	ivm_file_t *cache = ivm_file_new(path, IVM_FMODE_READ_BINARY);
	ivm_exec_unit_t *unit = IVM_NULL;
	ivm_function_t *root;
	ivm_context_t *dest;

	*is_const = IVM_TRUE;

	if (!cache) {
		tmp_err = IVM_ERROR_MSG_FAILED_OPEN_FILE;
		goto END;
	}

	unit = ivm_serial_parseCacheFile(cache);

	ivm_file_free(cache);
	
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
